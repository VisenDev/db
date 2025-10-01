#include <wayland-client.h>
#include <unistd.h>

/* 3rdparty */
#include <raylib/raylib.h>
#include <sqlite/sqlite3.h>
#define CORE_IMPLEMENTATION
#include <core.h/core.h>


int main() {
    //    create_new_db("main.db");
    InitWindow(1000, 1000, "hello");

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }
    
    CloseWindow();
}
