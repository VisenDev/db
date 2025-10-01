BUILDDIR= ./.build/
LIBSQLITE= $(BUILDDIR)sqlite3.o
SQLITESRC= ./3rdparty/sqlite/sqlite3.c
SRC= main.c
LIBRAYLIB= ./3rdparty/raylib/libraylib.a
CC= gcc
STD= c99
CFLAGS= -Wall -Wextra -Wpedantic -std=$(STD) -fsanitize=undefined -g -I3rdparty -lm
LDFLAGS= -L./3rdparty/raylib/ -lraylib


all: main

$(LIBRAYLIB):
	cd 3rdparty/raylib && PLATFORM=PLATFORM_DESKTOP_GLFW make

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

main: $(LIBSQLITE) $(SRC) $(LIBRAYLIB) Makefile 
	$(CC) $(CFLAGS) $(LIBSQLITE) $(SRC) -o main $(LDFLAGS)


$(LIBSQLITE): $(BUILDDIR) $(SQLITESRC)
	$(CC) -c $(SQLITESRC) -o $(LIBSQLITE)

run: main
	./main

clean:
	if [ -e $(BUILDDIR) ]; then trash $(BUILDDIR); fi
	if [ -e main ]; then trash main; fi
	cd 3rdparty/raylib/ && make clean
