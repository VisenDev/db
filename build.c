#define CORE_IMPLEMENTATION
#include "src/3rdparty/core.h/core.h"


#if defined(CORE_UNIX)
#   define CC "cc "
#elif defined(CORE_WINDOWS)
#   define CC "cl.exe "
#endif

#define SQLITE_SRC "src/3rdparty/sqlite/sqlite3.c"
#define SQLITE_OBJ "src/3rdparty/sqlite/sqlite3.o"

const char * echo(const char * str) {puts(str); return str;}

void sqlite_obj(void) {
    const char * cmd = CC "-c " SQLITE_SRC " -o " SQLITE_OBJ;
    static const char * deps[] = { SQLITE_SRC, "build.c" };
    if(core_file_needs_update(SQLITE_OBJ, deps, CORE_ARRAY_LEN(deps))) {
        puts(cmd);
        if(system(cmd) != 0) CORE_FATAL_ERROR("Failed to build sqlite obj");
    } else {
        puts(SQLITE_OBJ " up to date!");
    }
}

/* cc -DPLATFORM_DESKTOP_GLFW -c raudio.c rcore.c rmodels.c rshapes.c rtext.c rtextures.c utils.c */
/* ar rcs libraylib.a raudio.o rcore.o rmodels.o rshapes.o rtext.o rtextures.o utils.o */

#define RAYLIB_DIR "src/3rdparty/raylib/"
static const char * raylib_obj[] = {
    RAYLIB_DIR"raudio.o",
    RAYLIB_DIR"rcore.o",
    RAYLIB_DIR"rmodels.o",
    RAYLIB_DIR"rshapes.o",
    RAYLIB_DIR"rtext.o",
    RAYLIB_DIR"rtextures.o",
    RAYLIB_DIR"utils.o",
    RAYLIB_DIR"rglfw.o",
};
static const char * raylib_src[] = {
   RAYLIB_DIR"raudio.c",
   RAYLIB_DIR"rcore.c",
   RAYLIB_DIR"rmodels.c",
   RAYLIB_DIR"rshapes.c",
   RAYLIB_DIR"rtext.c",
   RAYLIB_DIR"rtextures.c",
   RAYLIB_DIR"utils.c",    
   RAYLIB_DIR"rglfw.c"    
};
CORE_STATIC_ASSERT((CORE_ARRAY_LEN(raylib_src) == CORE_ARRAY_LEN(raylib_obj)), "src and obj arrays should be equal")

void libraylib(void) {
    unsigned long i;
    char cmd[1024];
    unsigned long fill = 0;

    for(i = 0; i < CORE_ARRAY_LEN(raylib_src); ++i) {
        static const char * deps[2] = {0};
        deps[0] = raylib_src[i];
        deps[1] = "build.c";
        if(core_file_needs_update(raylib_obj[i], deps, CORE_ARRAY_LEN(deps))) {
            fill = 0;
            core_strfmt(cmd, sizeof(cmd), &fill, CC "-c -I"RAYLIB_DIR"external/glfw/include -I"RAYLIB_DIR" ");
            #ifdef __linux__
                core_strfmt(cmd, sizeof(cmd), &fill, "-D_GLFW_WAYLAND -D_GLFW_X11 ");
            #endif /*__linux__*/
                
            #ifdef __APPLE__
                core_strfmt(cmd, sizeof(cmd), &fill, "-x objective-c ");
            #endif /*__APPLE__*/

            core_strfmt(cmd, sizeof(cmd), &fill, "-DPLATFORM_DESKTOP_GLFW ");
            core_strfmt(cmd, sizeof(cmd), &fill, raylib_src[i]);
            core_strfmt(cmd, sizeof(cmd), &fill, " -o ");
            core_strfmt(cmd, sizeof(cmd), &fill, raylib_obj[i]);

            puts(cmd);
            if(system(cmd) != 0) CORE_FATAL_ERROR("Failed to compile raylib file");
        } else {
            printf("%s%s\n", raylib_obj[i], " up to date!");
        }
    }
}

void all(void) {
    char cmd[1024];
    unsigned long fill = 0;
    unsigned long i;

    sqlite_obj();
    libraylib();
    core_strfmt(cmd, sizeof(cmd), &fill, CC);
    core_strfmt(cmd, sizeof(cmd), &fill, "src/main.c ");
    core_strfmt(cmd, sizeof(cmd), &fill, "-Isrc/3rdparty/ ");
    core_strfmt(cmd, sizeof(cmd), &fill, "-Wall -Wextra -Wpedantic -std=c99 -fsanitize=undefined ");
    core_strfmt(cmd, sizeof(cmd), &fill, SQLITE_OBJ);
    for(i = 0; i < CORE_ARRAY_LEN(raylib_obj); ++i) {
        core_strfmt(cmd, sizeof(cmd), &fill, " ");
        core_strfmt(cmd, sizeof(cmd), &fill, raylib_obj[i]);
    }
    core_strfmt(cmd, sizeof(cmd), &fill, " -o main");
    core_strfmt(cmd, sizeof(cmd), &fill, " -lm ");
#   ifdef __APPLE__
    core_strfmt(cmd, sizeof(cmd), &fill, "-lobjc -framework CoreFoundation -framework WebKit -framework Cocoa -framework IOKit ");
#   endif /*__APPLE__*/

    puts(cmd);
    if(system(cmd) != 0) CORE_FATAL_ERROR("Failed to build main");
}

void clean_file(const char * path) {
    if(core_file_modified_timestamp(path) <= 0) return;
    printf("remove(\"%s\");\n", path);
    if(remove(path) != 0) CORE_FATAL_ERROR("Failed to delete file");
}

void clean(void) {
    unsigned long i;
    clean_file("main");
    clean_file(SQLITE_OBJ);
    for(i = 0; i < CORE_ARRAY_LEN(raylib_obj); ++i) {
        clean_file(raylib_obj[i]);
    }
}

void run(void) {
    all();
    system("./main");
}

void update_core_h(void) {
    system(echo("wget https://github.com/VisenDev/core.h/raw/refs/heads/master/core.h --directory-prefix=src/3rdparty/core.h/"));
}

void rebuild_build_c(int argc, char ** argv) {
    char buf[1024];
    unsigned long fill = 0;
    int i = 0;
  
    puts("\"build.c\" has changed, recompiling...");
    if(rename(argv[0], ".old-build") != 0 ) CORE_FATAL_ERROR("Failed to rename old executable");
    fill = 0;

    core_strfmt(buf, sizeof(buf), &fill, CC);
    core_strfmt(buf, sizeof(buf), &fill, "build.c -o ");
    core_strfmt(buf, sizeof(buf), &fill, argv[0]);
    
    puts(buf);
    if(system(buf) != 0) CORE_FATAL_ERROR("Failed to rebuild build.c");
    
    fill = 0;
    for(i = 0; i < argc; ++i) {
        core_strfmt(buf, sizeof(buf), &fill, argv[i]);
        core_strfmt(buf, sizeof(buf), &fill, " ");
    }

    puts(buf);
    system(buf);
    clean_file(".old-build");
    exit(0);
}

int main(int argc, char ** argv) {
    static const char * input[] = {"build.c"};

    if(core_file_needs_update(argv[0], input, CORE_ARRAY_LEN(input))) {
        rebuild_build_c(argc, argv);
    }

    if(argc == 1) {
        all();
    } else if(argc == 2) {
        if(strcmp(argv[1], "clean") == 0) {
            clean();
        } else if(strcmp(argv[1], "run") == 0) {
            run();
        } else if(strcmp(argv[1], "all") == 0) {
            all();
        } else if(strcmp(argv[1], "update_core.h") == 0) {
            update_core_h();
        } else {
            CORE_FATAL_ERROR("Unknown argument, expected on of: clean run all update_core.h");
        }
    } else {
        CORE_FATAL_ERROR("Expected 0 or 1 arguments");
    }
    

    return 0;
}
