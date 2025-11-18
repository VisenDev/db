/* stdlib */
#include <unistd.h>

/* 3rdparty */
#include <raylib/raylib.h>
#include <sqlite/sqlite3.h>
#define CORE_IMPLEMENTATION
#include <core.h/core.h>
#define NK_INCLUDE_DEFAULT_FONT
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define RAYLIB_NUKLEAR_INCLUDE_DEFAULT_FONT
#include <raylib-nuklear/raylib-nuklear.h>
#include <nuklear/style.c>

core_Bool db_exec_sql_file(sqlite3 * db, const char * sql_file_path) {
    core_Arena arena = {0};
    char * exec_err = NULL;
    FILE * fp = fopen(sql_file_path, "r");
    char * buf;
    core_Bool success = CORE_TRUE;
    if(!fp) return CORE_FALSE;
    buf = core_file_read_all_arena(&arena, fp);
    fclose(fp);
    if(sqlite3_exec(db, buf, NULL, NULL, &exec_err) != 0) success = CORE_FALSE;
    core_arena_free(&arena);
    return success;
}

typedef enum {
    MENU_TAG_NEW_ITEM,
    MENU_TAG_VIEW_TABLES
} MenuTag;

typedef enum {
    DB_STATUS_PENDING,
    DB_STATUS_DONE
} db_Status;

db_Status db_customer_new(sqlite3 * db, struct nk_context * ctx) {
    static char name[256];
    static int name_len = 0;
    static char email[256];
    static int email_len = 0;
    static char city[256];
    static int city_len = 0;
    db_Status ret = DB_STATUS_DONE;
    if(nk_group_begin(ctx, "New Customer", 0)) {
        nk_layout_row_dynamic(ctx, 25, 2);
    
        nk_label(ctx, "Customer Name: ", NK_TEXT_RIGHT);
        nk_edit_string(ctx, NK_EDIT_FIELD, name, &name_len, sizeof(name), NULL);

        nk_label(ctx, "Customer Email: ", NK_TEXT_RIGHT);
        nk_edit_string(ctx, NK_EDIT_FIELD, email, &email_len, sizeof(email), NULL);

        nk_label(ctx, "Customer City: ", NK_TEXT_RIGHT);
        nk_edit_string(ctx, NK_EDIT_FIELD, city, &city_len, sizeof(city), NULL);

        if(nk_button_label(ctx, "Save")) {
            int err;
            sqlite3_stmt * stmt;
            const char sql[] = "insert into customers(name, email, city) values(?, ?, ?);";
            err = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            if(err != SQLITE_OK) {
                fprintf(stderr, "\n%s\n", sqlite3_errstr(err));
                fprintf(stderr, "\n%s\n", sqlite3_errmsg(db));
                CORE_TODO("Add an error handling popup");
            }
            sqlite3_bind_text(stmt, 1, name, name_len, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, email, email_len, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, city, city_len, SQLITE_TRANSIENT);
            if(sqlite3_step(stmt) != SQLITE_DONE) {
                CORE_TODO("Handle error");
            }
            sqlite3_finalize(stmt);
            ret = DB_STATUS_DONE;
        } else {
            ret = DB_STATUS_PENDING;
        }
        
    }
    nk_group_end(ctx);
    return ret;
}

#define FILE_EXISTS(path) (access(path, F_OK) == 0)

int main() {
    sqlite3 * db = NULL;
    struct nk_context * ctx = NULL;
    struct nk_colorf bg;
    Font font;
    bool fix_nuklear_sizing_bug = false; /*For some reason nuklear doesn't work on mac until the window is resized*/
    bool init_database = false;
    const char * db_path = ".main.db";

    if(!FILE_EXISTS(db_path)) init_database = true;
    if(sqlite3_open(db_path, &db) != SQLITE_OK) CORE_FATAL_ERROR("failed to open db");
    if(init_database) db_exec_sql_file(db, "sql/db_init.sql");
    /*db_iterate_table(db, "customers");*/

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1000, 750, "db");

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    font = LoadFontFromNuklear(41);
    ctx = InitNuklearEx(font, 20);
    nk_set_style(ctx, THEME_DRACULA);

    while(!WindowShouldClose()) {
        UpdateNuklear(ctx);

        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 430, 650),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                     NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE
            )) {
            nk_layout_row_dynamic(ctx, 200, 1);
            db_customer_new(db, ctx);
        }
        nk_end(ctx);
        
        BeginDrawing();
        {
            ClearBackground(ColorFromNuklearF(bg));
            
            DrawNuklear(ctx);
        }
        EndDrawing();

        if(!fix_nuklear_sizing_bug) {
            int sw = GetScreenWidth(), sh = GetScreenHeight();
            SetWindowSize(sw-1, sh);    /* tiny change */
            SetWindowSize(sw, sh);      /* back to original */
            fix_nuklear_sizing_bug = true;
        }

    }

    UnloadNuklear(ctx);
    CloseWindow();
    sqlite3_close(db);
}
