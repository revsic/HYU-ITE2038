# Concurrency Control

이번 과제는 Concurrency Control을 위한 Transaction과 Lock Manager 지원을 목표로 한다.

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



## Works

### TODO
- buffer manager
    - renew structure (too complicated)
    - searching policy (free list, allocated list)
    - new_page, free_page method run on buffer (not primitive api)
    - concurrency control
        - global lock
        - pin -> rwlock
- bptree
    - update method
- transaction manager
    - all
    - unit test
- lock manager
    - all
    - unit test

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
