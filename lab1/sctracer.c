/*
*/
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/reg.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "sctracer.h"

#define SUBARGSIZE 512
#define TOKENSIZE 128
#define FILEBUFFERSIZE 128

int main(int argc, char *argv[]) {
    pid_t traced;
    if((traced = fork())) {
        //parent - encap function
        int cstat, syscall_num;
        FILE *output;
        bst_node *head = NULL, *found;
        waitpid(traced, &cstat, 0);
        ptrace(PTRACE_SETOPTIONS, traced, 0, PTRACE_O_TRACESYSGOOD);
        while (1) {
            do {
                ptrace(PTRACE_SYSCALL, traced, 0, 0);
                waitpid(traced, &cstat, 0);
                if(WIFEXITED(cstat)) {
                    found->count++;
                    output = fopen(argv[2], "w");
                    rec_file_print(output, head);
                    fclose(output);
                    exit(1);
                }
            } while(!(WIFSTOPPED(cstat) && WSTOPSIG(cstat)&0x08));
            syscall_num = ptrace(PTRACE_PEEKUSER, traced, sizeof(long)*ORIG_RAX, NULL);
            //insert to data structure
            found = search_insert_bst(&head, syscall_num); 
            found->count++;
            ptrace(PTRACE_CONT, traced, NULL, NULL);
        }
    } else {
        //child - sub program
        int argcsub = 0;
        char argvsub_whole[SUBARGSIZE], **argvsub;
        strcpy(argvsub_whole, argv[1]); //MAKE SURE THIS IS RIGHT
        while(strtok(argvsub_whole, " "))
            argcsub++;
        strcpy(argvsub_whole, argv[1]);
        argvsub = (char **)malloc(sizeof(char *)*argcsub);
        for(int i=0; i<argcsub; i++) {
            argvsub[i] = (char *)malloc(TOKENSIZE);
            strcpy(argvsub[i], strtok(argvsub_whole, " "));
        }
        ptrace(PTRACE_TRACEME);
        kill(getpid(), SIGSTOP);
        execvp((const char *)argvsub[0], (char * const *)argvsub);
    }
}

bst_node *search_insert_bst(bst_node **head, int searchkey) {
    bst_node *prev, *slider = *head;
    if(*head==NULL) {
        *head = (bst_node *)malloc(sizeof(bst_node));
        (*head)->key = searchkey;
        (*head)->count = 0;
        (*head)->left = NULL;
        (*head)->right = NULL;
        return *head;
    }
    while(slider!=NULL && slider->key!=searchkey) {
        prev = slider;
        if(slider->key>searchkey)
            slider = slider->left;
        else
            slider = slider->right;
    }
    if(slider==NULL) {
        slider = (bst_node *)malloc(sizeof(bst_node));
        slider->key = searchkey;
        slider->count = 0;
        slider->left = NULL;
        slider->right = NULL;
        if(prev->key>searchkey)
            prev->left = slider;
        else
            prev->right = slider;
    }
    return slider;
}

void rec_file_print(FILE *output, bst_node *head) {
    if(head==NULL)
        return;
    char file_buffer[FILEBUFFERSIZE];
    rec_file_print(output, head->left);
    sprintf(file_buffer, "%i\t%i\n", head->key, head->count);
    fputs(file_buffer, output);
    rec_file_print(output, head->right);
}

