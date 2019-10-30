# [WIP] Buffer management system

- 본 프로젝트는 개발이 진행중인 프로젝트입니다. 진행 사항은 [Works](#works)에서 확인해 주세요.

On-Disk B+ Tree의 operation 가속화를 위한 Buffer Management Layer 구현.

## 0. Layered Architecture

[이전](../project2) 프로젝트에서는 Layered Architecture 설계에 따라 On-Disk B+ Tree를 구현하였다. 이번 프로젝트는 On-Disk B+Tree 가속화를 위한 Buffer Management Layer 구현을 목적으로 한다.

| name | function | source | methods |
| ---- | -------- | ------ | ------- |
| [Application](#6-application) | User IO | [main.c](./app/main.c) | main |
| [DBMS](#5-dbms) | DBMS Integration | [dbms.h](./include/dbms.h), [dbapi.h](./include/dbapi.h) | open, close table, key search, insert, delete |
| [Table Manager](#4-table-manager) | Table API | [table.h](./include/table.h), [table_manager.h](./include/table_manager.h) | load, find, close |
| [B+ Tree](#3-b-tree) | Index API | [bpt.h](./include/bpt.h) | find, insert, delete |
| [Buffer Manager](#2-buffer-manager) | Buffer API | [buffer_manager.h](./include/buffer_manager.h) | buffering, alloc buffer, free buffer |
| [Disk Manager](#1-disk-manager) | Disk API | [disk_manager.h](./include/disk_manager.h) | create, close, pagination, alloc page, free page |

## 1. Disk Manager

 파일 관리를 목적으로 하는 레이어이다. 파일을 Page 크기로 나누고, 각 페이지에 File Metadata, Leaf Node, Internal Node의 역할을 부여한다. 자세한 내용은 [Project2](../project2)를 참조한다.

## 2. Buffer Manager

On-disk operation 가속화를 위한 Buffer 관리 모듈이다.

Buffer Manager은 요청에 따라 주어진 파일에서 Page를 버퍼에 올려 반환한다. 이 페이지 프레임은 버퍼 매니저가 종료하거나, Page Replacement Policy에 의해 다른 페이지 프레임으로 교체되지 않는 이상 메모리를 유지하여 사용자가 메모리에서 연산을 진행하는 듯한 illusion을 제공한다. 

### 2.1. Pluggable policy

Buffer Manager은 정책에 따라 버퍼의 검색이나 교체를 지원한다.

1. [WIP] Buffer searching policy

진행 중이다.

2. Buffer replacement policy

```c
/// Page replacement policy.
struct release_policy_t {
    int(*initial_search)(struct buffer_manager_t* manager);     /// initial searching state.
    int(*next_search)(struct buffer_t* buffer);                 /// next buffer index.
};
```

버퍼 교체 정책은 다음과 같이 정의되며, `buffer_manager_release` 에서는 policy structure을 인자로 받아 교체할 페이지를 찾아 나간다. `initial_search`를 통해 탐색을 시작할 페이지를 결정하고, `next_search`를 통해 검색을 진행한다. 

현재에는 `RELEASE_LRU`로 정의된 LRU 정책에 따라 기본적인 페이지 교체를 지원한다. 사용하지는 않지만 `RELEASE_MRU`로 정의된 MRU 정책도 구현되어 있다. 

### 2.2. BUFFER macro

현재 버퍼 매니저는 버퍼의 사용 시작과 끝에 `pin` 변수를 통한 Rwlock 기능을 지원한다. 이를 단순화한 매크로가 `BUFFER` 매크로이다. 

```c
BUFFER(buffer, WRITE_FLAG, { 
    struct page_header_t* header =
        page_header(from_ubuffer(buffer));
    header->parent_page_number = INVALID_PAGENUM;
 })
```

다음과 같이 버퍼, RW_FLAG, statements를 통해 버퍼의 사용을 조정한다. 다른 스레드에서 버퍼를 읽고 있을 경우, 현재 스레드에서 버퍼를 읽을 수는 있지만, 쓰기 위해서는 다른 스레드가 버퍼를 반환하기까지 기다려야 한다. 반면에 버퍼를 쓰고 있을 경우에는 읽기와 쓰기가 모두 지연된다.

```c
BUFFER(buffer, WRITE_FLAG, {
    if (condition) {
        BUFFER_INTERCEPT(buffer, WRITE_FLAG, return FAILURE);
    }
    contents;
})
```

버퍼 사용 중 이용을 조기 종료하기 위해서 INTERCEPT 매크로를 추가하였다. 이는 버퍼 사용을 종료하고 주어진 컨텐츠를 실행하는 역할을 한다.

### 2.3. Buffer for user provision

현재 버퍼 매니저에서는 내부적으로 쓰이는 `struct buffer_t`와 사용자에게 제공되는 `struct ubuffer_t`가 분리된다.

`ubuffer_t`는 버퍼가 교체될 상황을 대비해 `check_ubuffer`의 버퍼 검증 기능과 `reload_ubuffer`의 버퍼 재호출 기능을 제공한다. 이는 `BUFFER` 매크로에서도 지원하며, 사용자가 버퍼를 사용하는 도중 누락되지 않게 돕는다.

## 3. B+ Tree

B+Tree 구조의 인덱스 관리를 위한 레이어이다. Buffer Manager를 통해 디스크에서 페이지 프레임을 읽어 오고, 각 페이지의 Tree Node를 토대로 주어진 Key의 검색, 범위 검색, 삽입, 삭제 기능을 지원한다. 자세한 내용은 [Project2](../project2)를 참조한다.

## 4. Table Manager

파일 단위의 Table을 관리하는 레이어이다. 파일명을 기반으로 테이블을 열어 매니저에 추가하고, Table ID를 기반으로 테이블의 검색과 삭제를 지원한다.

### 4.1. Table name hashing

Table ID는 같은 테이블에 대해서 같은 값을 가져야 한다. 가장 단순히 구현할 수 있는 방법으로는 파일명을 기반으로 hash 값을 구하는 것이다.

1. Disk Manager 에서 파일명의 hash 값을 구하고 이를 File ID로 이용한다. (create_filenum)
2. Table Manager 에서는 File ID를 기반으로 Table ID를 생성한다. (table_id_from_filenum)
3. 이 때 Table Manager 내에서의 hash collision을 방지하기 위해 double hash probing을 지원한다. (table_rehash)

현재 위 방법론을 구현하고 있지만, 방법론 자체에 문제가 존재한다. Table이 종료된 후 hash table에서 해당 파일의 hash value가 삭제된다. Table을 여는 과정에서 double hashing이 진행되었다 가정할 때, 이 후 Table을 다시 열었을 때 double hashing을 진행하지 않는다면 서로 다른 Table ID를 가져 Table Manager에 두개의 같은 테이블이 올라가게 된다. 

이를 방지하기 위해 파일명을 보관하고 있거나, probing 방식을 double hashing이 아닌 linked list로 나열하는 방식을 차용할 예정이다. 

### 4.2. Pluggable policy

Buffer 때와 마찬가지로 table searching policy를 교체할 수 있게 구현해 두었다. 현재는 linear search를 하며, 추후 테이블의 개수가 많아질 경우 hash 값에 따른 상수 시간 검색을 지원할 예정이다. 

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
    - parametrize searching policy
    - some unimplemented unit tests
    - lock metadata (ex mru lru pin)
    - try lock
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
