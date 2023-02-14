#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PIPENAME "/tmp/data_pipe"

void main(int argc, char const *argv[]) {
    pid_t check;
    umask(0);
    mkfifo(PIPENAME, 0666);
    FILE *pipe;
    if(!(check = fork())) {
        //child process, program to check
        
        execvpe(argv[1], argv+1);
    } else {
        //parent process, leak checker
        if((pipe = fopen(PIPENAME, "r"))==NULL) {
            perror("pipe cannot be read from.\n");
            exit(0);
        }
    }
}