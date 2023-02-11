#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void main(int argc, char const *argv[]) {
    pid_t check;
    if(!(check = fork())) {
        //child process, program to check
        
        execv(argv[1], argv+1);
    } else {
        //parent process, leak checker
    }
}