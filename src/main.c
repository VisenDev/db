#include "3rdparty/sqlite/sqlite3.h"

#define CORE_IMPLEMENTATION
#include "3rdparty/core.h/core.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define OEMRESOURCE
#define NK_KEYSTATE_BASED_INPUT
#define NK_IMPLEMENTATION
#include "3rdparty/nuklear/nuklear.h"

#define RGFW_IMPLEMENTATION
#include "3rdparty/rgfw/RGFW.h"

#define NK_RGFW_GL2_IMPLEMENTATION
#include "3rdparty/rgfw-nuklear/nuklear_rgfw_gl2.h"

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

        if(nk_button_text(ctx, "Save\0\0   ", 4)) {
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
        
        nk_group_end(ctx);
    }
    return ret;
}

/* int main() { */
/*     sqlite3 * db = NULL; */
/*     int err; */
/*     if(access("main.db", F_OK) != 0) { */
/*         if(!database_create("main.db", &db)) CORE_FATAL_ERROR("Failed to create database"); */
/*     } else { */
/*         if((err = sqlite3_open("main.db", &db)) != SQLITE_OK) { */
/*             CORE_FATAL_ERROR(sqlite3_errstr(err)); */
/*         } */
/*     } */
/*     db_iterate_table(db, "customers"); */


/*     InitWindow(1000, 700, "db"); */
/*     SetWindowState(FLAG_WINDOW_RESIZABLE); */

/*     struct nk_context * ctx = NULL; */
/*     struct nk_colorf bg; */
/*     bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f; */
/*     Font font = LoadFontFromNuklear(20); */
/*     ctx = InitNuklearEx(font, 20); */


/*     while(!WindowShouldClose()) { */

/*         UpdateNuklear(ctx); */
/*         /\* GUI *\/ */
/*         if (nk_begin(ctx, "db", nk_rect(0, 0, GetScreenWidth(), GetScreenHeight()), NK_WINDOW_BORDER)) { */

            
/*             nk_menubar_begin(ctx); */
/*             { */
/*                 /\* toolbar *\/ */
/*                 nk_layout_row_static(ctx, 40, 100, 3); */
/*                 /\*if (nk_menu_begin_text(ctx, "Music", 50, NK_TEXT_RIGHT, nk_vec2(110,120)))*\/ */
/*                 /\*    {                                                                     *\/ */
/*                 /\*                    nk_layout_row_dynamic(ctx, 25, 1);                    *\/ */
/*                 /\*        nk_menu_item_text(ctx, "Play", 10, NK_TEXT_RIGHT);                *\/ */
/*                 /\*        nk_menu_item_text(ctx, "Stop", 10, NK_TEXT_RIGHT);                *\/ */
/*                 /\*        nk_menu_item_text(ctx, "Pause", 10, NK_TEXT_RIGHT);               *\/ */
/*                 /\*        nk_menu_item_text(ctx, "Next", 10, NK_TEXT_RIGHT);                *\/ */
/*                 /\*        nk_menu_item_text(ctx, "Prev", 10, NK_TEXT_RIGHT);                *\/ */
/*                 /\*        nk_menu_end(ctx);                                                 *\/ */
/*                 /\*    }                                                                     *\/ */
/*                 nk_button_text(ctx, "Home", 10); */
/*                 nk_button_text(ctx, "New", 10); */
/*                 nk_button_text(ctx, "Inventory", 10); */
/*             } */
/*             enum {EASY, HARD}; */
/*             static int op = EASY; */
/*             static int property = 20; */

/*             nk_layout_row_static(ctx, 30, 80, 1); */
/*             if (nk_button_label(ctx, "button")) */
/*                 TraceLog(LOG_INFO, "button pressed!"); */
/*             nk_layout_row_dynamic(ctx, 30, 2); */
/*             if (nk_option_label(ctx, "easy", op == EASY)) op = EASY; */
/*             if (nk_option_label(ctx, "hard", op == HARD)) op = HARD; */
/*             nk_layout_row_dynamic(ctx, 22, 1); */
/*             nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1); */

/*             nk_layout_row_dynamic(ctx, 200, 1); */
/*             db_customer_new(db, ctx); */



/*             static int value = 0; */
/*             static char buffer[32]; */
/*             snprintf(buffer, sizeof(buffer), "%d", value); */

/*             nk_layout_row_dynamic(ctx, 30, 1); */
/*             if (nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, buffer, sizeof(buffer), nk_filter_decimal)) { */
/*                 value = atoi(buffer); */
/*             } */

/*             nk_label(ctx, buffer, NK_TEXT_LEFT); */

/*             nk_layout_row_dynamic(ctx, 20, 1); */
/*             nk_label(ctx, "background:", NK_TEXT_LEFT); */
/*             nk_layout_row_dynamic(ctx, 25, 1); */
/*             if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) { */
/*                 nk_layout_row_dynamic(ctx, 120, 1); */
/*                 bg = nk_color_picker(ctx, bg, NK_RGBA); */
/*                 nk_layout_row_dynamic(ctx, 25, 1); */
/*                 bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f); */
/*                 bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f); */
/*                 bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f); */
/*                 bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f); */
/*                 nk_combo_end(ctx); */
/*             } */


/*         } */
/*         nk_end(ctx); */

        
/*         BeginDrawing(); */
/*         { */
/*             ClearBackground(ColorFromNuklearF(bg)); */
/*             DrawNuklear(ctx); */
/*         } */
/*         EndDrawing(); */

/*     } */
    
/*     CloseWindow(); */
/*     sqlite3_close(db); */
/* } */


int main() {
    /* GUI */
    struct nk_context *ctx;
    struct nk_colorf bg;
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


    /* RGFW */
    RGFW_window* win = RGFW_createWindow("RGFW Demo", RGFW_RECT(0, 0, 1000, 800), RGFW_windowCenter);
    
    ctx = nk_RGFW_init(win, NK_RGFW_INSTALL_CALLBACKS);

    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {struct nk_font_atlas *atlas;
    nk_RGFW_font_stash_begin(&atlas);
    /*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
    /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
    nk_RGFW_font_stash_end();
    /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    /*nk_style_set_font(ctx, &droid->handle);*/}

    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    char buf[256] = {0};
    (void)buf;

    while (!RGFW_window_shouldClose(win))
    {
        /* Input */
        RGFW_window_checkEvents(win, RGFW_eventNoWait);
        nk_RGFW_new_frame();

        /* GUI */

        /*BEGIN END*/
        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                     NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(ctx, 200, 1);
            db_customer_new(db, ctx);
            nk_end(ctx);
        }

        /* Draw */
        glViewport(0, 0, win->r.w, win->r.h);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bg.r, bg.g, bg.b, bg.a);
        /* IMPORTANT: `nk_RGFW_render` modifies some global OpenGL state
         * with blending, scissor, face culling and depth test and defaults everything
         * back into a default state. Make sure to either save and restore or
         * reset your own state after drawing rendering the UI. */
        nk_RGFW_render(NK_ANTI_ALIASING_ON);
        RGFW_window_swapBuffers(win);
    }
    nk_RGFW_shutdown();
    RGFW_window_close(win);
    return 0;
}
