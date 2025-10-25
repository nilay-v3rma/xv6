#include "types.h"
#include "user.h"

struct lock l;
void* count(void* arg){
    for(int i=0; i<1000000; i++)
	{	
		acquireLock(&l);
		*(int*)arg = *(int*)arg + 1;
		releaseLock(&l);
	}
    thread_exit();
    return 0;
}

int main()
{
	int x;
    initiateLock(&l);
	// two threads increment the same counter 10000 times each
	uint tid1, tid2;

	x = 0;

    thread_create(&tid1, count, (void*)&x);
    thread_create(&tid2, count, (void*)&x);
    thread_join(tid1);
    thread_join(tid2);

    printf(1, "Final value of x: %d\n", x);
    exit();
}

