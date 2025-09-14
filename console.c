// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

static void consputc (int);

static int panicked = 0;

static struct {
    struct spinlock lock;
    int locking;
} cons;

#define MAX_COMMANDS 100
#define MAX_COMMAND_LENGTH 32

char commands[MAX_COMMANDS][MAX_COMMAND_LENGTH];
int command_count = 0;

void init_usr_commands(void)
{
    struct inode *dp;
    struct dirent de;
    int off;

    command_count = 0;
    trie_init(); // initialize trie

    if((dp = namei("/")) == 0) {
        return;
    }

    ilock(dp);

    for(off = 0; off < dp->size && command_count < MAX_COMMANDS; off += sizeof(de)) {
        if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
            break;
        }

        if(de.inum == 0) {
            continue;
        }

        if(de.name[0] != '.' && de.name[1] != '\0') {
            char *cmdname = de.name;
            int cmdlen = strlen(cmdname);

            if(cmdlen < MAX_COMMAND_LENGTH) {
                safestrcpy(commands[command_count], cmdname, MAX_COMMAND_LENGTH);
                trie_insert(cmdname); // insert into trie
                command_count++;
            }
        }
    }

    iunlockput(dp);
}

static void printint (int xx, int base, int sign)
{
    static char digits[] = "0123456789abcdef";
    char buf[16];
    int i;
    uint x;

    if (sign && (sign = xx < 0)) {
        x = -xx;
    } else {
        x = xx;
    }

    i = 0;

    do {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (sign) {
        buf[i++] = '-';
    }

    while (--i >= 0) {
        consputc(buf[i]);
    }
}
//PAGEBREAK: 50

// Print to the console. only understands %d, %x, %p, %s.
void cprintf (char *fmt, ...)
{
    int i, c, locking;
    uint *argp;
    char *s;

    locking = cons.locking;

    if (locking) {
        acquire(&cons.lock);
    }

    if (fmt == 0) {
        panic("null fmt");
    }

    argp = (uint*) (void*) (&fmt + 1);

    for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
        if (c != '%') {
            consputc(c);
            continue;
        }

        c = fmt[++i] & 0xff;

        if (c == 0) {
            break;
        }

        switch (c) {
        case 'd':
            printint(*argp++, 10, 1);
            break;

        case 'x':
        case 'p':
            printint(*argp++, 16, 0);
            break;

        case 's':
            if ((s = (char*) *argp++) == 0) {
                s = "(null)";
            }

            for (; *s; s++) {
                consputc(*s);
            }
            break;

        case '%':
            consputc('%');
            break;

        default:
            // Print unknown % sequence to draw attention.
            consputc('%');
            consputc(c);
            break;
        }
    }

    if (locking) {
        release(&cons.lock);
    }
}

void panic (char *s)
{
    cli();

    cons.locking = 0;

    cprintf("cpu%d: panic: ", cpu->id);

    show_callstk(s);
    panicked = 1; // freeze other CPU

    while (1)
        ;
}

//PAGEBREAK: 50
#define BACKSPACE 0x100
#define CRTPORT 0x3d4

void consputc (int c)
{
    if (panicked) {
        cli();
        while (1)
            ;
    }

    if (c == BACKSPACE) {
        uartputc('\b');
        uartputc(' ');
        uartputc('\b');
    } else {
        uartputc(c);
    }

    // cgaputc(c);
}

#define INPUT_BUF 512
struct {
    struct spinlock lock;
    char buf[INPUT_BUF];
    uint r;  // Read index
    uint w;  // Write index
    uint e;  // Edit index
} input;

#define TRIE_CHILDREN 128  // ASCII

typedef struct TrieNode {
    struct TrieNode *children[TRIE_CHILDREN];
    int is_end;
    char command[MAX_COMMAND_LENGTH];
} TrieNode;

TrieNode trie_root;

// Initialize the trie
void trie_init() {
    memset(&trie_root, 0, sizeof(trie_root));
}

// Insert a command into the trie
void trie_insert(const char *cmd) {
    TrieNode *node = &trie_root;
    for (int i = 0; cmd[i]; i++) {
        unsigned char idx = (unsigned char)cmd[i];
        if (!node->children[idx]) {
            node->children[idx] = (TrieNode*)alloc_page();
            if (node->children[idx])
                memset(node->children[idx], 0, sizeof(TrieNode));
        }
        node = node->children[idx];
    }
    node->is_end = 1;
    safestrcpy(node->command, cmd, MAX_COMMAND_LENGTH);
}

// Helper to collect matches (for single match, can stop early)
void trie_collect(TrieNode *node, char *result, int *found) {
    if (!node || *found > 1) return;
    if (node->is_end) {
        safestrcpy(result, node->command, MAX_COMMAND_LENGTH);
        (*found)++;
    }
    for (int i = 0; i < TRIE_CHILDREN; i++) {
        if (node->children[i])
            trie_collect(node->children[i], result, found);
    }
}

// Find node for prefix
TrieNode* trie_find(const char *prefix) {
    TrieNode *node = &trie_root;
    for (int i = 0; prefix[i]; i++) {
        unsigned char idx = (unsigned char)prefix[i];
        if (!node->children[idx])
            return 0;
        node = node->children[idx];
    }
    return node;
}

void autocomplete(uint *e, char *buf, uint w) {
    int start = *e - 1;
    while (start >= (int)w && buf[start % INPUT_BUF] != ' ' && buf[start % INPUT_BUF] != '\n') {
        start--;
    }
    start++;

    int len = *e - start;
    if (len <= 0) {
        return;
    }

    char prefix[MAX_COMMAND_LENGTH];
    for (int i = 0; i < len && i < MAX_COMMAND_LENGTH-1; i++) {
        prefix[i] = buf[(start + i) % INPUT_BUF];
    }
    prefix[len] = '\0';

    TrieNode *node = trie_find(prefix);
    if (!node){
        return;
    }

    char match[MAX_COMMAND_LENGTH];
    int found = 0;
    trie_collect(node, match, &found);

    if (found == 1) {
        // Complete the command in the input buffer
        int matchlen = strlen(match);
        if (matchlen > len) {
            for (int i = len; i < matchlen; i++) {
                buf[(*e) % INPUT_BUF] = match[i];
                consputc(match[i]);
                (*e)++;
            }
        }
    }
    else if (found > 1) {
        consputc('\n');
        // Print all matches
        for (int i = 0; i < command_count; i++) {
            if (strncmp(commands[i], prefix, len) == 0) {
                cprintf("%s\n", commands[i]);
            }
        }
        // Reprint prompt and current input
        cprintf("> ");
        for (int i = w; i < *e; i++) {
            consputc(buf[i % INPUT_BUF]);
        }
    }
    else{
        // No matches found
        consputc('\n');
        cprintf("$");
        for (int i = w; i < *e; i++) {
            consputc(buf[i % INPUT_BUF]);
        }
    }
}

#define C(x)  ((x)-'@')  // Control-x
void consoleintr (int (*getc) (void))
{
    int c;

    acquire(&input.lock);

    while ((c = getc()) >= 0) {
        switch (c) {
        case C('P'):  // Process listing.
            procdump();
            break;

        case C('U'):  // Kill line.
            while ((input.e != input.w) && (input.buf[(input.e - 1) % INPUT_BUF] != '\n')) {
                input.e--;
                consputc(BACKSPACE);
            }

            break;

        case C('H'):
        case '\x7f':  // Backspace
            if (input.e != input.w) {
                input.e--;
                consputc(BACKSPACE);
            }

            break;
        case '\t': // Tab
            autocomplete(&input.e, input.buf, input.w);
            break;

        default:
            if ((c != 0) && (input.e - input.r < INPUT_BUF)) {
                c = (c == '\r') ? '\n' : c;

                input.buf[input.e++ % INPUT_BUF] = c;
                consputc(c);

                if (c == '\n' || c == C('D') || input.e == input.r + INPUT_BUF) {
                    input.w = input.e;
                    wakeup(&input.r);
                }
            }

            break;
        }
    }

    release(&input.lock);
}

int consoleread (struct inode *ip, char *dst, int n)
{
    uint target;
    int c;

    iunlock(ip);

    target = n;
    acquire(&input.lock);

    while (n > 0) {
        while (input.r == input.w) {
            if (proc->killed) {
                release(&input.lock);
                ilock(ip);
                return -1;
            }

            sleep(&input.r, &input.lock);
        }

        c = input.buf[input.r++ % INPUT_BUF];

        if (c == C('D')) {  // EOF
            if (n < target) {
                // Save ^D for next time, to make sure
                // caller gets a 0-byte result.
                input.r--;
            }

            break;
        }

        *dst++ = c;
        --n;

        if (c == '\n') {
            break;
        }
    }

    release(&input.lock);
    ilock(ip);

    return target - n;
}

int consolewrite (struct inode *ip, char *buf, int n)
{
    int i;

    iunlock(ip);

    acquire(&cons.lock);

    for (i = 0; i < n; i++) {
        consputc(buf[i] & 0xff);
    }

    release(&cons.lock);

    ilock(ip);

    return n;
}

void consoleinit (void)
{
    initlock(&cons.lock, "console");
    initlock(&input.lock, "input");

    devsw[CONSOLE].write = consolewrite;
    devsw[CONSOLE].read = consoleread;

    cons.locking = 1;

    // init_commands();
}

