#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <limits.h>
#include "queue.h"

//for assigning new ids
int currentID = -2;

//pushes new context onto queue, returns id of new thread, unless INVALID 
int qpush(ucontext_t *newContext, queue *q) {
    if(q==NULL)
        return INVALID;
    if(q->size==0) {
        q->head = (qnode *)malloc(sizeof(qnode));
        q->head->key = currentID--;
        q->head->thread = newContext;
        q->head->link = NULL;
        q->head->waiting = -1;
        q->tail = q->head;
        q->size++;
        return q->head->key;
    }
    q->tail->link = (qnode *)malloc(sizeof(qnode));
    q->tail = q->tail->link;
    q->tail->key = currentID++;
    q->tail->waiting = -1;
    q->tail->thread = newContext;
    q->tail->link = NULL;
    q->size++;
    return q->tail->key;
}


//forces node to front of queue, returns thread id when successful, INVALID on else
int qfront(qnode *node, queue *q) {
    if(q==NULL||node==NULL) 
        return INVALID;
    node->link = q->head;
    q->head = node;
    q->size++;
    return node->key;
}

//moves front thread to back, returns thread node of rotated process returns NULL on error
qnode *qrotate(queue *q) {
    if(q==NULL||q->size==0)
        return NULL;
    q->tail->link = q->head;
    q->head = q->head->link;
    q->tail = q->tail->link;
    q->tail->link = NULL;
    return q->tail;
}

//pops off top block, returns ptr to node, return NULL on NULL/empty queue
qnode *qpop(queue *q) {
    if(q==NULL||q->size==0)
        return NULL;
    if(q->size==1)
        q->tail = NULL;
    qnode *ret = q->head;
    q->head = q->head->link;
    q->size--;
    return ret;
}

//returns ptr to thread block with threadid, unless not found or null queue, returns NULL 
qnode *qfind(int search, queue *q) {
    if(q==NULL)
        return NULL;
    qnode *slider;
    for(slider=q->head;slider!=NULL&&slider->key!=search;slider=slider->link);
    if(slider!=NULL)
        return slider;
    else
        return NULL;
}

//finds current running thread qith qmatch()
qnode *qcurrent(queue *q) {
    ucontext_t current;
    getcontext(&current);
    return qmatch(&current, q);
}

//same as qfind(), but searches wait values
qnode *qwaitfind(int search, queue *q) {
    if(q==NULL)
        return NULL;
    qnode *slider;
    for(slider=q->head;slider!=NULL&&slider->waiting!=search;slider=slider->link);
    if(slider!=NULL)
        return slider;
    else
        return NULL;
}

//same as qfind, but instead matches ucontext to other ucontexts
qnode *qmatch(ucontext_t *search, queue *q) {
    if(q==NULL)
        return NULL;
    qnode *slider = q->tail;
    if(slider->thread->uc_stack.ss_sp!=search->uc_stack.ss_sp)
        for(slider=q->head;slider!=NULL&&slider->thread->uc_stack.ss_sp!=
search->uc_stack.ss_sp;slider=slider->link);
    if(slider!=NULL)
        return slider;
    else
        return NULL;
}

//removes node from queue without deleting, returns key, if failed returns INVALID
int qremove(qnode *target, queue *q) {
    if(q==NULL||target==NULL||q->size==0)
        return INVALID;
    if(target==NULL||q==NULL||qfind(target->key, q)==NULL)
        return INVALID;
    qnode *prev, *ret;
    for(prev=q->head; prev->link!=target; prev=prev->link);
    ret = prev->link;
    prev->link = ret->link;
    if(q->tail==ret) {
        q->tail = q->head;
        while(q->tail->link!=NULL)
            q->tail = q->tail->link;
    }
    q->size--;
    return ret->key;
}

//adds node to queue, returns key of node unless failed, then returns INVALID
int qadd(qnode *node, queue *q) {
    if(node==NULL||q==NULL)
        return INVALID;
    q->tail->link = node;
    q->size++;
    return node->key;
}

//deletes node from queue, frees it, returns thread context
ucontext_t *qdelete(qnode *target, queue *q) {
    if(target==NULL||q->size==0||qfind(target->key, q)==NULL)
        return NULL;
    ucontext_t *ret;
    if(target==q->head) 
        q->head = q->head->link;
    else {
        qnode *prev;
        for(prev=q->head; prev->link!=target; prev=prev->link);
        prev->link = target->link;
    }
    if(q->tail==target) {
        q->tail = q->head;
        while(q->tail->link!=NULL)
            q->tail = q->tail->link;
    }
    ret = target->thread;
    free(target);
    q->size--;
    return ret;
}

//returns thread context unless NULL, returns INVALID
ucontext_t *getThread(qnode *target) {
    if(target==NULL)
        return NULL;
    return target->thread;
}

//returns id of thread unless NULL, returns INVALID
int getID(qnode *target) {
    if(target==NULL)
        return INVALID;
    return target->key;
}

//sets wait value of thread block unless NULL
void setWait(qnode *target, int waitval) {
    if(target==NULL)
        return;
    target->waiting = waitval;
    return;
}

//gets wait value of thread block unless NULL, returns INVALID
int getWait(qnode *target) {
    if(target==NULL)
        return INVALID;
    return target->waiting;
}

//lockon(), lockoff(), and findlock() are for accessing lock array, should
//return lock holder id or INVALID if wrong (findlock returns signal)
int lockon(int locknum, int holdid, int signal, lockqueue *q) {
    if(q==NULL)
        return INVALID;
    if(q->size==0) {
        q->head = (lockqnode *)malloc(sizeof(lockqnode));
        q->tail = q->head;
    } else {
        q->tail->link = (lockqnode *)malloc(sizeof(lockqnode));
        q->tail = q->tail->link;
    }
    q->tail->link = NULL;
    q->tail->lock = locknum;
    q->tail->sig = signal;
    q->tail->holder = holdid;
    q->size++;
    return holdid;
}
int findlock(int locknum, lockqueue *q) {
    if(q==NULL)
        return INVALID;
    lockqnode *slider;
    for(slider=q->head; slider!=NULL&&slider->lock!=locknum; slider=slider->link);
    if(slider==NULL)
        return INVALID;
    return slider->sig;
}
int lockoff(int locknum, lockqueue *q) {
    int ret;
    if(findlock(locknum, q)==INVALID)
        return INVALID;
    if(q->size==1) {
        ret = q->head->holder;
        free(q->head);
        q->head = NULL;
        q->tail = NULL;
        q->size = 0;
        return ret;
    }
    lockqnode *prev;
    for(prev=q->head; prev!=NULL&&prev->link->lock!=locknum; prev=prev->link);
    if(q->tail==prev->link)
        q->tail = prev;
    ret = prev->link->holder;
    free(prev->link);
    prev->link = prev->link->link;
    return ret;
}