#include <condition_variable>

#include "lock_manager.hpp"
#include "xaction_manager.hpp"

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
    HashableID id = hid.make_hashable();
    auto new_lock = std::make_shared<Lock>(hid, mode, backref);

    std::unique_lock<std::mutex> own(mtx);

    auto iter = locks.find(id);
    if (iter == locks.end() || lockable(iter->second, new_lock)) {
        LockStruct& module = locks[id];
        module.mode = mode;
        module.run.push_front(new_lock);
        return new_lock;
    }

    backref->state = TrxState::WAITING;
    backref->wait = new_lock;
    locks[id].wait.push_back(new_lock);

    std::condition_variable cv;
    cv.wait(own, [&]{ return !new_lock->is_wait(); });

    LockStruct& module = locks[id];
    module.wait.remove(new_lock);
    module.mode = mode;
    module.run.push_front(new_lock);

    own.unlock();
    cv.notify_all();

    backref->state = TrxState::RUNNING;
    backref->wait = nullptr;
    return new_lock;
}

Status LockManager::release_lock(std::shared_ptr<Lock> lock) {
    std::unique_lock<std::mutex> own(mtx);

    HashableID hid = lock->get_hid().make_hashable();
    

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
