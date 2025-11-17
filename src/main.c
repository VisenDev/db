#include <unistd.h>

/* 3rdparty */
#include <raylib/raylib.h>
#include <sqlite/sqlite3.h>
#define CORE_IMPLEMENTATION
#include <core.h/core.h>
//#define NK_INCLUDE_FIXED_TYPES
//#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
//#define NK_INCLUDE_DEFAULT_ALLOCATOR
//#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
//#define NK_INCLUDE_FONT_BAKING
//#define NK_INCLUDE_DEFAULT_FONT
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define RAYLIB_NUKLEAR_INCLUDE_DEFAULT_FONT
#include <raylib-nuklear/raylib-nuklear.h>

#define database_create_script_path "./database_create.sql"

core_Bool database_create(const char * path, sqlite3 ** result) {
    if(access(path, F_OK) == 0) {
        fprintf(stderr, "File already exists\n");
        return CORE_FALSE;
    } else {
        int err = sqlite3_open(path, result);
        if(err != 0) {
            fprintf(stderr, "Failed to open db\n");
            sqlite3_close(*result);
            return CORE_FALSE;
        } else {
            core_Arena arena = {0};
            FILE * fp = fopen(database_create_script_path, "r");
            char * buf = core_file_read_all_arena(&arena, fp);
            fclose(fp);
            char * exec_err = NULL;
            printf("%s", buf);
            if(sqlite3_exec(*result, buf, NULL, NULL, &exec_err) != 0) {
                fprintf(stderr, "sql execution failed: %s\n", exec_err);
                core_arena_free(&arena);
                sqlite3_close(*result);
                return CORE_FALSE;
            }
            core_arena_free(&arena);
            return CORE_TRUE;
        }
    }
}

typedef enum {
    MENU_TAG_NEW_ITEM,
    MENU_TAG_VIEW_TABLES
} MenuTag;

void db_iterate_table(sqlite3 * db, const char * table_name) {
    sqlite3_stmt * stmt;
    char sql[256];
    const char * unused;
    int code;
    sqlite3_snprintf(sizeof(sql), sql, "pragma table_info(%s);", table_name);
    
    if((code = sqlite3_prepare_v2(db, sql, -1, &stmt, &unused)) != SQLITE_OK) {
        fprintf(stderr, "\n%s\n", sqlite3_errstr(code));
        CORE_FATAL_ERROR("Failed to prepare statement");
    }

    while((code = sqlite3_step(stmt)) == SQLITE_ROW) {
        int col;
        int count = sqlite3_column_count(stmt);
        for(col = 0; col < count; ++col) {
            int type = sqlite3_column_type(stmt, col);
            switch(type) {
            case SQLITE_INTEGER:
                printf("%d,", sqlite3_column_int(stmt, col));
                break;
            case SQLITE_FLOAT:
                printf("%lf,", sqlite3_column_double(stmt, col));
                break;
            case SQLITE_TEXT:
                printf("%s,", sqlite3_column_text(stmt, col));
                break;
            case SQLITE_BLOB: {
                int bytes = sqlite3_column_bytes(stmt, col);
                (void)bytes;
                printf("%p,", sqlite3_column_blob(stmt, col));
            } break;
            case SQLITE_NULL:
                printf("NULL,");
                break;
            }
        }
        printf("\n");
    }
    if(code != SQLITE_DONE) {
        CORE_FATAL_ERROR("step returned error");
    }
    sqlite3_finalize(stmt);
}

sqlite3_stmt * db_table_info(sqlite3 * db, const char * table_name) {
    sqlite3_stmt * stmt;
    char sql[256];
    const char * unused;
    int code;
    sqlite3_snprintf(sizeof(sql), sql, "pragma table_info(%s);", table_name);
    
    if((code = sqlite3_prepare_v2(db, sql, -1, &stmt, &unused)) != SQLITE_OK) {
        fprintf(stderr, "\n%s\n", sqlite3_errstr(code));
        return NULL;
    }
    return stmt;
}

/*
typedef struct {
    const char * name;
    int type;
} db_TableMetadataEntry;

typedef core_Vec(db_TableMetadataEntry) db_TableMetadata;

core_Bool db_table_metadata(sqlite3 * db, core_Arena * a, const char * table_name, db_TableMetadata * out) {
    sqlite3_stmt * stmt;
    char sql[256];
    const char * unused;
    int code;
    db_TableMetadata meta = {0};
    (void)meta;
    sqlite3_snprintf(sizeof(sql), sql, "pragma table_info(%s);", table_name);
    
    if((code = sqlite3_prepare_v2(db, sql, -1, &stmt, &unused)) != SQLITE_OK) {
        fprintf(stderr, "\n%s\n", sqlite3_errstr(code));
        return CORE_FALSE;
    }

    while((code = sqlite3_step(stmt)) == SQLITE_ROW) {
        db_TableMetadataEntry entry = {0};
        char * typestr = sqlite3_column_str(stmt, 2);
        if(strcmp("TEXT", typestr) == 0) {
            typestr = SQLITE_TEXT;
        }
        entry.name
    }
}
*/

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



int main() {
    sqlite3 * db = NULL;
    int err;
    if(access("main.db", F_OK) != 0) {
        if(!database_create("main.db", &db)) CORE_FATAL_ERROR("Failed to create database");
    } else {
        if((err = sqlite3_open("main.db", &db)) != SQLITE_OK) {
            CORE_FATAL_ERROR(sqlite3_errstr(err));
        }
    }
    db_iterate_table(db, "customers");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1000, 750, "db");


    struct nk_context * ctx = NULL;
    struct nk_colorf bg;
    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    Font font = LoadFontFromNuklear(20);
    ctx = InitNuklearEx(font, 20);

    bool fix_nuklear_sizing_bug = false;


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
    
    CloseWindow();
    sqlite3_close(db);
}
