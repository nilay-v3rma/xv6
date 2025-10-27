#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"

char*
strcpy(char *s, char *t)
{
    char *os;
    
    os = s;
    while((*s++ = *t++) != 0)
        ;
    return os;
}

int
strcmp(const char *p, const char *q)
{
    while(*p && *p == *q)
        p++, q++;
    return (uchar)*p - (uchar)*q;
}

uint
strlen(char *s)
{
    int n;
    
    for(n = 0; s[n]; n++)
        ;
    return n;
}

void*
memset(void *dst, int v, uint n)
{
	uint8	*p;
	uint8	c;
	uint32	val;
	uint32	*p4;

	p   = dst;
	c   = v & 0xff;
	val = (c << 24) | (c << 16) | (c << 8) | c;

	// set bytes before whole uint32
	for (; (n > 0) && ((uint)p % 4); n--, p++){
		*p = c;
	}

	// set memory 4 bytes a time
	p4 = (uint*)p;

	for (; n >= 4; n -= 4, p4++) {
		*p4 = val;
	}

	// set leftover one byte a time
	p = (uint8*)p4;

	for (; n > 0; n--, p++) {
		*p = c;
	}

	return dst;
}

char*
strchr(const char *s, char c)
{
    for(; *s; s++)
        if(*s == c)
            return (char*)s;
    return 0;
}

char*
gets(char *buf, int max)
{
    int i, cc;
    char c;
    
    for(i=0; i+1 < max; ){
        cc = read(0, &c, 1);
        if(cc < 1)
            break;
        buf[i++] = c;
        if(c == '\n' || c == '\r')
            break;
    }
    buf[i] = '\0';
    return buf;
}

int
stat(char *n, struct stat *st)
{
    int fd;
    int r;
    
    fd = open(n, O_RDONLY);
    if(fd < 0)
        return -1;
    r = fstat(fd, st);
    close(fd);
    return r;
}

int
atoi(const char *s)
{
    int n;
    
    n = 0;
    while('0' <= *s && *s <= '9')
        n = n*10 + *s++ - '0';
    return n;
}

void*
memmove(void *vdst, void *vsrc, int n)
{
    char *dst, *src;
    
    dst = vdst;
    src = vsrc;
    while(n-- > 0)
        *dst++ = *src++;
    return vdst;
}

static inline int arm_xchg(volatile int *addr, int newval) {
    int old;
    // Atomic swap using ARM SWP instruction
    __asm__ __volatile__(
        "swp %0, %2, [%1]"
        : "=&r"(old)
        : "r"(addr), "r"(newval)
        : "memory");
    return old;
}

void initiateLock(struct lock* l) {
    if(!l) return;
    l->isInitiated = 1;
    l->lockvar = 0; // initiate lock
}

void acquireLock(struct lock* l) {
    if(!l || !l->isInitiated) return;
    // spin until we atomically set lockvar from 0 to 1
    // arm_xchg returns the previous value
    while(arm_xchg(&l->lockvar, 1) != 0) {
        // busy-wait; optional backoff could be added if needed
    }
}

void releaseLock(struct lock* l) {
    if(!l || !l->isInitiated) return;
    // atomically set lockvar back to 0
    arm_xchg(&l->lockvar, 0);
}

void initiateCondVar(struct condvar* cv) {
    if(!cv) return;
    // Allocate a channel through getChannel and set cv->var to the channel's value
    cv->var = getChannel();
    cv->isInitiated = 1;
}

void condWait(struct condvar* cv, struct lock* l) {
    if(!cv || !cv->isInitiated || !l || !l->isInitiated) return;
    
    // Release the lock, sleep on the channel, and re-acquire the lock after waking up
    releaseLock(l);
    sleepChan(cv->var);
    acquireLock(l);
}

void broadcast(struct condvar* cv) {
    if(!cv || !cv->isInitiated) return;
    // Wake up all threads sleeping on the CV's channel
    sigChan(cv->var);
}

void signal(struct condvar* cv) {
    if(!cv || !cv->isInitiated) return;
    // Wake up one specific thread sleeping on the CV's channel
    sigOneChan(cv->var);
}

void semInit(struct semaphore* s, int initVal) {

}

void semUp(struct semaphore* s) {

}

void semDown(struct semaphore* s) {

}

