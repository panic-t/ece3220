CC=gcc
CFLAGS=-Wall -g

BINS= memory_shim test leakcount sctracer


all: $(BINS)

test: test.c
	$(CC) $(CFLAGS) -o test test.c

leakcount: leakcount.c leakcount.h
	$(CC) $(CFLAGS) -o leakcount leakcount.c leakcount.h
	chmod +x leakcount

memory_shim: memory_shim.c
	$(CC) $(CFLAGS) -shared -fPIC memory_shim.c -o memory_shim.so -ldl

sctracer: sctracer.c sctracer.h
	$(CC) $(CFLAGS) -o sctracer sctracer.c sctracer.h

clean:
	$(RM) $(BINS)
	$(RM) -r *.dSYM
