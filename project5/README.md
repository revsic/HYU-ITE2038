# Transaction Manager

이번 과제는 Concurrency Control을 위한 Transaction과 Lock Manager 지원을 목표로 한다.

## Transaction Manager

## Lock Manager

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
