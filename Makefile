CFLAGS=-std=c11 -g -static

tcc: tcc.c

test: tcc
	./test.sh

clean:
	rm -f tcc *.o *~ tmp*

.PHONY: test clean

