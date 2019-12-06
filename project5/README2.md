# Concurrency Control: Real implementation

Concurrency 모듈의 실구현체에 대해 설명한다.

## 1. Log Manager

Log Manager은 Transaction이 Abort 되었을 때 Update log를 기반으로 Undo recovery를 수행한다. 

```c++
class LogManager {
public:
    ...

    lsn_t log_update(
        trxid_t xid, HID hid, int offset,
        Record const& before, Record const& after);

private:
    std::mutex mtx;
    lsn_t last_lsn;
    std::unordered_map<trxid_t, std::list<Log>> log_map;

    ...
};
```

각각의 Log를 Transaction에 따라 관리하고 있으며, BPTree에서는 update method를 수행하기 전에 `log_update`를 통해 update 관련 정보를 기록한다.

현재에는 crash recovery를 고려하지 않기 때문에 no force 정책에 따라 Commit 되지 않은 TRX에 대해서도 버퍼가 디스크에 페이지를 기록하는 것을 허용하며, 모든 로그 기록은 In-Memory 방식으로 관리된다.

TRX Abort 상황에서의 Undo Recovery는 전적으로 Transaction 모듈에서 관리하며, LogManager는 Transaction에 `std::list<Log>` 형태의 로그를 제공하는 역할만을 수행한다.

## 2. Lock Manager

Lock Manager은 Page 단위로 Lock을 제공하며, `HierarchicalID`라는 이름의 객체로 Table ID와 Page ID를 묶어 관리하게 된다.

```c++
class Lock {
public:
    ...
private:
    HID hid;
    LockMode mode;
    Transaction* backref;
    std::atomic<bool> wait_flag;
};
```

Lock은 다음과 같이 구성되며, 자신이 가리키는 대상인 HID, 권한을 나타내는 LockMode, Lock 소유주인 `backref`, 현재 Lock 상태가 대기 중 인기 기록하는 `wait_flag`로 구성되어 있다. 

```c++
class LockManager {
public:
    ...
private:
    std::mutex mtx;
    std::unordered_map<HierarchicalID, LockStruct> locks;
    std::unordered_map<trxid_t, std::pair<Transaction*, int>> trxs;
    DeadlockDetector detector;
    Database* db;
};
```

LockManager은 HID를 기준으로 Lock의 집합인 `LockStruct`를 구성한다.

```c++
struct LockStruct {
    LockMode mode;
    std::conditional_variable cv;
    std::list<std::shared_ptr<Lock>> run;
    std::lsit<std::shared_ptr<Lock>> wait;
}
```

LockStruct는 현재 페이지에 할당된 접근 권한, 실행 중인 Lock과 대기 중인 Lock을 표현한다. 또 모든 LockStruct에는 개별 conditional_variable이 존재하는데, 이는 Page 단위로 대기 중인 Lock을 깨워 본인이 실행 가능한지 확인할 수 있게 하기 위함이다.

LockManager에는 부가적으로, 현재 모듈이 관리 중인 Lock의 전체 소유주와 몇 번의 reference count를 가지는지 나타내는 `trxs`와 Deadlock 탐색 관련 모듈인 `Deadlock Detector`를 가진다. `trxs`를 따로 관리하는 이유는 Deadlock 탐색에 필요한 Waiting-Graph를 더 쉽게 구현하기 위함이다.

```c++
struct DeadlockDetector {
    struct Node {
        std::set<trxid_t> next_id;
        std::set<trxid_t> prev_id;
    };

    std::chrono::microseconds unit;
    std::chrono::time_point<std::chrono::steady_clock> last use;
}
```

Deadlock Detection은 크게 실제 탐색 알고리즘과 스케줄링 알고리즘으로 구분된다.

Deadlock Detection은 Transaction 간 종속 정도를 나타내는 Waiting-Graph를 작성하여 Topological sort와 유사히 우선 In-Degree가 0인 노드를 정리한다. 이때 모든 노드가 정리되면 해당 Waiting-Graph가 DAG이므로 Deadlock이 존재하지 않는다.

하지만 모든 노드가 정리되지 않으면 Graph 내부에 Cycle이 존재하고 이는 Deadlock을 의미하므로 abort 해야 하는 후보군을 지정해야 한다. 

`choose_abort` 메소드에서는 in-degree를 우선, out-degree를 차선으로 비교하여 가장 연결이 많은 노드를 찾아 abort 후보군에 추가하고 node를 삭제, 연쇄적으로 in-degree가 0이 되는 노드를 하나씩 지워가며 그래프에 노드가 남아 있지 않을 때까지 반복한다. 이후 abort 후보군에 있는 TRX를 하나씩 Abort 한다.

Scheduling은 매번 graph를 구성 reduce 하는 방식이 많은 시간을 소용할 듯하여, 마지막 detection에서 deadlock이 존재했는지 아닌지를 따져 존재했다면 `DeadlockDetector::unit`을 기본값으로 초기화, 아닌 경우 대기 시간을 늘려가며 효율적으로 graph를 탐색하기 위한 방법론이다.

## 3. Transaction Manager.

Transaction Manager은 monotonically increasing order에 따라 Transaction ID를 부여하고, 사용자가 XID를 통해 Transaction을 제어할 수 있도록 돕는다.

```c++
class TransactionManager {
public:
    ...
private:
    mutable std::mutex mtx;
    LockManager* lock_manager;

    trxid_t last_id;
    std::unordered_map<trxid_t, Transaction> trxs;
};
```

각각의 Transaction은 대기 중인 Lock, 소유 중인 Lock, 현재 TRX의 상태, XID 등을 기록하고 있다.

```c++
class Transaction {
public:
    ...
private:
    std::unique_ptr<std::mutex> mtx;
    trxid_t id;
    TrxState state;
    std::shared_ptr<Lock> wait;
    std::map<HID, std::shared_ptr<Lock>> locks;
};
```

사용자가 Lock을 요구하는 경우 Transaction Manager은 Transaction에 LockManager과 함께 HID, LockMode 를 전달하여 Transaction이 Lock을 요청할 수 있도록 한다.

`acquire_lock`의 경우 크게 처음으로 Page에 lock을 요청한 경우와 이미 lock을 보유하고 있지만, 권한을 더 강하게 잡아야 하는 경우로 나뉘며, 이는 `elevate_lock` 메소드에서 처리하였다.

DeadlockDetector에 의해서 Abort를 해야 할 때에는 `Database` 구조체에서 LogManager를 불러와 관련 Update Log에 따라 복원을 진행한다. 이후 소유하고 있던 Lock을 모두 release 하면서 루틴을 종료한다.
