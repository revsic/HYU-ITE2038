#include <condition_variable>

#include "lock_manager.hpp"

HierarchicalID::HierarchicalID()
    : HierarchicalID(INVALID_TABLEID, INVALID_PAGENUM, -1)
{
    // Do Nothing
}

HierarchicalID::HierarchicalID(tableid_t tid, pagenum_t pid, int rid) :
    tid(tid), pid(pid), rid(rid)
{
    // Do Nothing
}

HashableID HierarchicalID::make_hashable() const {
    return HashableID(pack_init, tid, pid, rid);
}

Lock::Lock() : hid(), mode(LockMode::IDLE), backref(nullptr), wait(false)
{
    // Do Nothing
}

Lock::Lock(HierarchicalID hid, LockMode mode, Transaction* backref
) : hid(hid), mode(mode), backref(backref), wait(false)
{
    // Do Nothing
}

Lock::~Lock() {
    // TODO: Impl lock destructor.
}

Lock::Lock(Lock&& lock) noexcept :
    hid(lock.hid), mode(lock.mode),
    backref(lock.backref), wait(lock.wait.load())
{
    lock.hid = HierarchicalID();
    lock.mode = LockMode::IDLE;
    lock.backref = nullptr;
    lock.wait = false;
}

Lock& Lock::operator=(Lock&& lock) noexcept {
    hid = lock.hid;
    mode = lock.mode;
    backref = lock.backref;
    wait = lock.wait.load();

    lock.hid = HierarchicalID();
    lock.mode = LockMode::IDLE;
    lock.backref = nullptr;
    lock.wait = false;

    return *this;
}

HierarchicalID Lock::get_hid() const {
    return hid;
}

LockMode Lock::get_mode() const {
    return mode;
}

Transaction& Lock::get_backref() const {
    return *backref;
}

bool Lock::is_wait() const {
    return wait;
}

LockManager::LockStruct::LockStruct() : mode(LockMode::IDLE), run(), wait() {
    // Do Nothing
}

LockManager::~LockManager() {
    // TODO: Impl shutdown
}

std::shared_ptr<Lock> LockManager::require_lock(
    Transaction* backref, HierarchicalID hid, LockMode mode
) {
    std::unique_lock<std::mutex> lock(mtx);

    HashableID id = hid.make_hashable();
    auto new_lock = std::make_shared<Lock>(hid, mode, backref);

    auto iter = locks.find(id);
    if (iter == locks.end() || lockable(iter->second, new_lock)) {
        locks[id].mode = mode;
        locks[id].run.push_front(new_lock);
        return new_lock;
    }

    locks[id].wait.push_back(new_lock);

    std::condition_variable cv;
    cv.wait(lock, [&]{ return !new_lock->is_wait(); });

    locks[id].wait.remove(new_lock);
    locks[id].mode = mode;
    locks[id].run.push_front(new_lock);

    cv.notify_all();
    return new_lock;
}

Status LockManager::release_lock(std::shared_ptr<Lock> lock) {
    std::unique_lock<std::mutex> own(mtx);



    return Status::SUCCESS;
}

Status LockManager::detect_deadlock() {
    return Status::SUCCESS;
}

Status LockManager::detect_and_release() {
    return Status::SUCCESS;
}

bool LockManager::lockable(
    LockStruct const& module, std::shared_ptr<Lock> const& target
) const {
    return module.mode == LockMode::IDLE
        || (module.mode == LockMode::SHARED
            && target->get_mode() == LockMode::SHARED);
}
