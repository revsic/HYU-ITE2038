# Join

## Works

### TODO
- buffer manager
    - searching policy
    - create, new method run on buffer (not primitive api)
- bpt:
    - ubuffer: only pass by value
- table
    - seperate table id and file id
    - unit test
- table manager
    - unit test
- all
    - replace return type of procedure which use SUCCESS

### Done
- headers
    - Add type definition
    - type size test (all pass)
- utility
    - function for simplicyfing member variable access
    - unit test (all pass)
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
    - unit test (all pass)

### Next project
- buffer manager
    - lock metadata (ex mru lru pin)
    - try lock
- bpt:
    - estimate propagation + partial lock
