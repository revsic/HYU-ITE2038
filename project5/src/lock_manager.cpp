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

Status Lock::run() {
    wait = false;
    return Status::SUCCESS;
}

LockManager::LockStruct::LockStruct() :
    mode(LockMode::IDLE), cv(), run(), wait()
{
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

    LockStruct* module = &locks[id];
    module->wait.push_back(new_lock);
    module->cv.wait(own, [&]{ return !new_lock->is_wait(); });

    module = &locks[id];
    module->wait.remove(new_lock);
    module->mode = mode;
    module->run.push_front(new_lock);

    own.unlock();

    backref->state = TrxState::RUNNING;
    backref->wait = nullptr;
    return new_lock;
}

Status LockManager::release_lock(std::shared_ptr<Lock> lock) {
    std::unique_lock<std::mutex> own(mtx);

    HashableID hid = lock->get_hid().make_hashable();
    auto iter = locks.find(hid);
    CHECK_TRUE(iter != locks.end());

    LockStruct& module = iter->second;
    module.run.remove(lock);
    if (module.run.size() > 0) {
        return Status::SUCCESS;
    }

    if (module.wait.size() == 0) {
        module.mode = LockMode::IDLE;
        return Status::SUCCESS;
    }
    lock = module.wait.front();
    lock->run();

    own.unlock();
    module.cv.notify_all();

    return Status::SUCCESS;
}

std::shared_ptr<Lock> LockManager::detect_deadlock() {
    std::unique_lock<std::mutex> own(mtx);

    

    return nullptr;
}

Status LockManager::detect_and_release() {
    CHECK_SUCCESS(schedule_detection());
    return Status::SUCCESS;
}

bool LockManager::lockable(
    LockStruct const& module, std::shared_ptr<Lock> const& target
) const {
    return module.mode == LockMode::IDLE
        || (module.mode == LockMode::SHARED
            && target->get_mode() == LockMode::SHARED);
}

Status LockManager::schedule_detection() {
    return Status::SUCCESS;
}
