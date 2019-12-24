# Concurrency Control

* * *

* Milestone #1

~~~
* Design my lock manager and express it on Wiki
* Implement begin_trx(), end_trx() APIs.
* Deadline: Nov. 19th 23:59.
~~~

* APIs

> Wrapping operation in TNX.

~~~c
int begin_trx();
~~~

~~~c
int end_trx(int tid);
~~~



* Lock Manager

- [ ] Conflict-serializable schedule for TXNs.
- [ ] Strict-2PL ( avoid CASCADing aborts )
- [ ] Deadlock detection ( abort the TXN if detected )
- [ ] Row-level locking with S(Shared) & X(Exclusive) mode



* * *

* Milestone #2

~~~
* Implement lock manager
* Submit a report including implementation, design, issues, etc.
* Deadline: Dec. 5th 23:59. 
~~~

* * *

# 아무 디비 백엔드말 대잔치

~~~c
struct lock_t {
    int table_id;
    int record_id; // or key
    enum lock_mode mode; // SHARED, EXCLUSIVE
    trx_t* trx; // backpointer to lock holder
    ... // up to my design
};

struct trx_t {
    int trx_id;
    enum trx_state state; //  IDLE, RUNNING, WAITTING
    list<lock_t*> trx_locks; // 
    lock_t* wait_lock; //lock object that trx is waiting
    ... // up to my design
};
~~~

이라고 Object struct. 예시를 주시는데
저 list<> Data type에서 이미 그뤠도 C++를
쓰는 게 좋즤 않겠늬? 라고 말하네요.

머 저거 하나 때문에 C++를 쓸 수는 없어요우.

일단 끄적거리면서 수업을 정리하려고 함.
내가 볼 때는 그게 먼저임.

아무튼 디비 백엔드의 과제는 2개,
locking manager와
logging & recovery
를 구현하는 것.

conflict가 발생했을 때 어떻게 대처해야
하는가에서부터 생각해보자.

* * * 

지금까지,
SQL Client 하나로만 DBMS를 만들어서 썼잖아, single thread니까. 그런데 여러 thread로 다수의 Client가 동시에 가깝게 DB에 접근한다고 하자, (물론 나는 thread를 안써봐서 잘은 모르겠지만 느낌적인 느낌으로다가 설명하겠다.) 그게 또 하필 같은 자료, 동일한 record을 필요로 한다고 하자.

여러분들이 만든 DBMS는
`Consistency`한 state를 guarantee해야한다.

~~~c

~~~~

~~~c

~~~~
리턴 value를 master thread로 전달

pthread_

* * *

pthread로 global var 증가 시
-> 에셈 단위에선 3개 인스트럭션으로 이루어짐
..

* mutual execlusive
-> 해당 동작이 하나의 쓰레드로 이루어짐을 보장가능

* 락
-> 쓰레드가 

* 트라이 락
-> 한 번만 시도



머야 갑자기 조교님이 과제를 내고있어

prime_mt.cpp -> _cond.cpp로 개선한 걸 만들어서 ...머 해봐요

