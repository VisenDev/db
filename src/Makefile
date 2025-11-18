CC= gcc
STD= c99
WARNFLAGS= -Wall -Wextra -Wpedantic -Wdeclaration-after-statement
CFLAGS= $(WARNFLAGS) --std=$(STD) -g -I3rdparty -fsanitize=undefined 
LDFLAGS= -lm
OUT= main
SQLITEOBJ= 3rdparty/sqlite/sqlite3.o
SRC= main.c
LIBRAYLIB= 3rdparty/raylib/libraylib.a

ifeq ($(shell uname -s), Darwin)
	LDFLAGS += -lobjc -framework CoreFoundation -framework WebKit -framework Cocoa -framework IOKit
endif

all: main

$(LIBRAYLIB):
	cd 3rdparty/raylib && PLATFORM=PLATFORM_DESKTOP_GLFW make -j

$(SQLITEOBJ): 3rdparty/sqlite/sqlite3.c
	$(CC) -g -O3 -c 3rdparty/sqlite/sqlite3.c -o $(SQLITEOBJ)

$(OUT): $(SRC) $(SQLITEOBJ) Makefile $(LIBRAYLIB)
	$(CC) $(CFLAGS) $(SRC) $(SQLITEOBJ) $(LIBRAYLIB) -o $(OUT) $(LDFLAGS)

run: $(OUT)
	./$(OUT)

clean:
	if [ -e main ]; then trash main; fi
	if [ -e TAGS ]; then trash TAGS; fi
	if [ -e main.db ]; then trash main.db; fi
	if [ -e a.out ]; then trash a.out; fi
	if [ -e $(SQLITEOBJ) ]; then trash $(SQLITEOBJ); fi
	if [ -e build ]; then trash build; fi
	cd 3rdparty/raylib/ && make clean


TAGS:
	find . -name '*.[ch]' -type f -exec etags {} +

.PHONY: main TAGS

