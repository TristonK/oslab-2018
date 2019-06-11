NAME := $(shell basename $(PWD))
export TASK := M6
all: $(NAME)-64.so $(NAME)-32.so
CFLAGS += -U_FORTIFY_SOURCE

include ../Makefile
include ../Makefile.lab

test32:
	make
	gcc -m32 -o test.out test.c libkvdb-32.so -lpthread -Wl,-rpath=./
	./a.out

test:
	make
	gcc -o test.out test.c libkvdb-64.so -lpthread -Wl,-rpath=./
	./a.out

de:
	rm test.out
