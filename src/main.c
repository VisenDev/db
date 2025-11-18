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
    DB_MENU_TAB_HOME,
    DB_MENU_TAB_CUSTOMERS,
    DB_MENU_TAB_OPEN_ORDERS,
    DB_MENU_TAB_COUNT
} db_MenuTab;

typedef enum {
    DB_STATUS_PENDING,
    DB_STATUS_DONE
} db_Status;

typedef struct {
    core_Arena * arena;
    sqlite3 * db;
    db_MenuTab menu_tab;
    struct nk_context * ctx;
} db_State;


db_Status db_customer_new(db_State * s) {

    static char name[256];
    static int name_len = 0;
    static char email[256];
    static int email_len = 0;
    static char city[256];
    static int city_len = 0;
    db_Status ret = DB_STATUS_DONE;
    if (nk_group_begin_titled(s->ctx, "Add New Customer", "Add New Customer", NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(s->ctx, 30, 2);
    
        nk_label(s->ctx, "Customer Name: ", NK_TEXT_RIGHT);
        nk_edit_string(s->ctx, NK_EDIT_FIELD, name, &name_len, sizeof(name), NULL);

        nk_label(s->ctx, "Customer Email: ", NK_TEXT_RIGHT);
        nk_edit_string(s->ctx, NK_EDIT_FIELD, email, &email_len, sizeof(email), NULL);

        nk_label(s->ctx, "Customer City: ", NK_TEXT_RIGHT);
        nk_edit_string(s->ctx, NK_EDIT_FIELD, city, &city_len, sizeof(city), NULL);

        if(nk_button_label(s->ctx, "Save")) {
            int err;
            sqlite3_stmt * stmt;
            const char sql[] = "insert into customers(name, email, city) values(?, ?, ?);";
            err = sqlite3_prepare_v2(s->db, sql, -1, &stmt, NULL);
            if(err != SQLITE_OK) {
                fprintf(stderr, "\n%s\n", sqlite3_errstr(err));
                fprintf(stderr, "\n%s\n", sqlite3_errmsg(s->db));
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
    nk_group_end(s->ctx);
    return ret;
}

void db_menu_tab_button(db_State * s, const char * label, db_MenuTab menu_tab) {
    struct nk_rect bounds = nk_widget_bounds(s->ctx);
    const struct nk_input *in = &s->ctx->input;
    char buf[1024];
    db_MenuTab active = s->menu_tab;
    if (nk_input_is_mouse_hovering_rect(in, bounds)) {
        if(menu_tab == active) {
            sqlite3_snprintf(sizeof(buf), buf, "Currently working on \"%s\"", label);
        } else {
            sqlite3_snprintf(sizeof(buf), buf, "Click to swap to \"%s\"", label);
        }
        nk_tooltip(s->ctx, buf);
    }

    if(menu_tab == active) {
        nk_style_push_style_item(s->ctx, &s->ctx->style.button.normal, s->ctx->style.window.fixed_background);
        nk_style_push_color(s->ctx, &s->ctx->style.button.border_color, s->ctx->style.window.background);
        nk_style_push_style_item(s->ctx, &s->ctx->style.button.hover, s->ctx->style.window.fixed_background);
        nk_style_push_style_item(s->ctx, &s->ctx->style.button.active, s->ctx->style.window.fixed_background);
    }

    if(nk_button_label(s->ctx, label)) s->menu_tab = menu_tab;

    if(menu_tab == active) {
        nk_style_pop_style_item(s->ctx);
        nk_style_pop_color(s->ctx);
        nk_style_pop_style_item(s->ctx);
        nk_style_pop_style_item(s->ctx);
    }
}


void db_application_run(db_State *s) {

    /*tabs*/
    if(nk_begin(s->ctx, "db", nk_rect(0, 0, GetScreenWidth(), GetScreenHeight()), NK_WINDOW_BORDER)) {
        nk_layout_row_static(s->ctx, 30, 200, DB_MENU_TAB_COUNT);
        db_menu_tab_button(s, "Home", DB_MENU_TAB_HOME);
        db_menu_tab_button(s, "Customers", DB_MENU_TAB_CUSTOMERS);
        db_menu_tab_button(s, "Open Orders", DB_MENU_TAB_OPEN_ORDERS);
    }

    switch(s->menu_tab) {
    case DB_MENU_TAB_HOME: {
        nk_label(s->ctx, "Welcome to db!", NK_TEXT_RIGHT);
    } break;
    case DB_MENU_TAB_OPEN_ORDERS: {
        nk_label(s->ctx, "<in development, this page will show open orders>", NK_TEXT_RIGHT);
    } break;
    case DB_MENU_TAB_CUSTOMERS: {
        nk_layout_row_template_begin(s->ctx, 500);
        nk_layout_row_template_push_variable(s->ctx, 80);
        nk_layout_row_template_push_static(s->ctx, 400);
        nk_layout_row_template_end(s->ctx);

        nk_group_begin(s->ctx, "label", 0);
        nk_layout_row_dynamic(s->ctx, 20, 1);
        nk_label(s->ctx, "<in development, this page will show open customers>", NK_TEXT_RIGHT);
        nk_label(s->ctx, "<in development, this page will show open customers>", NK_TEXT_RIGHT);
        nk_label(s->ctx, "<in development, this page will show open customers>", NK_TEXT_RIGHT);
        nk_label(s->ctx, "<in development, this page will show open customers>", NK_TEXT_RIGHT);
        nk_group_end(s->ctx);

        db_customer_new(s);
    } break;
    case DB_MENU_TAB_COUNT:
        CORE_UNREACHABLE;
    }
    nk_end(s->ctx);
}

void db_application_init(db_State ** out) {
    bool init_database = false;
    const char * db_path = ".main.db";
    Font font;
    db_State * s;

    *out = malloc(sizeof(db_State));
    assert(*out != NULL);
    s = *out;

    s->arena = malloc(sizeof(core_Arena));
    memset(s->arena, 0, sizeof(core_Arena));

    if(!core_file_exists(db_path)) init_database = true;
    if(sqlite3_open(db_path, &s->db) != SQLITE_OK) CORE_FATAL_ERROR("failed to open db");
    if(init_database) db_exec_sql_file(s->db, "sql/db_init.sql");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1000, 750, "db");

    font = LoadFontFromNuklear(41);
    s->ctx = InitNuklearEx(font, 20);
    nk_set_style(s->ctx, THEME_DRACULA);
}

void db_application_deinit(db_State ** state) {
    db_State * s = *state;
    UnloadNuklear(s->ctx);
    CloseWindow();
    sqlite3_close(s->db);
    core_arena_free(s->arena);
    free(s);
    *state = NULL;
}

void db_application_frame_begin(db_State * s) {
    UpdateNuklear(s->ctx);
}

void db_application_frame_end(db_State * s) {
    static core_Bool fix_nuklear_sizing_bug = CORE_TRUE; /*For some reason nuklear doesn't work on mac until the window is resized*/
    BeginDrawing();
    {
        ClearBackground(ColorFromNuklear(s->ctx->style.window.background));
        DrawNuklear(s->ctx);
    }
    EndDrawing();

    if(fix_nuklear_sizing_bug) {
        int sw = GetScreenWidth(), sh = GetScreenHeight();
        SetWindowSize(sw-1, sh);    /* tiny change */
        SetWindowSize(sw, sh);      /* back to original */
        fix_nuklear_sizing_bug = CORE_FALSE;
    }
}

core_Bool db_application_should_close(db_State * s) {
    (void)s;
    return WindowShouldClose();
}



int main() {
    db_State * s;
    db_application_init(&s);
 
    while(!db_application_should_close(s)) {
        db_application_frame_begin(s);
        db_application_run(s);
        db_application_frame_end(s);
    }

    db_application_deinit(&s);
    core_exit(0);
}
