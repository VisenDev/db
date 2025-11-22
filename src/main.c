/* stdlib */
#include <unistd.h>

/* 3rdparty */
#include <raylib/raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui/src/raygui.h>
#include <raygui/examples/styles/style_jungle.h>
#include <raygui/examples/styles/style_dark.h>
#include <sqlite/sqlite3.h>
#define CORE_IMPLEMENTATION
#include <core.h/core.h>
#define NK_INCLUDE_DEFAULT_FONT
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define RAYLIB_NUKLEAR_INCLUDE_DEFAULT_FONT
#include <raylib-nuklear/raylib-nuklear.h>
#include <nuklear/style.c>


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
        nk_style_select(s->ctx);
    } break;
    case DB_MENU_TAB_OPEN_ORDERS: {
        /* static db_Address addr = {0}; */
        /* nk_label(s->ctx, "<in development, this page will show open orders>", NK_TEXT_RIGHT); */
        /* db_ui_address(s, &addr); */
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
    db_State * s;

    *out = malloc(sizeof(db_State));
    assert(*out != NULL);
    s = *out;

    s->arena = malloc(sizeof(core_Arena));
    memset(s->arena, 0, sizeof(core_Arena));

    #if 0
    /* TODO: fix this on windows */
    if(!core_file_exists(db_path)) init_database = true;
    #endif
    if(sqlite3_open(db_path, &s->db) != SQLITE_OK) CORE_FATAL_ERROR("failed to open db");
    if(init_database) db_exec_sql_file(s->db, "sql/db_init.sql");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1000, 750, "db");

    /* font = LoadFontFromNuklear(-1); */
    /* font = GetFontDefault(); */
    //s->ctx = InitNuklearEx(font, RAYLIB_NUKLEAR_DEFAULT_FONTSIZE * 2);
    /*    nk_set_style(s->ctx, THEME_WHITE); */
}

void db_application_deinit(db_State ** state) {
    db_State * s = *state;
    CloseWindow();
    sqlite3_close(s->db);
    core_arena_free(s->arena);
    free(s);
    *state = NULL;
}


#define EDIT_STRING_CAP 1024

typedef struct {
    char buf[EDIT_STRING_CAP];
    int len;
    bool active;
} ui_EditString;

void ui_textbox(Rectangle bounds, ui_EditString * out) {
    if (GuiTextBox(bounds, out->buf, EDIT_STRING_CAP, out->active)) {
        out->active = !out->active;
    }
}

typedef struct {
    ui_EditString street;
    ui_EditString street_extra;
    ui_EditString city;
    ui_EditString state;
    ui_EditString postal;
    ui_EditString country;
} ui_Address;

#define ROW_H 25
#define PAD 10

int ui_address(Rectangle bounds, ui_Address * out) {
    assert(bounds.height == -1 && "The address widget has a fixed height");

    const int x = bounds.x;
    const int w = bounds.width;
    const int h = ROW_H;
    const int qw = w / 4; /*quarter width*/

    int y = bounds.y;

    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);

    GuiLabel((Rectangle){x, y, qw, h}, "Street:");
    ui_textbox((Rectangle){x + qw, y, 3 * qw, h}, &out->street);
    y += ROW_H + PAD;

    GuiLabel((Rectangle){x, y, qw, h}, "PO/Apt #:");
    ui_textbox((Rectangle){x + qw, y, qw, h}, &out->street_extra);
    GuiLabel((Rectangle){x + 2 * qw, y, qw, h}, "City:");
    ui_textbox((Rectangle){x + 3 * qw, y, qw, h}, &out->city);
    y += ROW_H + PAD;

    GuiLabel((Rectangle){x, y, qw, h}, "State:");
    ui_textbox((Rectangle){x + qw, y, qw, h}, &out->state);
    GuiLabel((Rectangle){x + 2 * qw, y, qw, h}, "Postal:");
    ui_textbox((Rectangle){x + 3 * qw, y, qw, h}, &out->postal);
    y += ROW_H + PAD;

    GuiLabel((Rectangle){x, y, qw, h}, "Country:");
    ui_textbox((Rectangle){x + qw, y, 3 * qw, h}, &out->country);
    y += ROW_H + PAD;

    return y;
}

int main(void) {
    db_State * s;
    db_application_init(&s);
    ui_Address address = {0};
    GuiLoadStyleDark();

    while(!WindowShouldClose()) {
    BeginDrawing();
    {
        const int width = 512;
        
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        GuiPanel((Rectangle){ 0, 0, width + PAD * 2, 210 }, "Address Information");
        
        int x = PAD;
        int y = PAD + ROW_H;
        y = ui_address((Rectangle) {x, y, width, -1}, &address);
        if(GuiButton((Rectangle){ x, y, width, ROW_H }, "Save")) {
            printf("y: %d\n", y);
        }
    }
    EndDrawing();
    }

    db_application_deinit(&s);
    core_exit(0);
}
