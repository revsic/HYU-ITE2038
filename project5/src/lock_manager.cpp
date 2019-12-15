#include <condition_variable>
#include <thread>

#include "dbms.hpp"
#include "lock_manager.hpp"
#include "utils.hpp"
#include "xaction_manager.hpp"

HierarchicalID::HierarchicalID()
    : HierarchicalID(INVALID_TABLEID, INVALID_PAGENUM, 0)
{
    // Do Nothing
}

HierarchicalID::HierarchicalID(tableid_t tid, pagenum_t pid, size_t rid) :
    tid(tid), pid(pid), rid(rid)
{
    // Do Nothing
}

HashableID HierarchicalID::make_hashable() const {
    return HashableID(utils::token, tid, pid, rid);
}

bool HierarchicalID::operator<(HierarchicalID const& other) const {
    return tid < other.tid
        || (tid == other.tid && pid < other.pid)
        || (tid == other.tid && pid == other.pid && rid < other.rid);
}

bool HierarchicalID::operator==(HierarchicalID const& other) const {
    return tid == other.tid && pid == other.pid && rid == other.rid;
}

Lock::Lock() : hid(), mode(LockMode::IDLE), backref(nullptr), wait_flag(false)
{
    // Do Nothing
}

Lock::Lock(HID hid, LockMode mode, Transaction* backref
) : hid(hid), mode(mode), backref(backref), wait_flag(false)
{
    // Do Nothing
}

Lock::Lock(Lock&& lock) noexcept :
    hid(lock.hid), mode(lock.mode),
    backref(lock.backref), wait_flag(lock.wait_flag.load())
{
    lock.hid = HID();
    lock.mode = LockMode::IDLE;
    lock.backref = nullptr;
    lock.wait_flag = false;
}

Lock& Lock::operator=(Lock&& lock) noexcept {
    hid = lock.hid;
    mode = lock.mode;
    backref = lock.backref;
    wait_flag = lock.wait_flag.load();

    lock.hid = HID();
    lock.mode = LockMode::IDLE;
    lock.backref = nullptr;
    lock.wait_flag = false;

    return *this;
}

HID Lock::get_hid() const {
    return hid;
}

LockMode Lock::get_mode() const {
    return mode;
}

Transaction& Lock::get_backref() const {
    return *backref;
}

bool Lock::stop() const {
    return wait_flag;
}

Status Lock::wait() {
    wait_flag = true;
    return Status::SUCCESS;
}

Status Lock::run() {
    wait_flag = false;
    backref->wait = nullptr;

    TrxState waiting = TrxState::WAITING;
    backref->state.compare_exchange_strong(
        waiting, TrxState::RUNNING);
    return Status::SUCCESS;
}

LockManager::LockStruct::LockStruct() :
    mode(LockMode::IDLE), run(), wait()
{
    // Do Nothing
}

LockManager::LockManager() :
    mtx(), locks(), detector(), db(nullptr)
{
    // Do Nothing
}

std::shared_ptr<Lock> LockManager::require_lock(
    Transaction* backref, HID hid, LockMode mode
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

    backref->wait = new_lock;
    backref->state = TrxState::WAITING;

    new_lock->wait();
    locks[id].wait.push_back(new_lock);

    own.unlock();

    int selfcheck = 10;
    while (new_lock->stop() && backref->get_state() != TrxState::ABORTED) {
        detect_and_release();
        if (--selfcheck == 0) {
            selfcheck = 10;
            std::unique_lock<std::mutex> inner(mtx);
            auto& target = locks[id].run.front();
            if (db->trx_state(target->get_backref().id) == TrxState::INVALID) {
                release_lock(target, false);
            }
        }
        std::this_thread::yield();
    }

    // module updates are already occurred in release_lock because of deadlock
    return new_lock;
}

Status LockManager::release_lock(std::shared_ptr<Lock> lock, bool acquire) {
    std::unique_lock<std::mutex> own(mtx, std::defer_lock);
    if (acquire) {
        own.lock();
    }

    HashableID hid = lock->get_hid().make_hashable();
    auto iter = locks.find(hid);
    CHECK_TRUE(iter != locks.end());

    LockStruct& module = iter->second;
    auto found = std::find(module.run.begin(), module.run.end(), lock);
    if (found != module.run.end()) {
        module.run.erase(found);
    } else {
        module.wait.remove(lock);
        lock->run();
    }

    Transaction& backref = lock->get_backref();

    if (module.run.size() > 0) {
        return Status::SUCCESS;
    }

    if (module.wait.size() == 0) {
        locks.erase(iter);
        return Status::SUCCESS;
    }

    if (module.wait.front()->get_mode() == LockMode::SHARED) {
        module.mode = LockMode::SHARED;
        for (auto iter = module.wait.begin(); iter != module.wait.end();) {
            lock = *iter;
            if (lock->get_mode() != LockMode::SHARED) {
                ++iter;
                continue;
            }

            iter = module.wait.erase(iter);
            module.run.push_front(lock);
            lock->run();
        }
    } else {
        module.mode = LockMode::EXCLUSIVE;

        lock = module.wait.front();
        module.wait.pop_front();
        module.run.push_front(lock);
        lock->run();
    }

    return Status::SUCCESS;
}

Status LockManager::detect_and_release() {
    CHECK_SUCCESS(detector.schedule());

    CHECK_TRUE(!detector.detection_lock.test_and_set());
    auto defer = utils::defer([&]{ detector.detection_lock.clear(); });

    std::unique_lock<std::mutex> own(mtx);
    std::vector<trxid_t> found = detector.find_cycle(locks);
    CHECK_TRUE(found.size() > 0);

    for (trxid_t xid : found) {
        CHECK_SUCCESS(db->abort_trx(xid));
    }
    return Status::SUCCESS;
}

Status LockManager::set_database(Database& db) {
    this->db = &db;
    return Status::SUCCESS;
}

bool LockManager::lockable(
    LockStruct const& module, std::shared_ptr<Lock> const& target
) const {
    return module.mode == LockMode::IDLE
        || (module.mode == LockMode::SHARED
            && target->get_mode() == LockMode::SHARED);
}

int LockManager::DeadlockDetector::Node::refcount() const {
    return prev_id.size();
}

int LockManager::DeadlockDetector::Node::outcount() const {
    return next_id.size();
}

LockManager::DeadlockDetector::DeadlockDetector() :
    detection_lock(ATOMIC_FLAG_INIT), tick_mtx(), tick(0)
{
    // Do Nothing
}

Status LockManager::DeadlockDetector::schedule() {
    std::unique_lock<std::mutex> own(tick_mtx);
    if (tick == 0) {
        return Status::SUCCESS;
    }
    --tick;
    return Status::FAILURE;
}

void LockManager::DeadlockDetector::reduce(
    graph_t& graph, trxid_t xid
) const {
    std::set<trxid_t> chained;
    auto iter = graph.find(xid);
    if (iter == graph.end()) {
        return;
    }

    Node& node = iter->second;
    for (trxid_t next_id : node.next_id) {
        Node& next = graph.at(next_id);
        next.prev_id.erase(xid);
        if (next.refcount() == 0) {
            chained.insert(next_id);
        }
    }

    for (trxid_t prev_id : node.prev_id) {
        graph.at(prev_id).next_id.erase(xid);
    }

    graph.erase(graph.find(xid));
    for (trxid_t next_xid : chained) {
        reduce(graph, next_xid);
    }
}

std::vector<trxid_t> LockManager::DeadlockDetector::find_cycle(
    locktable_t const& locks
) {
    graph_t graph = construct_graph(locks);

    while (graph.size() > 0) {
        auto iter = std::find_if(
            graph.begin(), graph.end(),
            [](auto const& pair) { return pair.second.refcount() == 0; });

        if (iter == graph.end()) {
            std::unique_lock<std::mutex> own(tick_mtx);
            tick = 0;
            return choose_abort(std::move(graph));
        }

        reduce(graph, iter->first);
    }

    std::unique_lock<std::mutex> own(tick_mtx);
    tick += 10;
    return std::vector<trxid_t>();
}

std::vector<trxid_t> LockManager::DeadlockDetector::choose_abort(
    graph_t graph
) const {
    std::vector<trxid_t> trxs;
    while (graph.size() > 0) {
        auto iter = std::max_element(
            graph.begin(), graph.end(),
            [](auto const& left, auto const& right) {
                return left.second.refcount() < right.second.refcount()
                    || (left.second.refcount() == right.second.refcount()
                        && left.second.outcount() < right.second.outcount());
            });

        trxid_t xid = iter->first;
        trxs.push_back(xid);
        reduce(graph, xid);
    }
    return trxs;
}

auto LockManager::DeadlockDetector::construct_graph(
    locktable_t const& locks
) -> LockManager::DeadlockDetector::graph_t {
    graph_t graph;

    for (auto const& iter : locks) {
        auto const& module = iter.second;
        for (auto const& wait_lock : module.wait) {
            trxid_t wait_xid = wait_lock->get_backref().get_id();
            for (auto const& run_lock : module.run) {
                trxid_t run_xid = run_lock->get_backref().get_id();
                graph[run_xid].prev_id.insert(wait_xid);
                graph[wait_xid].next_id.insert(run_xid);
            }
        }
    }

    return graph;
}
