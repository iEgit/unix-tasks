test: build
	cat test1 | ./a
	cat test2 | ./a
	cat test3 | ./a

build: strings.c
	gcc -g -rdynamic strings.c -o a
