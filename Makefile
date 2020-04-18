CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *c)
OBJS=$(SRCS:.c=.o)

tcc: $(OBJS)
	$(CC) -o tcc $(OBJS) $(LDFLAGS)

$(OBJS): tcc.h

test: tcc
	./test.sh

clean:
	rm -f tcc *.o *~ tmp*

.PHONY: test clean

