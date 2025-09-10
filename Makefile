CFLAGS= -Wall -Wextra -Wpedantic -std=c99 -fsanitize=address,undefined -g
BUILDDIR= ./.build/
LIBSQLITE= $(BUILDDIR)sqlite3.o
SQLITESRC= ./3rdparty/sqlite/sqlite3.c
SRC= main.c
LIBRAYLIB= ./3rdparty/raylib/libraylib.a

all: main

$(LIBRAYLIB):
	cd 3rdparty/raylib/ && make

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

main: $(LIBSQLITE) $(SRC) $(LIBRAYLIB) Makefile 
	cc $(CFLAGS) -l raylib -L $(LIBRAYLIB) $(LIBSQLITE) $(SRC) -o main

$(LIBSQLITE): $(BUILDDIR) $(SQLITESRC)
	cc -c -O1 $(SQLITESRC) -o $(LIBSQLITE)

run: main
	./main

clean:
	if [ -e $(BUILDDIR) ]; then trash $(BUILDDIR); fi
	if [ -e main ]; then trash main; fi
	cd 3rdparty/raylib/ && make clean
