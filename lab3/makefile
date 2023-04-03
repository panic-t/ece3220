CC=gcc
CFLAGS=-Wall -g

BINS= libmyalloc

all: $(BINS)

libmyalloc: allocator.c allocator.h
	$(CC) $(CFLAGS) -O2 -DNDEBUG -shared -fPIC allocator.c -o libmyalloc.so

clean:
	$(RM) $(BINS)
	$(RM) -r *.dSYM
