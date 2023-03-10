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

#define MAIN -2;

queue *threadq, *blockq;
lockqueue *locks;
int current;
void **result_holder;

void threadInit() {
    interruptsAreDisabled = 1;
    ucontext_t *currentBlock = (ucontext_t *)malloc(sizeof(ucontext_t));
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
    current = MAIN;
    getcontext(currentBlock);
    qpush(currentBlock, threadq);
    interruptsAreDisabled = 0;
    return;
}

int threadCreate(thFuncPtr funcPtr, void *argPtr) {
    int id;
    ucontext_t *newThread = (ucontext_t *)malloc(sizeof(ucontext_t)), 
    *currentThread = getThread(qfind(current, threadq));
    interruptsAreDisabled = 1;
    getcontext(newThread);
    newThread->uc_stack.ss_sp = malloc(STACK_SIZE);
    newThread->uc_stack.ss_size = STACK_SIZE;
    newThread->uc_stack.ss_flags = 0;
    makecontext(newThread, (void(*)())&ufuncwrap, 2, funcPtr, argPtr);
    id = qpush(newThread, threadq);
    current = id;
    swapcontext(currentThread, newThread);
    interruptsAreDisabled = 0;
    return id;
}

void threadYield() {
    ucontext_t *currentThread, *next;
    currentThread = getThread(qfind(current, threadq)); //searches for context in queue that has
    //same stack, therefore same thread
    if(currentThread==NULL)
        currentThread = getThread(qfind(current, blockq)); //searches in blockq, too for threadJoin()
    interruptsAreDisabled = 1;
    if(currentThread==NULL) {
        perror("current thread missing from scheduler, reinstantiating as ");
        getcontext(currentThread);
        int id = qpush(currentThread, threadq);
        fprintf(stderr, "threadID = %d", id);
    }
    qnode *nextBlock = qrotate(threadq);
    next = getThread(nextBlock);
    current = getID(nextBlock);
    swapcontext(currentThread, next);
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
    qnode *current_block = qfind(current, threadq);
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
    qnode *currentBlock, *unblocked;
    interruptsAreDisabled = 1;
    if(qremove((currentBlock = qfind(current, threadq)), threadq)==INVALID) {
        //handling of missing thread from handler
        perror("missing current thread\n");
        exit(1);
    }
    if(current==-2) { //this is the main thread, deconstructor follows
        lockqnode *lock;
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
            lockoff(lock->lock, lock->sig, locks);
        }
        free(locks); //freed lock queue
        free(result_holder);
        exit(0);
    }
    unblocked = qwaitfind(current, -1, blockq);
    if(unblocked!=NULL) {
        qremove(unblocked, blockq);
        qfront(unblocked, threadq);
        *result_holder = result;
        setWait(unblocked, -1);
    }
    free(currentBlock->thread->uc_stack.ss_sp);
    free(currentBlock->thread);
    free(currentBlock);
    unblocked = qrotate(threadq);
    current = getID(unblocked);
    setcontext(getThread(unblocked));
    interruptsAreDisabled = 0;
    return;
}

void threadLock(int lockNum) {
    if(lockNum<0||lockNum>=NUM_LOCKS) {
        perror("invalid lock number\n");
        return;
    }
    qnode *current_block = qfind(current, threadq);
    interruptsAreDisabled = 1;
    if(findlock(lockNum, -1, locks)==INVALID) {
        qremove(current_block, threadq);
        setWait(current_block, lockNum);
        qadd(current_block, blockq);
        threadYield();
    }
    lockon(lockNum, getID(current_block), -1, locks);
    interruptsAreDisabled = 0;
    return;
}
void threadUnlock(int lockNum) {
    if(lockNum<0||lockNum>=NUM_LOCKS) {
        perror("invalid lock number.\n");
        return;
    }
    interruptsAreDisabled = 1;
    if(findlock(lockNum, -1, locks)==INVALID) {
        perror("lock has signal or is not found");   
        interruptsAreDisabled = 0;
        return;
    }
    qnode *nextup = qwaitfind(lockNum, -1, blockq);
    if(lockoff(lockNum, -1, locks)!=getID(nextup)) {
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
void threadWait(int lockNum, int conditionNum) {
    if(lockNum<0||lockNum>=NUM_LOCKS||conditionNum<0||conditionNum>=CONDITIONS_PER_LOCK) {
        perror("invalid lock or condition.\n");
        return;
    }
    if(INVALID==findlock(lockNum, -1, locks)) {
        perror("no lock active.\n");
        return;
    }
    interruptsAreDisabled = 1;
    qnode *currentBlock = qfind(current, threadq);
    qremove(currentBlock, threadq);
    setWait(currentBlock, lockNum);
    qadd(currentBlock, blockq);
    lockoff(lockNum, -1, locks);
    lockon(lockNum, current, conditionNum, locks);
    threadYield();
    lockon(lockNum, current, -1, locks);
    interruptsAreDisabled = 0;
    return;
}
void threadSignal(int lockNum, int conditionNum) {
    if(lockNum<0||lockNum>=NUM_LOCKS||conditionNum<0||conditionNum>=CONDITIONS_PER_LOCK) {
        perror("invalid lock or condition.\n");
        return;
    }
    if(findlock(lockNum, conditionNum, locks)!=conditionNum) {
        perror("lock not found");
        return;
    }
    interruptsAreDisabled = 1;
    qnode *unblocking = qfind(findlock(lockNum, conditionNum, locks), blockq);
    qremove(unblocking, blockq);
    setWait(unblocking, -1);
    qfront(unblocking, threadq);
    lockoff(lockNum, conditionNum, locks);
    interruptsAreDisabled = 0;
}
//this needs to be defined in your library. Don't forget it or some of my tests won't compile.
int interruptsAreDisabled;