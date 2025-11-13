CC= clang
STD= c99
CFLAGS= -Wall -Wextra -Wpedantic -std=$(STD) -g
LDFLAGS=
OUT= main

ifeq '$(findstring ;,$(PATH))' ';'
	OS := Windows
else
	OS := $(shell uname 2>/dev/null || echo Unknown)
	OS := $(patsubst CYGWIN%,Cygwin,$(OS))
	OS := $(patsubst MSYS%,MSYS,$(OS))
	OS := $(patsubst MINGW%,MSYS,$(OS))
endif

ifeq ($(OS),Windows)
	LDFLAGS += -ggdb -ldwmapi -lshell32 -lwinmm -lgdi32 -lopengl32
	OUT= main.exe
endif
ifeq ($(OS),Darwin)
	LDFLAGS += -lm -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo 
endif
ifeq ($(OS),Linux)
    LDFLAGS += -lXrandr -lX11 -lm -lGL
endif

all: main

3rdparty/sqlite/sqlite3.o: 3rdparty/sqlite/sqlite3.c Makefile
	$(CC) $(CFLAGS) -c 3rdparty/sqlite/sqlite3.c -o 3rdparty/sqlite/sqlite3.o

$(OUT): main.c 3rdparty/sqlite/sqlite3.o Makefile
	$(CC) $(CFLAGS) main.c 3rdparty/sqlite/sqlite3.o -o $(OUT) $(LDFLAGS)

run: $(OUT)
	./$(OUT)

clean:
	if [ -e main ]; then trash main; fi
	if [ -e TAGS ]; then trash TAGS; fi
	if [ -e main.db ]; then trash main.db; fi
	if [ -e a.out ]; then trash a.out; fi
	if [ -e 3rdparty/sqlite/sqlite3.o ]; then trash 3rdparty/sqlite/sqlite3.o; fi
	cd 3rdparty/raylib/ && make clean


TAGS:
	find . -name '*.[ch]' -type f -exec etags {} +

.PHONY: main TAGS
