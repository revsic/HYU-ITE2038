# On-Disk B+ Tree implementation

On-Disk B+ Tree 구현 및 디자인 소개.

## 0. Layered Architecture

Software의 설계에 있어 필요한 기능들을 bottom-layer 부터 top-layer로 horizontal하게 구성하여, 각각의 layer가 독립적으로 요청받는 기능들을 수행하는 구조이다. 이 때 각각의 레이어는 타 레이어의 기능을 침해하지 않고, 창구로 기능을 요청하는 것 외에 구현에 관여할 수 없다. 

On-Disk B+ Tree에서는 세부히 4개의 Layer로 나누어 보았다. 

| 이름 | 기능 | 파일명 | 대표 Method |
|---|---|---|---|
| Application | 사용자 입출력 | [main.c](./src/main.c) | main |
| B+ Tree | Index 관리 및 Search, Insert, Delete 연산 지원 | [bpt.h](./include/bpt.h) [bpt.c](./src/bpt.c) | find, find_range, insert, delete |
| Disk Manager | Page structure 구현, Page 단위 입출력 | [disk_manager.h](./include/disk_manager.h) [disk_manager.c](./src/disk_manager.c) | file_open, page_read, page_write |
| Disk IO | BYTE단위 입출력, 파일 유틸 함수 | [fileio.h](./include/fileio.h) [fileio.c](./src/fileio.c) | fpwrite, fpread, fsize, fresize |

## 1. Disk IO Layer

Disk와 직접적으로 입출력 하는 Layer이다. Unix계열 운영체제와 Windows계열 운영체제의 입출력이 조금씩 다르기에 이 부분을 Wrapping한 레이어이다. 

기본적으로 `stdio.h`가 `FILE*`을 기반으로 표준 입출력을 정의해 두었기 때문에 많은 부분을 Wrapping할 필요는 없었다. `unistd`에서 파일의 존재 여부를 확인하는 `access`와 같이 일부 OS마다 헤더나 구현체가 다른 부분에 대해서만 구현하였다. 

현재는 Unix 기반의 IO만 구현되어 있으며, 추후 조건부 컴파일을 통해 Windows 기반의 IO를 추가할 예정이다. 

## 2. Disk Manager Layer

Disk IO를 통해 실제 B+Tree가 활용할 On-Disk Structure를 구성한 레이어이다. Structure 선언체는 [header.h](./include/headers.h)에서 확인할 수 있다. 

Disk Manager Layer은 하나의 B+Tree가 하나의 파일 안에서 동작할 수 있도록 관리하며, 이를 총괄하는 구조체가 `struct file_manager_t`와 하위 메소드들이다. 

`struct file_manager_t`는 `file_open`함수에 index파일을 열거나 생성하며, 해당 파일에 대한 포인터와 `struct file_header_t`를 캐싱하게 된다. 이는 하나의 파일이 열려 있을 때, 해당 파일에 대한 파일 헤더를 읽지 않고, in-memory로 보관, 수정하여 한번에 write 하는 일종의 buffering 기능을 제공하기 위함이다.

`struct file_manager_t`는 앞서 이야기 하였듯이 page 단위의 IO를 지원하는데, 새로운 페이지를 생성하는 `page_create`, 삭제하는 `page_free`, 입출력 함수 `page_read`, `page_write`를 제공한다. 상위 레이어에서는 해당 함수를 통해 page 단위 입출력만을 진행해야 한다.

일전의 [README.md](./README.md)에서는 여러개의 파일을 하나의 B+Tree에서 관리하는 Multi-File 기반의 시스템을 구상하였지만, specification에서 하나의 파일에 하나의 B+Tree를 구현하라는 명세가 존재하여 해당 시스템을 간략화 하여 단일 파일에 대한 B+Tree기반을 구상하게 되었다.

## 3. B+Tree Layer

Disk Manager를 통해 B+Tree를 구현한 Layer이다. 기존의 In-Memory 기반의 B+Tree에서 Disk Manager를 통해 On-Disk를 수정하였다.

일전의 [README.md](./README.md)에서 Node와 Page가 1:1 대응 관계가 존재하기 때문에 규칙에 맞게 변환만 하면 될 것이라 이야기 하였었고, 실제로 internal node에서 인덱스를 한칸씩 밀고, 기존의 0번 인덱스의 자식 노드를 `page_header_t::special_page_number`로 치환하는 정도의 작업을 진행하였다.

작업 과정에서 발생했던 이 외의 문제에 대해서는 하위 섹션에서 이야기 한다.

### 3.1. Order

기존의 In-Memory B+Tree에서는 Leaf의 Order와 Internal Node의 Order가 동일하게 구성되었다. 하지만, On-Disk 버전에서는 그 효율성을 극대화 하기 위해, 하나의 Page 내에 31개의 Record 혹은 248개의 Entry (Key, Pagenum)가 존재할 수 있도록 구성하였으므로, 기존과는 달리 Leaf Order와 Internal Order를 따로 두어야 한다.

해당 부분에 대해서는 `insert_into_leaf_after_splitting`과 같이 leaf node에 관여하는 메소드에서는 `LEAF_ORDER`를, `insert_into_node_after_splitting`나 `insert_into_parent`와 같이 internal node에 관여하는 메소드에서는 `INTERNAL_ORDER`를 사용하는 방식으로 쉽게 대체할 수 있었다.

### 3.2. Delayed Merge

하나의 B+Tree에 병렬접근할 경우 여러 노드에 Lock을 걸어야 하는 상황에 효율이 떨어질 수밖에 없는데, 이에 가장 대표적인 상황에 Merge operation (coalesce operation)이 있다.

이러한 상황을 타파하기 위해 Merge operation의 실행 횟수를 가능한 느리게 빼는 방법이 있으며, 이를 Delayed Merge라고 한다. 

처음 고안했던 방법으로는 Shadow flag를 설치하여 해당 레코드를 직접 삭제하지 않고, flag를 set하여 해당 레코드가 지워진 척 하는 것이다. 이후 `clean_up` 메소드를 호출하여 B+Tree 전체를 lock하고, 한번에 Shadow flag가 set된 레코드, 엔트리들을 삭제하게 된다. 이전까지는 하나의 노드가 모두 shadow flag로 set 되기 전까지 Node level operation (Merge, Redistribute)를 실행하지 않고, 그 자리를 insert 과정에서 덮어쓰거나 find 과정에서 무시하는 방식으로 workflow를 구성해 나가는 것이다.

하지만 이 또한 specification에서 지정되지 않은 플래그를 헤더에 추가할 수 없어 더 방법을 간략화 하였다. delete 요청이 왔을 때 해당 노드를 실제로 삭제하되, 동일히 노드에 키가 하나도 없을 때에만 Merge 혹은 Redistribute operation을 실행하는 것이다. 

단, 이러한 경우 B+Tree의 Height을 더 수동적으로 조절해야 한다는 단점을 가지고 있다. 이는 병렬처리의 속도 이점과 Height의 평균적인 조절 수준을 비교하여 적용해 보는게 옳다고 생각하나, 현재에 있어서는 Delayed Merge를 적용해 놓은 상태이다.

### 3.3 Global Variables

전역 변수나 상태 변수를 사용하게 되면, context manager를 추가로 둬야 하는 overhead를 가지게 된다. 이에 [bpt.h](./include/bpt.h)에서는 모든 함수가 전역 변수를 참조하지 않고, 필요한 정보를 모두 인자로 받게 구성되어 있다. 

다만, specification에서 `open_table`, `db_insert`, `db_delete`, `db_find`가 전역 변수를 통해 file을 하나 열어 작업하는 구조를 요구하여 [dbapi.h](./include/dbapi.h)를 추가, 전역 변수를 통해 context를 관리할 수 있게 `file_open`, `insert`, `delete`, `find` 함수를 Wrapping하였다.

table의 unique id는 단순히 정수형 변수 `GLOBAL_TABLE_ID`를 1씩 증가 연산하여 반환하고 있으며, 동시에 하나의 파일만을 열 수 있게 구성한 단순한 context 관리 구조이다. 

## 4. Application Layer

실제로 B+Tree에 사용자가 접근하여 조작하기 위한 Layer이다. 평범히 `scanf`를 통해 입력을 받아 `switch-case` 문법을 통해 적절한 operation을 지원한다. 기존의 대부분 operation을 지원하며, verbose 수준, order 변환, command line argument를 통한 operation은 지원하지 않고, 파일을 새로 open할 수 있는 기능은 추가하였다.
