
CC=clang
CFLAGS=-Wall -g
BINS=contextfun contexts funcptr makecontext_example #cooperative_test preemptive_test


all: $(BINS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^ 

clean:
	rm -rf $(BINS) *.o *.dSYM
