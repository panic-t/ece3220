
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "mythreads.h"
#include "threadwrapper.h"

void ufuncwrap(thFuncPtr func, void *argv) {
    interruptsAreDisabled = 0;
    threadExit((*func)(argv));
    return;
}
