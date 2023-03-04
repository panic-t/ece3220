#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "leakcount.h"

#define PIPENAME "/tmp/data_pipe"
#define PACKSIZE 80

stack_node *head;

int main(int argc, char const *argv[]) {
    pid_t check;
    FILE *pipe;
    char packet[PACKSIZE];
    if(!(check = fork())) {
        //child process, program to check
        umask(0);
        mkfifo(PIPENAME, 0666);
        setenv("LD_PRELOAD", "./memory_shim.so", 1);
        //CANNOT EXECUTE WITHOUT PERMS
        kill(getppid(), SIGCHLD);
        execvp(argv[1], (char * const *)&(argv[1]));
    } else {
        //parent process, leak checker
        wait(NULL);
        signal(SIGCHLD, wrapup);
        if((pipe = fopen(PIPENAME, "r"))==NULL) {
            perror("pipe cannot be read from.\n");
            exit(0);
        }
        /*
        void *ptrin;
        long sizein;
        char mode;
        stack_node *temp, *prev;
        */
        while(1) {
            printf("parent\n");
            //loop for managing packets
            fgets(packet, PACKSIZE, pipe);
            printf("%s\n", packet); //TEMP
            /*
            sscanf(packet, "%c %p %li", &mode, &ptrin, &sizein);
            switch(mode) {
                case 'm':
                //malloc handling
                    temp = head;
                    head = (stack_node *)malloc(sizeof(stack_node));
                    head->key = ptrin;
                    head->size = sizein;
                    head->link = temp;
                    break;
                case 'f':
                    if(head->link==NULL) {
                        if(head->key==ptrin) {
                            free(head);
                            head = NULL;
                        }
                        break;
                    }
                    temp = head->link;
                    prev = head;
                    while(temp->key!=ptrin && temp!=NULL) {
                        prev = temp;
                        temp = temp->link;
                    }
                    if(temp!=NULL) {
                        temp = temp->link;
                        free(prev->link);
                        prev->link = temp;
                    }
                    break;
                default:
                    break;
            }
            */
        }
    }
}

void wrapup() {
    remove(PIPENAME);
    printf("got to end\n"); //TEMP
    /*
    stack_node *revhead = head, *temp = NULL;
    int count = 0, total = 0;
    while(head) {
        revhead = head;
        head = head->link;
        revhead->link = temp;
        temp = revhead;
    }
    while(revhead) {
        count++;
        total += revhead->size;
        perror("LEAK\t%i\n", revhead->size);
        temp = revhead;
        revhead = revhead->link;
        free(temp);
    }
    perror("TOTAL\t%i\t%i\n", count, total);
    */
    exit(0);
}