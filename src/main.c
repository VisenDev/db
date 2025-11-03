#include <wayland-client.h>
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
            const unsigned long n = 20000;
            char * buf = malloc(n);
            FILE * fp = fopen(database_create_script_path, "r");
            assert(fp);
            
            if(!core_file_read_all(fp, buf, n)) {
                fprintf(stderr, "Failed to read whole file\n");
                free(buf);
                fclose(fp);
                sqlite3_close(*result);
                return CORE_FALSE;
            } else {
                char * exec_err = NULL;
                printf("%s", buf);
                if(sqlite3_exec(*result, buf, NULL, NULL, &exec_err) != 0) {
                    fprintf(stderr, "sql execution failed: %s\n", exec_err);
                    fclose(fp);
                    free(buf);
                    sqlite3_close(*result);
                    return CORE_FALSE;
                }
                fclose(fp);
                free(buf);
                return CORE_TRUE;
            }
        }
    }
}

int main() {
    sqlite3 * db = NULL;
    if(!database_create("main.db", &db)) CORE_FATAL_ERROR("Failed to create database");
    sqlite3_close(db);
    exit(0);

    InitWindow(230, 250, "hello");
    struct nk_context * ctx = NULL;
    struct nk_colorf bg;
    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    Font font = LoadFontFromNuklear(20);
    ctx = InitNuklearEx(font, 20);


    while(!WindowShouldClose()) {

        UpdateNuklear(ctx);
        /* GUI */
        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE| NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button"))
                TraceLog(LOG_INFO, "button pressed!");
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(ctx, 22, 1);
            nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }
        }
        nk_end(ctx);

        
        BeginDrawing();
        {
            ClearBackground(ColorFromNuklearF(bg));
            DrawNuklear(ctx);
        }
        EndDrawing();

    }
    
    CloseWindow();
}
