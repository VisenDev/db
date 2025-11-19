#define CORE_IMPLEMENTATION
#include "src/3rdparty/core.h/core.h"

#if defined(CORE_UNIX)
#   define CC "gcc "
#elif defined(CORE_WINDOWS)
#   define CC "cl.exe "
#endif

#define SQLITE_SRC "src/3rdparty/sqlite/sqlite3.c "
#define SQLITE_OBJ "src/3rdparty/sqlite/sqlite3.o "

void sqlite_obj(void) {
    if(core_file_exists(SQLITE_OBJ)) return;
    if(system(CC "-c " SQLITE_SRC "-o " SQLITE_OBJ) != 0) CORE_FATAL_ERROR("Failed to build sqlite obj");
}

/* cc -DPLATFORM_DESKTOP_GLFW -c raudio.c rcore.c rmodels.c rshapes.c rtext.c rtextures.c utils.c */
/* ar rcs libraylib.a raudio.o rcore.o rmodels.o rshapes.o rtext.o rtextures.o utils.o */

#define RAYLIB_DIR "src/3rdparty/raylib/"
#define RAYLIB_SRC                                  \
    RAYLIB_DIR"raudio.c"    " "                     \
    RAYLIB_DIR"rcore.c"     " "                     \
    RAYLIB_DIR"rmodels.c"   " "                     \
    RAYLIB_DIR"rshapes.c"   " "                     \
    RAYLIB_DIR"rtext.c"     " "                     \
    RAYLIB_DIR"rtextures.c" " "                     \
    RAYLIB_DIR"utils.c"     " "                     
#define RAYLIB_OBJ                                  \
    RAYLIB_DIR"raudio.o"    " "                     \
    RAYLIB_DIR"rcore.o"     " "                     \
    RAYLIB_DIR"rmodels.o"   " "                     \
    RAYLIB_DIR"rshapes.o"   " "                     \
    RAYLIB_DIR"rtext.o"     " "                     \
    RAYLIB_DIR"rtextures.o" " "                     \
    RAYLIB_DIR"utils.o"     " "                     
#define LIBRAYLIB RAYLIB_DIR"libraylib.a "
    
void libraylib(void) {
    const char * compile = CC "-c -DPLATFORM_DESKTOP_GLFW " RAYLIB_SRC " -o ray.o";
    const char * build_lib = "ar rcs " LIBRAYLIB " ray.o";
    if(core_file_exists(LIBRAYLIB)) return;
    puts(compile);
    if(system(compile) != 0) CORE_FATAL_ERROR("Failed to build raylib obj");
    puts(build_lib);
    if(system(build_lib) != 0) CORE_FATAL_ERROR("Failed to build libraylib");
}

int main(int argc, int argv) {
    sqlite_obj();
    libraylib();

    return 0;
}
