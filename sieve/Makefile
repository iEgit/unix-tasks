test: build
	./a 10000000 1
	./a 10000000 2
	./a 10000000 5
	./a 10000000 10

build: sieve.c
	gcc -g sieve.c -o a -lpthread -lm
