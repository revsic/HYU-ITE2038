# Concurrency Control

이번 과제는 Concurrency Control을 위한 Transaction과 Lock Manager 지원을 목표로 한다.

## 0. Real implementation

Reference [README2.md](./README2.md)

## 1, Definition

### 1.1. Transaction

Transaction (이하 TRX) 은 User Defined Instruction Sequence로, ACID의 속성을 가진다.

- Atomicity: 전체 TRX이 실행에 성공하거나, TRX 자체가 실행되지 않는다.
- Consistency: 읽고 쓰는 값의 일관성이 보장된다.
- Isolation: 사용자가 다른 사용자로부터 시스템이 독립되었다 느낀다.
- Durability: 실행에 성공한 Transaction에 대한 결과가 영속적으로 보존된다.

#### 1.1.1. Consistency

값의 일관성은 주어진 값을 여러 TRX이 수정하고자 할 때 위협을 받으며, 이때 일어날 수 있는 문제는 크게 3가지로 나뉜다.

1. lost update: 동시에 두 write operation이 발생하면, 하나의 write operation이 손실될 수 있다.
2. inconsistent read: 수정될 값을 읽을 때 하나의 개체에 대해 읽어온 두 값이 다를 수 있다.
3. dirty read: 아직 confirm 되지 않은 임시 값을 읽을 때 Undo 과정에서 값이 손실될 수 있다.

#### 1.1.2. Isolation

사용자가 다른 사용자로부터 시스템이 독립되었다 느끼는 정도로, 완전히 독립되었다 느끼는 Serializable과 commit 되지 않은 데이터를 읽어올 수 있는 read uncommitted 등 여러 수준을 가진다.

Serializable은 여러 TRX을 동시에 실행했을 때와 차례대로 처리했을 때가 같은 결과를 나타내는 것을 의미한다. 이는 Lock을 통해 데이터의 접근 순서를 강제함으로써 만족시킬 수 있다.

### 1.2. Lock Manager

TRX의 ACID 속성을 보장하기 위해 시스템 전체에 대한 접근을 Lock을 통해 제어하는 장치이다.

#### 1.2.1. Accessibility

Lock은 TRX이 Database, Table, Page, Record 등 대상 개체에 접근하는 것을 제한하며, TRX이 요청한 권한과 대상 개체에 접근 중인 TRX에서 접근 안전성을 고려하여 대기 혹은 접근의 결정을 내린다.

권한은 크게 Shared와 Exclusive로 나뉘며, 개체를 수정하지 않아 다른 TRX과 접근을 공유할 수 있는 경우 Shared, 개체에 대한 추가 처리가 이루어져 다른 TRX과 접근을 공유할 수 없는 경우 Exclusive 권한을 요청하게 된다.

Shared 권한을 요청한 TRX은 서로 접근을 공유할 수 있으며, Exclusive 권한을 요청한 경우 다른 TRX이 접근할 수 없다. Lock은 이러한 권한과 접근 상태를 통해 접근 안전성을 고려한다.

#### 1.2.2. Deadlock

서로 다른 TRX이 상대방이 소유한 개체에 대한 접근을 기다릴 때 두 TRX은 무한정 서로를 기다리게 되는데, 이를 Deadlock이라고 한다. Lock Manager은 주기적으로 혹은 매번 Lock을 처리할 때마다 TRX 사이에 Lock Cycle이 존재하는지 확인하고, 존재할 경우 Cycle을 풀기 위해 Abort 할 TRX을 지정하여 실행을 취소해야 한다.

## 2. Design

### 2.1. Lock Structure

현재 구상 중인 Lock의 자료 구조는 다음과 같다. 권한을 표현하기 위한 Lock Mode와 Lock을 설치하는 유닛의 단위로 Lock Level을 두었다. 이는 추후 Record 수가 기하급수적으로 증가할 때보다 상위 레벨에서부터 Lock을 잡아오는 방식을 구현하기 위함이다.

```c++
enum class LockMode { SHARED = 0, EXCLUSIVE = 1 };

enum class LockLevel { INVALID = 0, DATABASE = 1, TABLE = 2, PAGE = 3, RECORD = 4 };

struct Lock {
    tableid_t tid;
    pagenum_t pid;
    int record_idx;
    LockMode mode;
    LockLevel level;
    Transaction* backref;
};
```

Lock은 각각 어디에 설치되었는지를 나타내기 위해 Table ID tid, Page ID pid, Record Index record_idx로 구성되어 있으며, 해당 Lock이 현재 어떤 Transaction에 연결되어 있는지 확인하기 위해 back reference 포인터를 추가하였다.

```c++
struct HashablePID {
    tableid_t tid;
    pagenum_t pid;
};

struct LockManager {
    std::mutex mtx;
    std::unordered_map<HashablePID, std::list<Lock>> page_locks;
}
```

Lock Manager은 Lock Level에 따라 Hash Map을 통해 키와 lock을 관리하고, TRX의 요구에 따라 lock을 생성, 대기 상태 활성화, 접근 제어, deadlock 관리를 진행한다. 필요에 따라 bitmap을 통해 record level lock을 page level에서 구현하는 방식도 고려하고 있다.

### 2.2 Transaction structure

트랜잭션은 생성과 즉시 TRX ID를 부여받으며, Operation 중에는 Lock Manager와 통신하며 작업에 필요한 Lock을 획득, 사용자가 end_trx() 메소드를 호출하기 전까지 획득한 lock을 보존하는 역할을 한다. 이 과정에서 end_trx()까지 정상 호출되면 lock을 release하고, 그 전에 deadlock이 발생한 경우 abort 하게 된다.

```c++
enum class TrxState { IDLE = 0, RUNNING = 1, WAITING = 2 };

struct Transaction {
    trxid_t id;
    TrxState state;
    Lock* wait;
    std::list<Lock*> locks;
};
```

TRX Manager은 global lock을 가지고 있다가 새로운 TRX을 요청받으면 last id를 통해 TRX ID를 발급, 새 트랜잭션을 Hash Map에 추가한다. 이는 end_trx() 나 abort_trx()함수가 호출될 때까지 유지되며, end_trx()와 함께 commit 되거나 abort_trx와 함께 abort 되는 Transaction의 주기를 관리한다.

```c++
struct TransactionManager {
    std::mutex mtx;
    trxid_t lastid;
    std::unordered_map<trxid_t, Transaction> trxs;
};
```

### 2.3. Deadlock detection

Deadlock detection은 Lock의 backref와 Transaction이 가지고 있는 lock list를 통해 가능하다.

하나의 TRX을 선택하고, 소유하고 있는 lock을 순회한다. 해당 lock이 가리키는 page ID의 lock 리스트에서 backref의 wait ptr이 하나라도 선택된 TRX의 lock 리스트의 element를 가리킨다면 Deadlock이므로 Abort 한다. 이는 재귀적으로 다음 TRX에 대해서도 검사해 나갈 수 있으며, N개의 TRX이 연관된 Cycle을 찾을 수 있게 한다. 이는 Graph 탐색 문제이므로 DP를 통해 최적화할 수도 있을 듯하다.

### 2.4. Buffer Pin and RWLock

일전에 buffer의 pin이 RW Lock의 역할을 할 것을 이야기한 적이 있다. 실제로 read에서는 접근을 공유할 수 있었고, write에서는 하나의 접근만이 용납된다는 점에서 RW Lock과 같다. 현재는 단순 int로 이루어져 있으므로 `std::atomic<int>` 혹은 `std::shared_lock`을 통해 thread-safe 한 pin을 구현할 예정이다.

### 2.5. Buffer Manager

Buffer manager에 대해서는 현재 MRU, LRU 리스트 보존, buffer searching, allocating 과정이 thread-safe 하지 않기 때문에 global mutex를 추가해서 안전성을 확보해야 한다.

### 2.6. Strict 2-Phase-Lock

S2PL 구현을 위해서 TRX는 operation 과정에서 lock을 잡기만 할 뿐 release 하지 않기로 하였다. 추후 TRX 성공 및 Commit 후에 Lock을 Release 하는 방식으로 구현할 예정이다.

## Works

### TODO
- dbms
    - shutdown immediately

### Future work
- Concurrency Control
    - insert, delete
- query parser
- query optimizer
- relational operators

### Done
- headers
    - Add type definition
    - type size test (all pass)
- fileio
    - file level IO
    - unit test (all pass)
- file manager
    - file manager (disk management layer)
    - unit test (all pass)
- buffer manager
    - buffer manager
    - unit test (all pass)
- bpt
    - on-disk b+tree
    - support record based iterator
    - unit test (all pass)
- table manager
    - table manager.
    - unit test (all pass)
- join
    - primary key based natural join
    - unit test (all pass)
- lock manager
    - lock manager for database concurrency control.
    - unit test (all pass)
- transaction manager
    - managing transaction.
    - unit test (not implemented)
- log manager
    - logging result in in-memory structure.
    - unit test (all pass)
