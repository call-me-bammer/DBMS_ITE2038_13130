# **DBMS_ITE2038_13130**

* ## Page Header
>  Number of records
>  Free space
>  (maybe) a next/last pointer
>  Bitmaps, Slot Table ... and so on.

record length can be Fixed or Variable.
How can we find records?
> by record id! == ( PageId, Location in Page )
How do we add and delete records...?

* ### page layouts,
depends on
- Record length (fixed or variable)
- page packing method (packed or unpacked)

<bpt.h>

### add key?
typedef struct record {
    int value;
} record;

typedef struct node {
    void ** pointers;
    int * keys;
    struct node * parent;
    bool is_leaf;
    int num_keys;
    struct node * next; // Used for queue.
} node;

(In main,)
declare, 

<pre><code>
char * input_file;
FILE* fp;
node * root = NULL;

input input;
char instruction;
</code></pre>



[root = insert(root, input, input);]        
### in main,
        case 'i':
            scanf("%d", &input);
            root = insert(root, input, input);
            print_tree(root);
            break;
### in bpt.c, line 456 ~ 848

### (master func.) in bpt.c, line 803 ~ 848


* 현재 구현은 복사를 무시함...
root에 key, value에 해당하는 int type의 input를 parameter로
master insertion function에 전달한다. 먼저 트리에 동일한 key
값이 있는지 find() 함수를 호출하여 확인한다. 동일한 key가
존재하면 insert 내에서 root를 그대로 반환하여 중단하고, find가
NULL값을 반환할 경우 insert를 계속 진행한다.

make_record() 함수를 호출하여 새 record를 동적할당된 value값
을 갖는 pointer라는 이름의 record를 생성한다.

첫번째로 트리가 삭제되었거나 아직 존재하지 않아서 root가 NULL인
경우, start_new_tree() 함수를 호출하여 위에서 pointer라고
생성하고 해당 value를 갖는 record를 root로 삼는 새 트리를 만든다.
또한 새 트리의 루트 노드는 key값을 keys 배열 0번째 칸에 담고 있고,
insert에서 만든 record인 pointer는 pointers 배열 0번째 칸에 담긴다.
그리고 pointers 배열 order - 1번째 칸에 NULL값을 주어, 아직 
다음의 이웃 노드를 가리키는 링크가 없음을 알려준다. root의
parent는 NULL값이고, num_keys 값은 make_leaf(), make_node() 함수로
들어가 초기값이 0 이므로 1로 증가하고 insert로 돌아와서 main으로 반환된다.
make_leaf()를 통해 아직 root는 is_leaf가 true인 leaf이다.

두번째로 트리가 이미 존재하는 경우, insert 초기에 선언한 leaf에
find_leaf() 함수를 호출하여 리턴 값을 받는다. 먼저 파라미터로
전달한 root 노드가 leaf인지 is_leaf 값을 통해 검사한다.
만약 root 노드가 leaf라면, 바로 root 노드를 리턴한다. 반대로
root 노드가 leaf가 아니라면, i는 0부터 num_keys까지 insert할 key
값과 i 인덱스에 해당하는 keys 배열 값과 비교하면서, key값이 같거나
크다면 i를 증가시키고, 작다면 break로 탈출하고 i에 해당하는 
pointers 의 노드를 임시로 c에 저장하고 leaf일 때까지 while의
is_leaf 조건을 다시 확인한다.

find_leaf()를 통해 root에서 leaf 노드 자리를 확인했다. 세번째 경우,
leaf에 order - 1 만큼 key에 이미 존재하는지 확인하고 그보다 작아서
바로 insert 가능할 때 insert_into_leaf() 함수를 호출하고 insert가
완료된 root를 main으로 반환한다. insert_into_leaf() 함수는 insert할
key와 keys 배열값과 비교하여 insertion point를 찾은 후, 위치에 맞게
배열을 옮기는 작업을 수행하고 key와 pointer값을 준다음 leaf를 리턴하는
함수이다.

네번째 경우, leaf에 이미 order - 1 개수 만큼의 key가 존재하여 split한 
이후에 leaf에 insert를 수행해야한다. 



[root = delete(root, input);]
### in main,
        case 'd':
            scanf("%d", &input);
            root = delete(root, input);
            print_tree(root);
            break;
### in bpt.c, line 853 ~ 1213

### (master func.) in bpt.c, line 1201 ~ 1213

root와 삭제할 key값에 해당하는 input을 parameter로 master deletion
function에 전달한다. 

... key_record는 find()를 호출하여 delete할 key가 담겨있는 record를
전달받고, key_leaf = find_leaf()를 호출하여 delete할 key를 포함하는
leaf 노드를 전달받는다. key_record, key_leaf 순으로 short circuit
방식으로 NULL인지 검사하고 그럴 경우 delete를 수행하지 않고 그냥 root를
main으로 리턴한다. 만약 둘 다 NULL이 아닐 경우에는 delete_entry()를
호출하여 delete를 수행한다. delete 이후에는 key_record를 free하고
main으로 변경된 root를 리턴한다.

... delete_entry는 root, key_leaf (함수 내에서 n으로 정의한다.), key, key_record (함수 내에서 pointer로 정의한다.) 를 parameter로 받는다.
(그 외 선언 무시하고...)


> i 10
5 |
3 | 7 9 |
1 2 | 3 4 | 5 6 | 7 8 | 9 10 |
> d 6
5 |
3 | 9 |
1 2 | 3 4 | 5 7 8 | 9 10 |


remove_entry_from_node 함수 호출 ㄱㄱ,
... 예를 들어, delete 6을 해보겠음. 먼저 key_leaf 노드에서 delete할 key에
해당하는 i 인덱스를 keys 배열에서 찾는다.

(글고 왜 인지 모르겠으나,) keys 배열에서 삭제할 key 뒤에 다른 키가 있으면
한 칸씩 옮겨서 삭제할 key를 덮어버린다. 없다면, 머... 그대로 놔둔다.
n은 delete 함수에서 leaf로 전달받았으므로 num_pointers에 num_keys 만큼
값이 담긴다. leaf가 아닌 노드라면, pointer로 가리킬 수 있는 수가 하나 더
많다. ㅇㅋ?
역시 pointers 배열에서 삭제할 pointer 뒤에 다른 포인터가 있으면 한 칸씩
옮겨서 삭제한 pointer 또한 덮어버린다. 없다면, 머... 그대로 놔두고.

다음, num_keys를 하나 줄인다. 5 6 인데, 5 인거 알지? ㅇㅎㅇㅎ.

n은 leaf이므로 if 조건을 만족한다. 따라서 포인터를 i는 키 개수부터 order - 1까지
NULL로 할당한다.


    1. 첫번째 경우는, root에서 deletion이 일어난 후에는 adjust_root()를 호출한다.
    삭제 후에도 다른 키가 채워져있다면, 단순히 **root를 delete_entry로 반환**한다.
    만약 두번째 경우로 root가 empty해지는 deletion이라면 다시 두가지 분기로 나뉜다.

    (1). 자식 노드가 있다면 pointers[0]이 가리키는 자식 노드를 새 root로 한다.
    (2). 자식 노드가 없이 root가 leaf인 상태에서 empty하다면, 모든 tree가 empty
         하므로 adjust_root에서 선언한 new_root를 NULL로 만든다.

    empty root에 대한 작업 후, parameter로 받아온 root는 사용하지 않으므로 
    메모리를 free해주고 **new_root를 delete_entry로 반환**한다.


    2. 두번째 경우, root 외에서의 deletion으로 internal node나 leaf에서
    삭제가 일어난 후이다. leaf냐 아니냐에 따라 삭제가 일어난 노드가 허용하는
    최소 사이즈를 min_keys에 할당한다. (cut...일단 나중에.)

    (1). 삭제 이후, 노드에 들어가있는 key의 개수가 min_keys 조건에 맞다면 delete로
         root를 리턴한다.
    (2). 최소 사이즈 미만으로 내려가면, coalescence나 redistribution이 필요하다.
         먼저 delete에서 key_leaf로 전달받은 노드 n을 get_neighbor_index()로
         전달하여 neighbor_index를 받는다. 같은 부모 노드 기준으로, 왼쪽 칭(?)구가 
         있다면 그 아이의 인덱스를 받는다. 만약 없다면, -1을 받는다.

         neighbor_index에 따라서 k_prime_index를 결정하는데, -1일 경우 0, 그렇지
         않을 경우 neighbor_index와 같게 된다.

         ? 아무튼 지우고 그.. 머.. 2개를 뭐 한다음에
         delete_entry를 아무튼 빨리 끝냈어요 ㅇㅋ?
         root가 수정되고
         delete가 완료 되었으므로, key_record를 free하고 root를 메인으로 리턴한다.




## **insertion after split**

node * insert_into_leaf_after_splitting(node * root, node * leaf, int key, record * pointer);



## **merge after deletion**

node * delete(node * root, int key);



\
\
\
\
\
\
\
\
\
\
a-ha.


---
* * *

### **Milestone 1**

➢ Analyze the given b+ tree code and submit a report to the hconnect Wiki.

➢ Your report should includes
1. Possible call path of the insert/delete operation
2. Detail flow of the structure modification (split, merge)
3. (Naïve) designs or required changes for building on-disk b+ tree



```
    
      그러니까 1번은 걍 .c로 분석 ㄱㄱ하고
      2번은 split, merge에 대헤서 구조 흐름을 적고

      3번은 나입ㅂ브ㅡ하게 "on-disk" bpt를 짜기위한 디자인이나 필요한 변경사항

```

* * *
Milestone 2

➢ Implement on-disk b+ tree and submit a report(Wiki) including your design.
* * *
---