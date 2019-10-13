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

`struct file_manager_t`는 D

## 3. B+Tree Layer

## 4. Application Layer

## 5. Delayed Merge