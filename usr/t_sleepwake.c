#include "types.h"
#include "user.h"

int main() {

    int openChan = getChannel();
    int ret = fork();
    if (ret == 0) {
        sleep(200);
        sigChan(openChan);
        exit();
    }
    else {
        printf(1,"going to sleep on channel %d\n",openChan);
        sleepChan(openChan);
        printf(1,"Woken up by child\n");
        wait();
    }
    exit();

}
