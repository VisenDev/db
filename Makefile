CFLAGS= -Wall -Wextra -Wpedantic -std=c99 -fsanitize=address,undefined -g


main: main.c core.h sqlite3.o Makefile
	cc $(CFLAGS) sqlite3.o main.c -o main

sqlite3.o: sqlite/sqlite3.c
	cc sqlite/sqlite3.c -c -o sqlite3.o

run: main
	./main
