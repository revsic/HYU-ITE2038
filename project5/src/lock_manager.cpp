#include <condition_variable>

#include "lock_manager.hpp"
#include "utils.hpp"
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

    utils::Defer defer([&] {
        backref->wait = nullptr;
        backref->state = TrxState::RUNNING;
        backref->locks.push_back(new_lock);
    });

    std::unique_lock<std::mutex> own(mtx);

    utils::Defer inlock_defer([&] {
        LockStruct& module = locks[id];
        module.mode = mode;
        module.run.push_front(new_lock);

        if (trxs.find(backref->get_id()) == trxs.end()) {
            trxs[backref->get_id()] = 0;
        }
        trxs[backref->get_id()]++;
    });

    auto iter = locks.find(id);
    if (iter == locks.end() || lockable(iter->second, new_lock)) {
        // backref and module updates are occured
        // in defer and inlock defer.
        return new_lock;
    }

    backref->state = TrxState::WAITING;
    backref->wait = new_lock;

    locks[id].wait.push_back(new_lock);

    while (!locks[id].cv.wait_for(
        own,
        LOCK_WAIT,
        [&]{ return !new_lock->is_wait(); })
    ) {
        Status res = detect_and_release();
        if (res == Status::SUCCESS) {
            locks[id].cv.notify_all();
        }
    }

    locks[id].wait.remove(new_lock);
    // backref and module updates are occured
    // in defer and inlock defer.
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

    Transaction& backref = lock->get_backref();
    if (--trxs[backref.get_id()] == 0) {
        trxs.erase(backref.get_id());
    }

    own.unlock();
    module.cv.notify_all();

    backref.locks.remove(lock);
    return Status::SUCCESS;
}

Status LockManager::detect_and_release() {
    CHECK_SUCCESS(detector.schedule());

    Transaction* found = detector.find_cycle(locks);
    CHECK_NULL(found);

    return found->abort_trx(*this);
}

bool LockManager::lockable(
    LockStruct const& module, std::shared_ptr<Lock> const& target
) const {
    return module.mode == LockMode::IDLE
        || (module.mode == LockMode::SHARED
            && target->get_mode() == LockMode::SHARED);
}

LockManager::DeadlockDetector::DeadlockDetector() :
    coeff(1), last_success(false), last_use(std::chrono::steady_clock::now())
{
    // Do Nothing
}

Status LockManager::DeadlockDetector::schedule() {
    using namespace std::chrono;
    coeff = last_success ? coeff + 1 : 1;
    auto now = steady_clock::now();
    return LOCK_WAIT * coeff >= duration_cast<milliseconds>(now - last_use)
        ? Status::SUCCESS
        : Status::FAILURE;
}

Transaction* LockManager::DeadlockDetector::find_cycle(
    locktable_t const& locks
) {
    graph_t graph = construct_graph(locks);

    last_success = false;
    return nullptr;
}

auto LockManager::DeadlockDetector::construct_graph(
    locktable_t const& locks
) -> LockManager::DeadlockDetector::graph_t {
    return nullptr;
}
