# [WIP] Buffer management system

- 본 프로젝트는 개발이 진행중인 프로젝트입니다. 진행 사항은 [Works](#works)에서 확인해 주세요.

On-Disk B+ Tree의 operation 가속화를 위한 Buffer Management Layer 구현.

## 0. Layered Architecture

[이전](../project2) 프로젝트에서는 Layered Architecture 설계에 따라 On-Disk B+ Tree를 구현하였다. 이번 프로젝트는 On-Disk B+Tree 가속화를 위한 Buffer Management Layer 구현을 목적으로 한다.

| name | function | source | methods |
| ---- | -------- | ------ | ------- |
| [Application](#6-application) | User IO | [main.c](./app/main.c) | main |
| [DBMS](#5-dbms) | DBMS Integration | [dbms.h](./include/dbms.h), [dbapi.h](./include/dbapi.h) | init, shutdown db, open, close table, key search, insert, delete |
| [Table Manager](#4-table-manager) | Table API | [table.h](./include/table.h), [table_manager.h](./include/table_manager.h) | load, find, close |
| [B+ Tree](#3-b-tree) | Index API | [bpt.h](./include/bpt.h) | find, insert, delete |
| [Buffer Manager](#2-buffer-manager) | Buffer API | [buffer_manager.h](./include/buffer_manager.h) | buffering, alloc buffer, free buffer |
| [Disk Manager](#1-disk-manager) | Disk API | [disk_manager.h](./include/disk_manager.h) | create, close, pagination, alloc page, free page |

## 1. Disk Manager

 파일 관리를 목적으로 하는 레이어이다. 파일을 Page 크기로 나누고, 각 페이지에 File Metadata, Leaf Node, Internal Node의 역할을 부여한다. 자세한 내용은 [Project2](../project2)를 참조한다.

## 2. Buffer Manager

On-disk operation 가속화를 위한 Buffer 관리 모듈이다. 

## 3. B+ Tree

B+Tree 구조의 인덱스 관리를 위한 레이어이다. Buffer Manager를 통해 디스크에서 페이지 프레임을 읽어 오고, 각 페이지의 Tree Node를 토대로 주어진 Key의 검색, 범위 검색, 삽입, 삭제 기능을 지원한다. 자세한 내용은 [Project2](../project2)를 참조한다.

## 4. Table Manager

파일 단위의 Table을 관리하는 레이어이다.

## 5. DBMS

Buffer Manager과 Table Manager를 합쳐 Database Management System을 구현한 레이어이다. 테이블을 DBMS에 올려 Key의 삽입, 삭제, 검색을 지원한다. 

## 6. Application

사용자와 상호작용을 하는 레이어이다. 사용자의 입력을 받아 테이블을 DBMS에 올리고, Key의 삽입, 삭제, 검색 기능을 지원한다.

## Works

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
