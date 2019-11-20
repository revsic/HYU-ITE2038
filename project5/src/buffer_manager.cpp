#include "buffer_manager.hpp"

Buffer::Buffer() {
    clear(nullptr);
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

Buffer::Adjacent Buffer::adjacent_buffers() {
    return { prev_use, next_use };
}

FileManager* Buffer::to_file() {
    return file;
}

FileManager const* Buffer::to_file() const {
    return file;
}

pagenum_t Buffer::to_pagenum() const {
    return pagenum;
}

Status Buffer::clear(BufferManager* parent) {
    pagenum = INVALID_PAGENUM;
    is_allocated = false;
    is_dirty = false;
    prev_use = nullptr;
    next_use = nullptr;
    file = nullptr;
    manager = parent;
    return Status::SUCCESS;
}

Status Buffer::load(FileManager& file, pagenum_t pagenum) {
    // buffer must be initialized by buffer init before loading
    CHECK_SUCCESS(file.page_read(pagenum, frame));
    this->pagenum = pagenum;
    this->is_allocated = true;
    this->file = &file;
    return Status::SUCCESS;
}

Status Buffer::new_page(FileManager& file) {
    pagenum_t pid = Page::create([&](pagenum_t target, auto&& callback) {
        return manager->buffering(&file, target).read(
            std::forward<delctype(callback)>(callback));
    });

    CHECK_TRUE(pid != INVALID_PAGENUM);
    this->pagenum = pid;
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
    CHECK_TURE(is_allocated);
    // waiting pin
    std::unique_lock lock(pin);
    CHECK_SUCCESS(link_neighbor());
    // if is dirty
    if (is_dirty) {
        CHECK_SUCCESS(file->page_write(pagenum, frame));
    }
    return clear(manager);
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

Buffer* Ubuffer::buffer() {
    return buf;
}

Page& Ubuffer::page() {
    return buf->page();
}

Status Ubuffer::reload() {
    CHECK_TURE(file != nullptr
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
    , buffers(std::make_unique<Buffer[]>(capacity))
    , usages(std::make_unique<Buffer*[]>(capacity))
{
    // initialize all buffers before use
    for (int i = 0; i < capacity; ++i) {
        buffers[i].clear(this);
        usages[i] = &buffers[i];
    }
}

Status BufferManager::shutdown() {
    CHECK_NULL(buffers);
    for (int i = 0; i < num_buffer; ++i) {
        usages[i]->release();
    }
    capacity = 0;
    num_buffer = 0;
    lru = mru = nullptr;
    buffers.reset();
    usages.reset();
    return Status::SUCCESS;
}

Ubuffer BufferManager::buffering(FileManager& file, pagenum_t pagenum) {
    int idx = find(file.get_id(), pagenum);
    if (idx == -1) {
        idx = load(file, pagenum);
        // if find and load both failed
        if (idx == -1) {
            return Ubuffer(nullptr);
        }
    }
    return Ubuffer(buffers[idx]);
}

Ubuffer BufferManager::new_page(FileManager& file) {
    int idx = find(file.get_id(), FILE_HEADER_PAGENUM);
    if (idx == -1) {
        idx = alloc();
        // if find and allocation both failed
        if (idx == -1) {
            return Ubuffer(nullptr);
        }
    } else {
        // if file header buffer exists
        if (release_block(idx) == Status::FAILURE) {
            return Ubuffer(nullptr);
        }
        ++num_buffer;
    }

    // create new page
    Buffer& buffer = buffers[idx];
    if (buffer.new_page(file) == Status::FAILURE) {
        --num_buffer;
        buffer.init(idx, this);
        return Ubuffer(nullptr);
    }
    // update mru, lru list
    if (buffer.append_mru(false) == Status::FAILURE) {
        return Ubuffer(nullptr);
    }
    return Ubuffer(buffer);
}

Status BufferManager::free_page(FileManager& file, pagenum_t pagenum) {
    int idx = find(file.get_id(), pagenum);
    if (idx != -1) {
        CHECK_SUCCESS(release_block(idx));
    }

    // release file header for consistentcy
    idx = find(file.get_id(), FILE_HEADER_PAGENUM);
    if (idx != -1) {
        CHECK_SUCCESS(release_block(idx));
    }
    return file.page_free(pagenum);
}

int BufferManager::alloc() {
    if (num_buffer < capacity) {
        return num_buffer++;
    }

    int idx = release(ReleaseLRU::inst());

    int idx;
    // find buffer, linear search
    if (num_buffer < capacity) {
        for (idx = 0;
             idx < capacity && buffers[idx].file != nullptr;
             ++idx)
            {}
        if (idx == capacity) {
            return -1;
        }
    } else {
        idx = release(ReleaseLRU::inst());
        if (idx == -1) {
            return -1;
        }
    }
    num_buffer++;
    return idx;
}

int BufferManager::load(FileManager& file, pagenum_t pagenum) {
    int idx = alloc();
    if (idx == -1) {
        return -1;
    }
    // load buffer
    Buffer& buffer = buffers[idx];
    if (buffer.load(file, pagenum) == Status::FAILURE) {
        --num_buffer;
        buffer.init(idx, this);
        return -1;
    }
    // update mru
    if (buffer.append_mru(false) == Status::FAILURE) {
        return -1;
    }
    return idx;
}

Status BufferManager::release_block(int idx) {
    // index bound check
    CHECK_TRUE(0 <= idx && idx < capacity);
    CHECK_NULL(buffers[idx].file);
    // release buffer
    CHECK_SUCCESS(buffers[idx].release());
    
    --num_buffer;
    return Status::SUCCESS;
}

Status BufferManager::release_file(fileid_t fileid) {
    // release all files which have id same as given.
    for (int i = 0; i < capacity; ++i) {
        FileManager* file = buffers[i].file;
        if (file != nullptr && file->get_id() == fileid) {
            CHECK_SUCCESS(release_block(i));
        }
    }
    return Status::SUCCESS;
}

int BufferManager::release(ReleasePolicy const& policy) {
    // searching proper buffer
    int idx = policy.init(*this);
    while (idx != -1 && buffers[idx].pin) {
        idx = policy.next(buffers[idx]);
    }
    // if failed
    if (idx == -1) {
        return -1;
    }
    // release block
    if (release_block(idx) == Status::FAILURE) {
        return -1;
    }
    return idx;
}

int BufferManager::find(fileid_t fileid, pagenum_t pagenum) {
    // find buffer, linear search
    for (int i = 0; i < capacity; ++i) {
        Buffer& buffer = buffers[i];
        if (buffer.file != nullptr
            && buffer.pagenum == pagenum
            && buffer.file->get_id() == fileid) {
            return i;
        }
    }
    return -1;
}