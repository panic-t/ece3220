/* <ryan ware>
project2
ece3220 spring 2023
*/

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <limits.h>
#include "queue.h"
#include "mythreads.h"
#include "threadwrapper.h"

queue *threadq, *blockq;
lockqueue *locks;
void **result_holder;

void threadInit() {
    interruptsAreDisabled = 1;
    ucontext_t *current = (ucontext_t *)malloc(sizeof(ucontext_t));
    threadq = (queue *)malloc(sizeof(queue));
    blockq = (queue *)malloc(sizeof(queue));
    locks = (lockqueue *)malloc(sizeof(lockqueue));
    result_holder = (void **)malloc(sizeof(void *));
    threadq->head = NULL;
    threadq->tail = NULL;
    threadq->size = 0;
    blockq->head = NULL;
    blockq->tail = NULL;
    blockq->size = 0;
    locks->head = NULL;
    locks->tail = NULL;
    locks->size = 0;
    getcontext(current);
    qpush(current, threadq);
    interruptsAreDisabled = 0;
    return;
}

int threadCreate(thFuncPtr funcPtr, void *argPtr) {
    int id;
    ucontext_t *newThread = (ucontext_t *)malloc(sizeof(ucontext_t)), 
    *current = getThread(qcurrent(threadq));
    interruptsAreDisabled = 1;
    getcontext(newThread);
    newThread->uc_stack.ss_sp = malloc(STACK_SIZE);
    newThread->uc_stack.ss_size = STACK_SIZE;
    newThread->uc_stack.ss_flags = 0;
    makecontext(newThread, (void(*)())&ufuncwrap, 2, funcPtr, argPtr);
    id = qpush(newThread, threadq);
    swapcontext(current, newThread);
    interruptsAreDisabled = 0;
    return id;
}

void threadYield() {
    ucontext_t *current, *next;
    current = getThread(qcurrent(threadq)); //searches for context in queue that has
    //same stack, therefore same thread
    if(current==NULL)
        current = getThread(qcurrent(blockq)); //searches in blockq, too for threadJoin()
    interruptsAreDisabled = 1;
    if(current==NULL) {
        perror("current thread missing from scheduler, reinstantiating as ");
        getcontext(current);
        int id = qpush(current, threadq);
        fprintf(stderr, "threadID = %d", id);
    }
    next = getThread(qrotate(threadq));
    swapcontext(current, next);
    interruptsAreDisabled = 0;
    return;
}

void threadJoin(int thread_id, void **result) {
    int ided = getID(qfind(thread_id, threadq));
    if(ided==INVALID)
        ided = getID(qfind(thread_id, blockq));
    if(ided==INVALID) {
        interruptsAreDisabled = 0;
        return; //not found in either list
    }
    qnode *current_block = qcurrent(threadq);
    interruptsAreDisabled = 1;
    qremove(current_block, threadq);
    setWait(current_block, thread_id);
    qadd(current_block, blockq);
    threadYield();
    if(result!=NULL)
        *result = *result_holder;
    interruptsAreDisabled = 0;
    return;
}

//exits the current thread -- calling this in the main thread should terminate the program
void threadExit(void *result) {
    qnode *current, *unblocked;
    interruptsAreDisabled = 1;
    if(qremove((current = qcurrent(threadq)), threadq)==INVALID) {
        //handling of missing thread from handler
        perror("missing current thread\n");
        exit(1);
    }
    if(getID(current)==-2) { //this is the main thread, deconstructor follows
        lockqnode *lock;
        qremove(current, threadq);
        while(threadq->size>0) {
            unblocked = threadq->head;
            qremove(unblocked, threadq);
            free(unblocked->thread->uc_stack.ss_sp);
            free(unblocked->thread);
            free(unblocked);
        }
        free(threadq); //freed thread queue
        while(blockq->size>0) {
            unblocked = blockq->head;
            qremove(unblocked, blockq);
            free(unblocked->thread->uc_stack.ss_sp);
            free(unblocked->thread);
            free(unblocked);
        }
        free(blockq); //freed blocked thread queue
        while(locks->size>0) {
            lock = locks->head;
            lockoff(lock->lock, locks);
        }
        free(locks); //freed lock queue
        free(result_holder);
        exit(0);
    }
    unblocked = qwaitfind(getID(current), blockq);
    if(unblocked!=NULL) {
        qremove(unblocked, blockq);
        qfront(unblocked, threadq);
        *result_holder = result;
        setWait(unblocked, -1);
    }
    qremove(current, threadq);
    free(current->thread->uc_stack.ss_sp);
    free(current->thread);
    free(current);
    interruptsAreDisabled = 0;
    return;
}

void threadLock(int lockNum) {
    if(lockNum<0||lockNum>=NUM_LOCKS) {
        perror("invalid lock number\n");
        return;
    }
    qnode *current_block = qcurrent(threadq);
    interruptsAreDisabled = 1;
    if(findlock(lockNum, locks)==INVALID) {
        qremove(current_block, threadq);
        setWait(current_block, lockNum);
        qadd(current_block, blockq);
        threadYield();
    }
    lockon(lockNum, -1, getID(current_block), locks);
    interruptsAreDisabled = 0;
    return;
}
void threadUnlock(int lockNum) {
    if(lockNum<0||lockNum>=NUM_LOCKS) {
        perror("invalid lock number.\n");
        return;
    }
    interruptsAreDisabled = 1;
    if(findlock(lockNum, locks)!=-1) {
        perror("lock has signal or is not found");   
        interruptsAreDisabled = 0;
        return;
    }
    qnode *nextup = qwaitfind(lockNum, blockq);
    if(lockoff(lockNum, locks)!=getID(nextup)) {
        perror("not locked right now.\n");
        interruptsAreDisabled = 0;
        return;
    }
    if(nextup!=NULL) {
        qremove(nextup, blockq);
        setWait(nextup, -1);
        qfront(nextup, threadq);
    }
    interruptsAreDisabled = 0;
    return;
}
void threadWait(int lockNum, int conditionNum); 
void threadSignal(int lockNum, int conditionNum); 

//this needs to be defined in your library. Don't forget it or some of my tests won't compile.
int interruptsAreDisabled;