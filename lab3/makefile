CC=clang
CFLAGS=-Wall -g

BINS= libmyalloc.so
EXTBINS= libmyalloc.so slowalloc.so test

all: $(BINS)

test: test.c 
	$(CC) $(CFLAGS) test.c -o test

tar: allocator.c allocator.h README makefile
	tar cvzf project3.tgz makefile README allocator.c allocator.h

slowalloc.so: allocator.c allocator.h
	$(CC) $(CFLAGS) -shared -fPIC allocator.c -o slowalloc.so

libmyalloc.so: allocator.c allocator.h
	$(CC) $(CFLAGS) -O2 -DNDEBUG -shared -fPIC allocator.c -o libmyalloc.so

clean:
	$(RM) $(EXTBINS)
	$(RM) -r *.dSYM
