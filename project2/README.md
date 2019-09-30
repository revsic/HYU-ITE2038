# On-Disk B+ Tree

Database 내부 Record Mangement system을 위한 On-disk B+ Tree 소개 및 In-Memory 구현체 분석.

## 1. What is B+ Tree

Tree형 자료구조의 일종으로 정렬된 Key-Value Pair에 대한 효율적인 삽입, 검색, 삭제를 지원한다.

<img src="./rsrc/img1.png" width="60%">

각각의 노드는 최대 노드 개수 B를 기준으로 루트 노드를 제외, {B - 1} 개의 키와 B개의 자식 노드를 가질 수 있다. Pair의 수에 따라 다계층으로 구성되며, 계층의 수와 검색에 걸리는 시간이 비례한다. 이에 계층 수를 tight 하게 관리하기 위해 추가적인 Balancing policy를 보유하고 있다. 이는 기존의 B-Tree와도 유사하며, B+Tree는 sibling에 대한 포인터를 가져 다음 Key-Value Pair로의 Sequential 한 검색이 가능하단 장점이 있다.

## 2. Why it used as on-disk data structure

B+ Tree는 B라는 Branching Factor를 Hyper parameter로 가지고 있는데, 이 크기가 늘어날수록 하나의 노드에서 키를 찾는데 시간이 상대적으로 증가하지만, 디스크 접근 속도가 월등히 느린 현대 컴퓨터에서는 B를 키워 노드를 읽는 수를 줄일 수 있다는 장점이 있다.

하나의 노드 내에서 키를 검색하는 데에는 선형 탐색을 가정할 때 B 번, 2진 탐색을 가정할 때 logB의 시간 복잡도를 가진다.

![time complexity of linear and binary search](./rsrc/form1.png)

N개의 데이터를 가정할 때, 자료 구조에서 요구하는 노드의 수와 트리의 높이는 다음과 같다.

![the number of nodes for B+ Tree](./rsrc/form2.png)

기존의 BST나 AVL 트리가 B=2를 기준으로 했다면, B+ Tree (혹 B-Tree)는 이를 보다 일반화하여 노드를 불러오는 횟수를 줄인 자료구조이다. 디스크에서 노드를 불러온 후, 키를 검색하는 알고리즘은 In-Memory로 작동하기 때문에 디스크 기반에서 기존의 트리형 자료구조에 비해 빠른 검색을 지원한다.

## 3. Balancing policy

B+ Tree에는 몇가지 Constraint가 존재한다.
- 최대 자식 노드 수 : B
- 최소 자식 노드 수 (root 제외) : ceil(B / 2)
- 최대 key 수 : B - 1
- 최소 key 수 (root 제외) : ceil(B / 2) - 1

이는 노드가 가지는 키의 수가 불균형할 수 있어 생기는 문제를 해결하기 위함이다. 위의 조건을 만족하기 위해 B+ Tree (혹은 B-Tree)는 Tree Balancing Policy를 가진다. 

1. Redistribute

일반 Tree의 경우 새로운 Key-Value Pair가 특정 Leaf node에만 추가되거나, 특정 노드에서만 Deletion이 발생할 경우 Tree Unbalancing 문제가 발생할 수 있다.

<img src="./rsrc/balanced.png" width="60%">

B+ Tree는 이에 노드가 최대 최소 조건을 어겼을 때, Sibling 노드를 조사하여 자신의 Pair를 Sibling에게 전달하거나 받아온다. 

<img src="./rsrc/redist.png" width="60%">

좌측 노드에 키와 포인터를 넘겨 주거나(insertion), 받을(deletion) 여력이 있다면 가장 작은 키를 좌측 노드에 추가하거나, 좌측 노드의 가장 큰 키를 받아온다. 같은 논리로 우측 노드에 자리가 있다면, 가장 큰 키를 우측 노드로 전달하거나, 우측 노드의 가장 작은 키를 가져온다. 이를 Key Rotation 혹은 Redistribution이라 표현한다.

2. Split

Insertion 과정에서 더는 Redistribution을 할 수 없는 경우, 혹 특정 level이 수용할 수 있는 최대 key의 수를 넘어설 때 B+Tree는 하나의 노드를 두 개의 노드로 분리, Split 한다. 

<img src="./rsrc/split.png" width="60%">

요청받은 키를 추가하고 (B+1개 키), 중간값을 기준으로 두 개의 독단적인 노드로 분리한다 (left node B/2개 키 + key + right node B/2개 키). 이후 중간 key를 상위 노드에 insert 하며, 삽입된 키의 좌우로 분리된 두 개의 노드를 하위 노드로 연결한다. 

3. Merge

Remove 과정에서 더는 Redistribution을 할 수 없는 경우, 혹 특정 level이 수용해야 하는 최소 key의 수를 넘어설 때 B+Tree는 두 개의 노드를 하나의 노드로 합친다 (Merge).

<img src="./rsrc/merge.png" width="60%">

두 개 노드를 연결하고 있는 상위 노드의 키를 가져와 left node + key + right node의 꼴로 연결한다. 이후 상위 노드에서 해당 키를 Delete 한다. 

## 4. Analyze bpt.c

[bpt.c](./src/bpt.c)은 B+Tree의 In-Memory 구현체이다. 실제 구현체를 통해 insertion, deletion, merge, split, redistribution operation을 분석한다.

### 1. Insertion method in bpt.c

[bpt.c](./src/bpt.c)에서는 insertion을 총 4가지 경우로 나눈다.

1. 이미 주어진 key가 존재할 경우
```c
if (find(root, key, false) != NULL)
    return root;
```
추가 수정을 하지 않는다.

2. tree가 비어 있는 경우
```c
if (root == NULL) 
    return start_new_tree(key, pointer);
```
새로운 tree를 생성하여 반환한다.

3. number of key constraint를 어기지 않는 경우
```c
leaf = find_leaf(root, key, false);

if (leaf->num_keys < order - 1) {
    leaf = insert_into_leaf(leaf, key, pointer);
    return root;
}
```
record를 생성하여 추가적인 balancing policy 없이 leaf에 덧붙인다.

4. 어기는 경우
```c
return insert_into_leaf_after_splitting(root, leaf, key, pointer);
```
Split policy에 따라 key를 삽입한다.

위 구현체에서는 insertion 과정에 redistribution policy를 적용하지 않고 split만을 진행한다.

### 2. Split operation in bpt.c

Split의 첫 함수는 insert 내부에 있는 `insert_into_leaf_after_splitting`에서 시작하며, 상위 노드에 key를 삽입하는 과정을 `insert_into_parent`로 축약한다.

`insert_into_parent`는 다음 3가지 조건에 따라 분기한다.
1. root를 split한 경우 : `insert_into_new_root`
```c
if (parent == NULL)
    return insert_into_new_root(left, key, right);
```
2. 노드에 키를 단순 삽입하는 경우 : `insert_into_node`
```c
left_index = get_left_index(parent, left);

if (parent->num_keys < order - 1)
    return insert_into_node(root, parent, left_index, key, right);
```
3. 노드를 split한 후 키를 삽입하는 경우 : `insert_into_node_after_splitting`
```c
return insert_into_node_after_splitting(root, parent, left_index, key, right);
```

이후 `insert_into_node_after_splitting` 에서 다시 상위 노드에 key를 삽입하기 위해 `insert_into_parent`를 호출한다.

### 3. Deletion method in bpt.c

[bpt.c](./src/bpt.c)에서는 `delete`함수가 내부적으로 `delete_entry`를 호출한다. `delete_entry`는 4가지 분기를 가진다. 

1. root에서 deletion이 발생한 경우 : root가 비어있는지, child가 하나인지 검사하여 shrink한다.
```c
if (n == root) 
    return adjust_root(root);
```

2. key의 최대 최소 조건을 만족할 경우 : 조기 반환한다.
```c
if (n->num_keys >= min_keys)
    return root;
```

3. Constraint를 만족하지 않지만, Merge가 가능한 경우 : 인접 노드와 merge한다.
```c
if (neighbor->num_keys + n->num_keys < capacity)
    return coalesce_nodes(root, n, neighbor, neighbor_index, k_prime);
```

4. Constraint도 만족하지 않고, Merge도 불가능한 경우 : Redistribution policy에 따라 인덱스를 재배분한다.
```c
return redistribute_nodes(root, n, neighbor, neighbor_index, k_prime_index, k_prime);
```

### 4. Merge operations in bpt.c

[bpt.c](./src/bpt.c)에서는 Merge operation을 `coalesce_nodes`를 통해 구현한다. 중간값 `k_prime`과 right node `n`를 left node `node`에 덧붙이고, `delete_entry`를 통해 상위 노드에서 중간값을 삭제한다.

### 5. Redistribution operations in bpt.c

`redistribute_node`에서는 좌측에 노드가 있는지와 leftmost 노드인지를 기준으로 첫 분기를 실행한다 `neighbor_index != -1`. 이후 좌측에 노드가 있는 경우, key와 pointer를 오른쪽으로 한 칸씩 옮긴 후 좌측 노드의 마지막 값을 처음 칸에 옮긴다. 부가적으로 상위 노드의 key 값과 하위 노드의 부모 노드 포인터를 수정한다. leftmost node의 경우 같은 논리로 오른쪽 노드에서 값을 가져온다.

## 5. Naive Design requirements for on-disk b+ tree

[bpt.c](./src/bpt.c)는 In-Memory Node를 기준으로 search, insert, delete operation이 동작했다면, Database system에서는 대규모 Key-Value Pair와 Structure를 저장하기 위해 On-Disk 방식으로 작동할 필요가 있다.

기존의 노드 구조는 [bpt.h](./src/bpt.h)에 다음과 같이 정의되어 있다.
```c
typedef struct node {
    void ** pointers;
    int * keys;
    struct node * parent;
    bool is_leaf;
    int num_keys;
    struct node * next; // Used for queue.
} node;
```
자식 노드가 `void**` 타입으로 연결되어 있고, 상위 노드 또한 `struct node*` 타입으로 모두 In-Memory Pointer 타입으로 상관되고 있다. 이를 On-Disk 방식으로 바꾸기 위해 PageID와 PageID를 실제 File 내 Page로 연결하기 위한 PagePointer 구조를 정의하고, In-Memory Pointer 대신 PageId를 가지게 한다.

PagePointer은 Page의 고유 아이디인 `page_id`, 해당 Page가 존재하는 `file_id`, 해당 File 내에서 Page의 상대 위치인 `rel_page_id`를 가진다.
```c
typedef struct _PagePointer {
    int page_id;
    int file_id;
    int rel_page_id;
} PagePointer;
```

또한 `PagePointer`를 통해 global context에서 PageID를 실 주소로 mapping하기 위한 `PagePointerManager`를 정의한다.
```c
typedef struct _PagePointerManager {
    int num_pointers;
    PagePointer pointers[MAX_POINTERS];
} PagePointerManager;
```

이후, 실제 In-Memory Node를 On-Disk Page로 변환한다.
```c
typedef struct _Record {
    char some_values[SIZE_OF_RECORDS];
} Record;

typedef struct _KeyValuePair {
    int key;
    Record value;
} KeyValuePair;

typedef struct _Page {
    int parent_pid;
    int special_pid;
    int is_leaf;
    int num_keys;
    KeyValuePair pairs[MAX_PAIRS];
} Page;
```
위 Page에서 `special_pid`는 Free page일 때 다음 Free Page의 Page ID, Leaf 노드일 때 Sibling Page ID, Internal 노드일 때 Leftmost Child Page ID를 가진다.

실제 File에는 여러 Page가 저장되어야 하므로 File에 관련된 자료구조도가 별도로 필요하다.
```c
typedef struct _File {
    int free_pid;
    int num_pages;
    Page pages[MAX_PAGES];
} File;
```
free_pid는 Linked List 형태로 연결된 Free Page의 Head를 가리킨다.

또한 B+Tree에서 이용할 Global Context 관리를 위한 RootFile이 필요하다.
```c
typedef struct _RootFile {
    int root_pid;
    int num_files;
    int file_ids[MAX_FILES];
    PagePointerManager manager;
} RootFile;
```

Disk-Level Structure을 위해 몇가지 Abstracted Procedure이 필요하다.
1. `int open_table(char* filepath, RootFile* retval, OpenFlag flag)`: flag에 따라 파일을 열거나 새로 생성하여 RootFile 구조를 반환한다.
2. `int search_page_id(int page_id, PagePointerManager* manager, PagePointer* retval)`: 해당 `page_id`를 통해서 실제 `file_id`와 `rel_page_id`를 검색한다.
3. `int load_page(PagePointer* page_ptr, Page* retval)`: 해당 `PagePointer`를 통해 `Page`를 메모리에 올린다.
4. `int make_page(RootFile* root, Page* retval)`: 해당 `RootFile`에 새로운 `Page`를 생성한다. 필요한 경우 `File`을 추가생성할 수 있다.
5. `int delete_page(RootFile* root, int page_id)`: 해당 `Page`를 삭제한다.

이를 통해서 B+Tree를 On-Disk 방식으로 수정한다.
1. `int open(char* pathname, RootFile* retval, OpenFlag flag)`: `pathname`에서 `RootFile`을 업로드 한다.
2. `int insert(RootFile* root, int key, Record* record)`: 해당 B+Tree에 `key`와 `record`를 삽입한다.
3. `int find(RootFile* root, int key, Record* retval)`: 해당 B+Tree에서 `key`를 찾아 `Record`를 반환한다.
4. `int delete(RootFile* root, int key)`: 해당 B+Tree에서 `key`를 삭제한다.

B+Tree를 In-Memory 방식에서 On-Disk 방식으로 바꾸기 위해 `make_node`를 `make_page`로, node의 삭제를 `delete_page`로, 포인터 접근 방식을 `load_page`로 변환한다. 이후에는 `Node`와 `Page`가 1대1 대응이 가능한 관계이기 때문에 논리에 맞게 수정할 수 있다. 
