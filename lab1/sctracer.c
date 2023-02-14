/*
*/
#include <sys/ptrace.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

int main(int argc, char *argv[]) {
    pid_t traced;
    if((traced = fork())) {
        //parent - encap function
    } else {
        //child - sub program
        ptrace(PTRACE_TRACEME);
        kill(getpid(), SIGSTOP);
    }
}