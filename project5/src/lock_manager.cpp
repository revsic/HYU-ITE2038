#include <condition_variable>

#include "dbms.hpp"
#include "lock_manager.hpp"
#include "utils.hpp"
#include "xaction_manager.hpp"

HierarchicalID::HierarchicalID()
    : HierarchicalID(INVALID_TABLEID, INVALID_PAGENUM)
{
    // Do Nothing
}

HierarchicalID::HierarchicalID(tableid_t tid, pagenum_t pid) :
    tid(tid), pid(pid)
{
    // Do Nothing
}

HashableID HierarchicalID::make_hashable() const {
    return HashableID(utils::token, tid, pid);
}

bool HierarchicalID::operator<(HierarchicalID const& other) const {
    return tid < other.tid
        || (tid == other.tid && pid < other.pid);
}

bool HierarchicalID::operator==(HierarchicalID const& other) const {
    return tid == other.tid && pid == other.pid;
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

bool Lock::runnable() const {
    return !wait_flag;
}

Status Lock::wait() {
    wait_flag = true;
    return Status::SUCCESS;
}

Status Lock::run() {
    wait_flag = false;
    return Status::SUCCESS;
}

LockManager::LockStruct::LockStruct() :
    mode(LockMode::IDLE), cv(), run(), wait()
{
    // Do Nothing
}

std::shared_ptr<Lock> LockManager::require_lock(
    Transaction* backref, HID hid, LockMode mode
) {
    HashableID id = hid.make_hashable();
    auto new_lock = std::make_shared<Lock>(hid, mode, backref);

    std::unique_lock<std::mutex> own(mtx);

    auto defer = utils::defer([&] {
        LockStruct& module = locks[id];
        module.mode = mode;
        module.run.push_front(new_lock);

        if (trxs.find(backref->get_id()) == trxs.end()) {
            trxs[backref->get_id()] = std::make_pair(backref, 0);
        }
        trxs[backref->get_id()].second++;
    });

    auto iter = locks.find(id);
    if (iter == locks.end() || lockable(iter->second, new_lock)) {
        // backref and module updates are occured in defer.
        return new_lock;
    }

    backref->wait = new_lock;
    backref->state = TrxState::WAITING;

    new_lock->wait();
    locks[id].wait.push_back(new_lock);

    while (!locks[id].cv.wait_for(
        own,
        LOCK_WAIT,
        [&]{ return new_lock->runnable(); })
    ) {
        Status res = detect_and_release();
        if (res == Status::SUCCESS) {
            locks[id].cv.notify_all();
        }
    }

    backref->wait = nullptr;
    backref->state = TrxState::RUNNING;

    locks[id].wait.remove(new_lock);
    // backref and module updates are occured in defer.
    return new_lock;
}

Status LockManager::release_lock(std::shared_ptr<Lock> lock) {
    std::unique_lock<std::mutex> own(mtx);

    HashableID hid = lock->get_hid().make_hashable();
    auto iter = locks.find(hid);
    CHECK_TRUE(iter != locks.end());

    LockStruct& module = iter->second;
    module.run.remove(lock);

    Transaction& backref = lock->get_backref();
    if (--trxs[backref.get_id()].second == 0) {
        trxs.erase(backref.get_id());
    }

    if (module.run.size() > 0) {
        return Status::SUCCESS;
    }

    if (module.wait.size() == 0) {
        module.mode = LockMode::IDLE;
        return Status::SUCCESS;
    }

    if (module.wait.front()->get_mode() == LockMode::SHARED) {
        for (auto& lock_ptr : module.wait) {
            if (lock_ptr->get_mode() == LockMode::SHARED) {
                lock_ptr->run();
            }
        }
    } else {
        module.wait.front()->run();
    }

    module.cv.notify_all();
    return Status::SUCCESS;
}

Status LockManager::detect_and_release() {
    CHECK_SUCCESS(detector.schedule());

    std::vector<trxid_t> found = detector.find_cycle(locks, trxs);
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
    unit(LOCK_WAIT), last_use(std::chrono::steady_clock::now())
{
    // Do Nothing
}

Status LockManager::DeadlockDetector::schedule() {
    using namespace std::chrono;
    auto now = steady_clock::now();
    return unit <= duration_cast<milliseconds>(now - last_use)
        ? Status::SUCCESS
        : Status::FAILURE;
}

void LockManager::DeadlockDetector::reduce(
    graph_t& graph, trxid_t xid
) const {
    std::set<trxid_t> chained;
    Node& node = graph.at(xid);

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
    for (trxid_t xid : chained) {
        reduce(graph, xid);
    }
}

std::vector<trxid_t> LockManager::DeadlockDetector::find_cycle(
    locktable_t const& locks, trxtable_t const& xtable
) {
    last_use = std::chrono::steady_clock::now();
    graph_t graph = construct_graph(locks, xtable);

    while (graph.size() > 0) {
        auto iter = std::find_if(
            graph.begin(), graph.end(),
            [](auto const& pair) { return pair.second.refcount() == 0; });

        if (iter == graph.end()) {
            unit = LOCK_WAIT;
            return choose_abort(std::move(graph));
        }

        reduce(graph, iter->first);
    }

    unit += LOCK_WAIT;
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
    locktable_t const& locks, trxtable_t const& xtable
) -> LockManager::DeadlockDetector::graph_t {
    graph_t graph;

    for (auto const& iter : xtable) {
        trxid_t xid = iter.first;

        auto const& value = iter.second;
        Transaction* trx = value.first;

        if (trx->wait == nullptr) {
            continue;
        }

        HID wait_id = trx->wait->get_hid();
        LockStruct const& module = locks.at(wait_id.make_hashable());

        for (auto const& run_lock : module.run) {
            trxid_t run_xid = run_lock->get_backref().get_id();
            graph[run_xid].prev_id.insert(xid);
            graph[xid].next_id.insert(run_xid);
        }
    }

    return graph;
}
