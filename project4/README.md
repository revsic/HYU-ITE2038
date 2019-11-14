# Join

Primary key based Natural Join 구현.

## 0. C++ Reimplementation

이번 과제부터 C++의 사용이 가능해져 14년도 표준에 맞추어 DB를 재구현하였다. 기본적으로 [project3](../project3)의 코드 구조를 따랐으며, 세부 구현체는 매크로나 조건부 컴파일 대신 cpp의 feature를 최대한 활용하였다. 

| name | function | source | methods |
| ---- | -------- | ------ | ------- |
| Application | User IO | [main.cpp](./app/main.cpp) | main |
| DBMS | DBMS Integration | [dbms.hpp](./include/dbms.hpp), [dbapi.hpp](./include/dbapi.hpp) | open, close table, key search, insert, delete |
| Operator | Relational operator | [join.hpp](./include/join.hpp) | set based merge |
| Table Manager | Table API | [table_manager.hpp](./include/table_manager.hpp) | load, find, close |
| B+ Tree | Index API | [bptree.hpp](./include/bptree.hpp) | find, insert, delete |
| Buffer Manager | Buffer API | [buffer_manager.hpp](./include/buffer_manager.hpp) | buffering, alloc buffer, free buffer |
| Disk Manager | Disk API | [disk_manager.hpp](./include/disk_manager.hpp) | create, close, pagination, alloc page, free page |

## 1. Primary key based Natural Join



## Works

### TODO
- buffer manager
    - searching policy
    - create, new method run on buffer (not primitive api)

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

### Next project
- buffer manager
    - lock metadata (ex mru lru pin)
    - try lock
- bpt:
    - estimate propagation + partial lock
