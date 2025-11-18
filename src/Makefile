CC ?= gcc
STD= c99
WARNFLAGS= -Wall -Wextra -Wpedantic -Wdeclaration-after-statement
CFLAGS= $(WARNFLAGS) -std=$(STD) -g -I3rdparty -fsanitize=undefined 
LDFLAGS= -lm
OUT= main
SQLITEOBJ= 3rdparty/sqlite/sqlite3.o
SRC= main.c
LIBRAYLIB= 3rdparty/raylib/libraylib.a

ifeq ($(shell uname -s), Darwin)
	LDFLAGS += -lobjc -framework CoreFoundation -framework WebKit -framework Cocoa -framework IOKit
endif

all: main

$(LIBRAYLIB): Makefile
	cd 3rdparty/raylib && CC=$(CC) PLATFORM=PLATFORM_DESKTOP_GLFW make -j

$(SQLITEOBJ): 3rdparty/sqlite/sqlite3.c Makefile
	$(CC) -g -c 3rdparty/sqlite/sqlite3.c -o $(SQLITEOBJ)

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

update_core.h:
	cd 3rdparty/core.h/ && trash core.h && wget https://github.com/VisenDev/core.h/raw/refs/heads/master/core.h

.PHONY: main TAGS update_core.h

