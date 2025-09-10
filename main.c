#include "./3rdparty/raylib/raylib.h"
#include "./3rdparty/sqlite/sqlite3.h"
#define CORE_IMPLEMENTATION
#include "./3rdparty/core.h/core.h"

char * read_whole_file(core_Arena * a, const char * filepath) {
    FILE * fp = fopen(filepath, "r");
    assert(fp);
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    char * buf = core_arena_alloc(a, length + 1);
    memset(buf, 0, length + 1);
    assert(buf);
    fseek(fp, 0, SEEK_SET);
    fread(buf, 1, length, fp);
    fclose(fp);
    return buf;
}

void create_new_db(const char * filepath) {
    sqlite3 * db = NULL;
    if(sqlite3_open(filepath, &db) != 0) CORE_FATAL_ERROR("failed to create new db");
    core_Arena arena = {0};
    char * err = NULL;
    sqlite3_exec(db, read_whole_file(&arena, "sql/new_db.sql"), NULL, NULL, &err);
    if(err != NULL) CORE_FATAL_ERROR("%s", err);
    core_arena_free(&arena);
    sqlite3_close(db);
}



int main() {
    create_new_db("main.db");
    InitWindow(1000, 1000, "hello");

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
}
