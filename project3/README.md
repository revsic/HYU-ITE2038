# [WIP] Buffer management system

- 본 프로젝트는 개발이 진행중인 프로젝트입니다. 진행 사항은 [0. Works](0-works)에서 확인해 주세요.

On-Disk B+ Tree의 operation 가속화를 위한 Buffer Management Layer 구현.

## 1. Layered Architecture

[이전](../project2) 프로젝트에서는 Layered Architecture 설계에 따라 On-Disk B+ Tree를 구현하였다. 이번 프로젝트는 On-Disk B+Tree 가속화를 위한 Buffer Management Layer 구현을 목적으로 한다.

| name | function | source | methods |
| ---- | -------- | ------ | ------- |
| Application | User IO | [main.c](./src/main.c) | main |
| Table Manager | Table API | [table.h](./include/table.h), [table_manager.h](./include/table_manager.h) | load, find, close |
| B+ Tree | Index API | [bpt.h](./include/bpt.h) | find, insert, delete |
| Buffer Manager | Buffer API | [buffer_manager.h](./include/buffer_manager.h) | buffering, alloc buffer, free buffer |
| Disk Manager | Disk API | [disk_manager.h](./include/disk_manager.h) | create, close, pagination, alloc page, free page |

## 0. Works

### TODO
- file manager
    - filename as searching, file id as identification
- buffer manager
    - split searching policy
    - some unimplemented unit tests
    - lock metadata (ex mru lru pin)
- bpt
    - estimate propagatino + partial lock
    - bug fix
    - unit test
    - ubuffer: only pass by value
- table
    - unit test
    - seperate table id and file id
    - table_load: Nullability of argument buffer manager
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
