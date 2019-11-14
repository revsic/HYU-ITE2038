# Join

Primary key based Natural Join 구현.

## 0. C++ Reimplementation

이번 과제부터 C++의 사용이 가능해져 14년도 표준에 맞추어 DB를 재구현하였다. 기본적으로 [project3](../project3)의 코드 구조를 따랐으며, 세부 구현체는 매크로나 조건부 컴파일 대신 cpp의 feature를 최대한 활용하였다. 다만, 아직 최적화를 진행하지 않아, 기존의 C 구현체보다 2배 정도 속도가 느려졌다.

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

이번 과제는 primary key를 기준으로 Record를 저장하는 B+Tree based index와 Buffer를 통해 natural join을 구현하는 것이다. 

가장 먼저 생각한 것은 sort merge join이다. 현재 인자로 받아오는 두 테이블 모두 B+Tree 기반의 index 구조에 Record가 저장되어 있고, primary key에 대해 정렬된 상태이다. 

Sort-Merge join은 두 테이블이 정렬된 상태에서 Merge operation을 통해 같은 key를 가진 record를 가져오는 join 방식이다. 현재 Table이 모두 primary key에 대해 정렬되어 있어 조건을 만족하므로, Merge만을 구현하여 Natural join을 구현할 수 있었다.

더군다나, 현재 index가 중복을 지원하지 않아 multiset이 아닌 set을 기반으로 한 merge가 가능하고, O(#table1 + #table2) 시간 안에 Join이 가능하다. 

[join.hpp](./include/join.hpp)에 `set_merge`가 구현되어 있고, callback을 받아 key가 같은 record에 대해서 추가 operation이 가능하도록 구현하였다. 

```c++
if (key1 == key2) {
    CHECK_SUCCESS((*iter1).use(RWFlag::READ, [&](Record& rec1) {
        return (*iter2).use(RWFlag::READ, [&](Record& rec2) {
            return callback(rec1, rec2);
        });
    }))
}
```

또한 더 단순한 구현을 위해 B+Tree의 Record를 순회할 수 있는 BPTreeIterator 을 만들어 buffering을 직접 호출하지 않고도 record에 대한 순회가 가능하도록 하였다. 

```c++
for (auto record_ref : *table1) {
    std::cout << record_ref.key() << std::endl;
}
```

현재 join 구현체에서는 range based for보다는 직접 iterator을 들고 순회하는 게 더 좋을 듯하여 iterator을 저장하고 있다.

```c++
auto iter1 = table1->begin();
auto iter2 = table2->begin();

auto end1 = table1->end();
auto end2 = table2->end();
```

이는 [dbms.hpp](./include/dbms.hpp)에 `Database::prikey_join`으로 wrapping 되어 있고, csv 포맷으로 저장하기 위해 [dbapi.hpp](./include/dbapi.hpp)의 join table에서는 record를 csv 포맷으로 저장하는 callback을 `prikey_join`에 인자로 주고 있다.

```c++
Status res = GLOBAL_DB->prikey_join(table_id_1, table_id_2,
        [&](Record& rec1, Record& rec2) {
            fprintf(
                fp,
                "%d,%s,%d,%s\n",
                rec1.key, rec1.value,
                rec2.key, rec2.value);
            return Status::SUCCESS;
        });
```

실행 예로, testfile(tid=121764155)은 {1, 3, 7, 8, 10}을, datafile(tid=1706433886)은 {1, 2, 4, 7, 10}의 primary key를 가질 때 output.txt의 내용이다.

```
>o testfile
tid: 121764155
1 3 7 8 10 | 
>t
1 3 7 8 10 | 
>o datafile
tid: 1706433886
1 2 4 7 10 | 
>j 1706433886 121764155 output.txt
```

```
1,1 value,1,1 value
7,7 value,7,7 value
10,10 value,10,10 value
```

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
