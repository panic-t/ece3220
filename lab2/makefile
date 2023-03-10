
CC=gcc
CFLAGS=-Wall -g
BINS=libmythreads.a 
OFILES=mythreads.o queue.o threadwrapper.o #for removing objects

all: $(BINS)

libmythreads.a: mythreads.c mythreads.h queue.c threadwrapper.c
	$(CC) $(CFLAGS) -c mythreads.c queue.c threadwrapper.c
	ar -cvrs libmythreads.a mythreads.o queue.o threadwrapper.o
	$(RM) $(OFILES)

tar: mythreads.c mythreads.h queue.c threadwrapper.c
	tar cvzf project2.tgz makefile mythreads.c mythreads.h queue.c queue.h threadwrapper.c threadwrapper.h

test: tester.c libmythreads.a 
	$(CC) $(CFLAGS) -o test tester.c libmythreads.a

clean:
	$(RM) $(BINS)
	$(RM) -r *.dSYM
