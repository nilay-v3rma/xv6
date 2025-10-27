#include "types.h"
#include "user.h"

struct lock l;
struct condvar cv;

void* toggle1(void* arg){
    acquireLock(&l);
    for(int i=0; i < 10; i++) {
        while(*(int*)arg == 1){
            condWait(&cv, &l);
        }
        printf(1, "I am thread 1\n");
        *(int*)arg = 1;
        broadcast(&cv);
    }
    releaseLock(&l);
    thread_exit();
    return 0;
}

void* toggle2(void* arg){
    acquireLock(&l);
    for(int i=0; i < 10; i++) {
        while(*(int*)arg == 0){
            condWait(&cv, &l);
        }
        printf(1, "I am thread 2\n");
        *(int*)arg = 0;
        broadcast(&cv);
    }
    releaseLock(&l);
    thread_exit();
    return 0;
}

int main()
{
	initiateLock(&l);
    initiateCondVar(&cv);
    int x = 0;

    uint tid1, tid2;
    thread_create(&tid1, toggle1, (void*)&x);
    thread_create(&tid2, toggle2, (void*)&x);
    thread_join(tid1);
    thread_join(tid2);

    exit();
}

