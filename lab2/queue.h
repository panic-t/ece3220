#include <ucontext.h>

#define INVALID INT_MIN

typedef struct qnode_tag {
    int key, waiting, sig;
    ucontext_t *thread;
    struct qnode_tag *link;
} qnode;

typedef struct queue_tag {
    qnode *head, *tail;
    int size;
} queue;

typedef struct lockqnode_tag {
    int lock, holder, sig;
    struct lockqnode_tag *link;
} lockqnode;

typedef struct lockqueue_tag {
    lockqnode *head, *tail;
    int size;
} lockqueue;

int qpush(ucontext_t *newContext, queue *q);
int qfront(qnode *node, queue *q);
qnode *qrotate(queue *q);
qnode *qpop(queue *q);
qnode *qfind(int search, queue *q);
qnode *qwaitfind(int search, int signal, queue*q);
qnode *qmatch(ucontext_t *search, queue *q);
int qremove(qnode *target, queue *q);
int qadd(qnode *node, queue *q);
ucontext_t *qdelete(qnode *target, queue *);
ucontext_t *getThread(qnode *target);
int getID(qnode *target);
void setWait(qnode* target, int waitval);
int getWait(qnode *target);
void setSig(qnode *target, int signal);
int getSig(qnode *target);
int lockon(int locknum, int holdid, int signal, lockqueue *q);
int findlock(int locknum, int signal, lockqueue *q);
int lockoff(int locknum, int signal, lockqueue *q);