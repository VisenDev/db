BUILDDIR= ./.build/
SQLITESRC= ./3rdparty/sqlite/sqlite3.c
SRC= main.c
LIBRAYLIB= ./3rdparty/raylib/libraylib.a
CC= gcc
STD= c99
CFLAGS= -Wall -Wextra -Wpedantic -std=$(STD) -fsanitize=undefined -g -I3rdparty -lm
LDFLAGS= -L./3rdparty/raylib/ -lraylib -L./.build/ -lsqlite


all: main

$(LIBRAYLIB):
	cd 3rdparty/raylib && PLATFORM=PLATFORM_DESKTOP_GLFW make

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

main: $(BUILDDIR)libsqlite.a $(SRC) $(LIBRAYLIB) Makefile 
	$(CC) $(CFLAGS) $(SRC) -o main $(LDFLAGS)

$(BUILDDIR)sqlite.o: $(BUILDDIR) $(SQLITESRC)
	$(CC) -c $(SQLITESRC) -o $(BUILDDIR)sqlite.o

$(BUILDDIR)libsqlite.a: $(BUILDDIR)sqlite.o
	ar rcs $(BUILDDIR)libsqlite.a $(BUILDDIR)sqlite.o

run: main
	./main

clean:
	if [ -e $(BUILDDIR) ]; then trash $(BUILDDIR); fi
	if [ -e main ]; then trash main; fi
	cd 3rdparty/raylib/ && make clean
