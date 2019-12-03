#include "buffer_manager.hpp"

Buffer::Buffer() {
    clear(-1, nullptr);
}

Buffer::~Buffer() {
    release();
}

Page& Buffer::page() {
    return frame;
}

Page const& Buffer::page() const {
    return frame;
}

Buffer::Adjacent Buffer::adjacent_buffers() const {
    return { prev_use, next_use };
}

Status Buffer::clear(int idx, BufferManager* parent) {
    pagenum = INVALID_PAGENUM;
    index = idx;
    is_allocated = false;
    is_dirty = false;
    pin = 0;
    prev_use = nullptr;
    next_use = nullptr;
    file = nullptr;
    manager = parent;
    return Status::SUCCESS;
}

Status Buffer::load(FileManager& file, pagenum_t pagenum, bool virtual_page) {
    // buffer must be initialized by buffer init before loading
    if (!virtual_page) {
        CHECK_SUCCESS(file.page_read(pagenum, frame));
    }
    this->pagenum = pagenum;
    this->is_allocated = true;
    this->file = &file;
    return Status::SUCCESS;
}

Status Buffer::link_neighbor() {
    // don't use unconnected node from lru to mru
    CHECK_NULL(manager);
    // if mru buffer
    if (next_use == nullptr) {
        manager->mru = prev_use;
    } else {
        next_use->prev_use = prev_use;
    }
    // if lru buffer
    if (prev_use == nullptr) {
        manager->lru = next_use;
    } else {
        prev_use->next_use = next_use;
    }
    return Status::SUCCESS;
}

Status Buffer::append_mru(bool link) {
    CHECK_NULL(manager);
    if (link) {
        CHECK_SUCCESS(link_neighbor());
    }

    prev_use = manager->mru;
    next_use = nullptr;
    // update mru block
    if (manager->mru != nullptr) {
        manager->mru->next_use = this;
    }
    manager->mru = this;
    if (manager->lru == nullptr) {
        manager->lru = this;
    }
    return Status::SUCCESS;
}

Status Buffer::release() {
    CHECK_TRUE(is_allocated);
    // waiting pin
    std::unique_lock<std::shared_timed_mutex> lock(mtx);
    CHECK_SUCCESS(link_neighbor());
    // if is dirty
    if (is_dirty) {
        CHECK_SUCCESS(file->page_write(pagenum, frame));
    }
    return clear(index, manager);
}

Ubuffer::Ubuffer(Buffer* buf, pagenum_t pagenum, FileManager* file)
    : buf(buf), pagenum(pagenum), file(file) {
    // Do Nothing
}

Ubuffer::Ubuffer(Buffer& buf) : Ubuffer(&buf, buf.pagenum, buf.file) {
    // Do Nothing
}

Ubuffer::Ubuffer(std::nullptr_t) : Ubuffer(nullptr, INVALID_PAGENUM, nullptr) {
    // Do Nothing
}

Ubuffer::Ubuffer(Ubuffer&& ubuffer) noexcept
    : buf(ubuffer.buf), pagenum(ubuffer.pagenum), file(ubuffer.file)
{
    // clear other
    ubuffer.buf = nullptr;
    ubuffer.pagenum = INVALID_PAGENUM;
    ubuffer.file = nullptr;
}

Ubuffer& Ubuffer::operator=(Ubuffer&& ubuffer) noexcept {
    buf = ubuffer.buf;
    pagenum = ubuffer.pagenum;
    file = ubuffer.file;
    // clear other
    ubuffer.buf = nullptr;
    ubuffer.pagenum = INVALID_PAGENUM;
    ubuffer.file = nullptr;
    return *this;
}

Ubuffer Ubuffer::clone() const {
    return Ubuffer(buf, pagenum, file);
}

Buffer* Ubuffer::buffer() {
    return buf;
}

Buffer const* Ubuffer::buffer() const {
    return buf;
}

Page& Ubuffer::page() {
    return buf->page();
}

Page const& Ubuffer::page() const {
    return buf->page();
}

Status Ubuffer::reload() {
    CHECK_TRUE(file != nullptr
        && buf != nullptr
        && buf->manager != nullptr);
    // rebuffering, shallow copy
    *this = buf->manager->buffering(*file, pagenum);
    CHECK_NULL(buf);
    return Status::SUCCESS;
}

Status Ubuffer::check_and_reload() {
    if (buf != nullptr && file != nullptr
        && buf->file != nullptr
        && buf->file->get_id() == file->get_id()
        && buf->pagenum == pagenum
    ) {
        return Status::SUCCESS;
    }
    return reload();
}

pagenum_t Ubuffer::to_pagenum() const {
    return pagenum;
}

BufferManager::BufferManager(int num_buffer)
    : capacity(num_buffer)
    , num_buffer(0)
    , lru(nullptr)
    , mru(nullptr)
    , dummy(std::make_unique<Buffer[]>(capacity))
    , buffers(std::make_unique<Buffer*[]>(capacity))
    , table()
{
    // initialize all buffers before use
    for (int i = 0; i < capacity; ++i) {
        dummy[i].clear(i, this);
        buffers[i] = &dummy[i];
    }
}

Status BufferManager::set_database(Database& dbms) {
    this->dbms = &dbms;
    return Status::SUCCESS;
}

Buffer* BufferManager::most_recently_used() const {
    return mru;
}

Buffer* BufferManager::least_recently_used() const {
    return lru;
}

Status BufferManager::shutdown() {
    std::unique_lock<std::recursive_mutex> lock(mtx);
    CHECK_NULL(buffers);
    for (int i = 0; i < num_buffer; ++i) {
        buffers[i]->release();
    }
    capacity = num_buffer = 0;
    lru = mru = nullptr;
    dummy.reset();
    buffers.reset();
    return Status::SUCCESS;
}

Ubuffer BufferManager::buffering(FileManager& file, pagenum_t pagenum, bool virtual_page) {
    std::unique_lock<std::recursive_mutex> lock(mtx);
    int idx = find(file.get_id(), pagenum);
    if (idx == -1) {
        idx = load(file, pagenum, virtual_page);
        // if find and load both failed
        if (idx == -1) {
            return Ubuffer(nullptr);
        }
    }
    return Ubuffer(*buffers[idx]);
}

Ubuffer BufferManager::new_page(FileManager& file) {
    std::unique_lock<std::recursive_mutex> lock(mtx);
    pagenum_t pid = Page::create(
        [&](pagenum_t target, bool virtual_page, auto&& callback) {
            return buffering(file, target, virtual_page).write(
                std::forward<decltype(callback)>(callback));
        });

    if (pid == INVALID_PAGENUM) {
        return Ubuffer(nullptr);
    }
    return buffering(file, pid);
}

Status BufferManager::free_page(FileManager& file, pagenum_t pagenum) {
    std::unique_lock<std::recursive_mutex> lock(mtx);
    int idx = find(file.get_id(), pagenum);
    if (idx != -1) {
        CHECK_SUCCESS(release_block(idx));
    }
    return Page::release([&](pagenum_t target, auto&& func) {
        return buffering(file, target).write(
            std::forward<decltype(func)>(func));
    }, pagenum);
}

int BufferManager::allocate_block() {
    if (num_buffer < capacity) {
        return num_buffer++;
    }
    return release(ReleaseLRU::inst());
}

int BufferManager::load(FileManager& file, pagenum_t pagenum, bool virtual_page) {
    int idx = allocate_block();
    if (idx == -1) {
        return -1;
    }
    // load buffer and update mru
    Buffer& buffer = *buffers[idx];
    if (buffer.load(file, pagenum, virtual_page) == Status::FAILURE
        || buffer.append_mru(false) == Status::FAILURE
    ) {
        release_block(idx);
        return -1;
    }
    table[{ utils::token, file.get_id(), pagenum }] = idx;
    return idx;
}

Status BufferManager::release_block(int idx) {
    if (buffers[idx]->is_allocated) {
        CHECK_TRUE(table.erase(
            { utils::token
            , buffers[idx]->file->get_id()
            , buffers[idx]->pagenum }) > 0);
        CHECK_SUCCESS(buffers[idx]->release());
    }
    std::swap(buffers[idx], buffers[--num_buffer]);
    std::swap(buffers[idx]->index, buffers[num_buffer]->index);
    if (idx != num_buffer) {
        table[
            { utils::token
            , buffers[idx]->file->get_id()
            , buffers[idx]->pagenum }] = idx;
    }
    return Status::SUCCESS;
}

Status BufferManager::release_file(fileid_t fileid) {
    std::unique_lock<std::recursive_mutex> lock(mtx);
    // release all files which have id same as given.
    for (int i = num_buffer - 1; i >= 0; --i) {
        Buffer* buffer = buffers[i];
        if (buffer->is_allocated
            && buffer->file != nullptr
            && buffer->file->get_id() == fileid
        ) {
            CHECK_SUCCESS(release_block(i));
        }
    }
    return Status::SUCCESS;
}

int BufferManager::release(ReleasePolicy const& policy) {
    // searching proper buffer
    Buffer* buf = policy.init(*this);
    while (buf != nullptr && buf->pin > 0) {
        buf = policy.next(*buf);
    }
    // if failed
    if (buf == nullptr) {
        return -1;
    }
    // release block
    if (buf->is_allocated) {
        if (table.erase(
            { utils::token
            , buf->file->get_id()
            , buf->pagenum }) == 0
            || buf->release() == Status::FAILURE
        ) {
            release_block(buf->index);
            return -1;
        }
    }
    return buf->index;
}

int BufferManager::find(fileid_t fileid, pagenum_t pagenum) {
    // find buffer, linear search
    auto iter = table.find({ utils::token, fileid, pagenum });
    if (iter != table.end()) {
        return (*iter).second;
    }
    return -1;
}
