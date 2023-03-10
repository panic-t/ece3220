/* <ryan ware>
ece3220 s2023
would you believe they're actually stacks?
*/

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
    qnode *held = q->head;
    q->head = (qnode *)malloc(sizeof(qnode));
    q->head = (qnode *)malloc(sizeof(qnode));
    q->head->key = currentID--;
    q->head->thread = newContext;
    q->head->link = held;
    q->head->waiting = -1;
    q->head->sig = -1;
    if(q->size==0)
        q->tail = q->head;
    q->size++;
    return q->head->key;
}


//forces node to front of queue, returns thread id when successful, INVALID on else
int qfront(qnode *node, queue *q) {
    if(q==NULL||node==NULL) 
        return INVALID;
    node->link = q->head;
    q->head = node;
    if(q->size==0)
        q->tail = node;
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

//pops off top block, returns ptr to node, return NULL on NULL/empty queue; DEPRECATED
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

//same as qfind(), but searches wait values
qnode *qwaitfind(int search, int signal, queue *q) {
    if(q==NULL)
        return NULL;
    qnode *slider;
    for(slider=q->head;slider!=NULL&&(slider->waiting!=search||slider->sig!=signal);slider=slider->link);
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
    if(q->size==1) {
        q->head = NULL;
        q->tail = NULL;
        q->size = 0;
        return target->key;
    }
    qnode *prev, *ret;
    if(target==q->head) {
        q->head = q->head->link;
        q->size--;
        return target->key;
    }
    for(prev=q->head; prev->link!=target; prev=prev->link);
    ret = prev->link;
    prev->link = ret->link;
    if(q->tail==ret) {
        q->tail = q->head;
        while(q->tail->link!=NULL)
            q->tail = q->tail->link;
    }
    target->link = NULL;
    q->size--;
    return ret->key;
}

//adds node to queue, returns key of node unless failed, then returns INVALID
int qadd(qnode *node, queue *q) {
    if(node==NULL||q==NULL)
        return INVALID;
    if(q->size==0) {
        q->head = node;
        q->tail = node;
        q->size = 1;
        return node->key;
    }
    q->tail->link = node;
    q->tail = q->tail->link;
    node->link = NULL;
    q->size++;
    return node->key;
}

//deletes node from queue, frees it, returns thread context; DEPRECATED
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

void setSig(qnode *target, int signal) {
    if(target==NULL)
        return;
    target->sig = signal;
    return;
}

int getSig(qnode *target) {
    if(target==NULL)
        return INVALID;
    return target->sig;
}

//lockon(), lockoff(), and findlock() are for accessing lock array, should
//return lock holder id or INVALID if wrong 
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
int findlock(int locknum, int signal, lockqueue *q) {
    if(q==NULL)
        return INVALID;
    lockqnode *slider;
    for(slider=q->head; slider!=NULL&&(slider->lock!=locknum||slider->sig!=signal); slider=slider->link);
    if(slider==NULL)
        return INVALID;
    return slider->holder;
}
int lockoff(int locknum, int signal, lockqueue *q) {
    int ret;
    if(findlock(locknum, signal, q)==INVALID)
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
    if(q->head->lock==locknum&&q->head->sig==signal) {
        prev = q->head;
        q->head = q->head->link;
        ret = prev->holder;
        free(prev);
        q->size--;
        return ret;
    }
    for(prev=q->head; prev!=NULL&&prev->link->lock!=locknum; prev=prev->link);
    if(q->tail==prev->link)
        q->tail = prev;
    ret = prev->link->holder;
    free(prev->link);
    prev->link = prev->link->link;
    return ret;
}