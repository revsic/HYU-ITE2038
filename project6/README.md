# Crash Recovery

Log based crash recovery

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
    - unit test (not implemented yet)
- log manager
    - logging result in in-memory structure.
    - unit test (all pass)
