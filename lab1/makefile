CC=gcc
CFLAGS=-Wall -g

BINS= memory_shim test leakcount 


all: $(BINS)

test: test.c
	$(CC) $(CFLAGS) -o test test.c

leakcount: leakcount.c leakcount.h
	$(CC) $(CFLAGS) -o leakcount leakcount.c leakcount.h


memory_shim: memory_shim.c
	$(CC) $(CFLAGS) -shared -fPIC memory_shim.c -o memory_shim.so -ldl

clean:
	$(RM) $(BINS)
	$(RM) -r *.dSYM
