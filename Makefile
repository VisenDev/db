CFLAGS= -Wall -Wextra -Wpedantic -std=c99



main: main.c core.h sqlite3.o
	cc $(CFLAGS) sqlite3.o main.c -o main

sqlite3.o: sqlite/sqlite3.c
	cc sqlite/sqlite3.c -c -o sqlite3.o

run: main
	./main
