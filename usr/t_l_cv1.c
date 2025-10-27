#include "types.h"
#include "user.h"

struct all{
    int* x;
    struct lock *l;
    struct condvar* cv;
};

void* thread2(void* arg){
    // printf(1, "Thread 2 created %d\n", *(int*)arg);
    struct all* t = (struct all*) arg;
    printf(1,"Gonna acquire lock\n");
    acquireLock(t->l);
    printf(1,"Acquired lock successfully\n");
    int* argPtr = t->x;
    *argPtr = (*argPtr + 1);
    broadcast(t->cv);
    releaseLock(t->l);
    thread_exit();
    return 0;
}

int main(){
    struct lock l;
    struct condvar cv;
    printf(1, "Hello World\n");
    int x = 10;
    struct all a;
    a.x = &x;
    a.l = &l;
    a.cv = &cv;
    uint tid1;
    initiateLock(&l);
    initiateCondVar(&cv);
    acquireLock(&l);
    thread_create(&tid1,thread2,(void*)&a);
    sleep(200);
    condWait(&cv,&l);  // Remove sleep, go directly to condWait
    releaseLock(&l);
    thread_join(tid1);
    printf(1,"Value of x = %d\n",x);
    exit();
}
