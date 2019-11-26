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

Lock::Lock() : hid(), mode(LockMode::INVALID), backref(nullptr) {
    // Do Nothing
}

Lock::Lock(HierarchicalID hid, LockMode mode, Transaction* backref)
    : hid(hid), mode(mode), backref(backref)
{
    // Do Nothing
}

HierarchicalID Lock::get_hid() const {
    return hid;
}

Status LockManager::require_lock(
    Transaction* backref, HierarchicalID hid, LockMode mode
) {
    return Status::SUCCESS;
}

Status LockManager::release_lock(HierarchicalID hid) {
    return Status::SUCCESS;
}

Status LockManager::detect_deadlock() {
    return Status::SUCCESS;
}

Status LockManager::detect_and_release() {
    return Status::SUCCESS;
}
