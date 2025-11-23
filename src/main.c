/* stdlib */
#include <unistd.h>

/* 3rdparty */
#include "3rdparty/raylib/raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "3rdparty/raygui/src/raygui.h"
#include "3rdparty/sqlite/sqlite3.h"
#define  CORE_IMPLEMENTATION
#include "3rdparty/core.h/core.h"

/*Local*/
#include "ui.c"

typedef enum {
    DB_MENU_TAB_HOME = 0,
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
    int gui_style_active;
    int gui_style_previous;
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
    if(sqlite3_exec(db, buf, NULL, NULL, &exec_err) != SQLITE_OK) success = CORE_FALSE;
    core_arena_free(&arena);
    return success;
}

void db_application_init(db_State ** out) {
    bool init_database = true;
    const char * db_path = ".main.db";
    db_State * s;

    *out = malloc(sizeof(db_State));
    assert(*out != NULL);
    s = *out;
    memset(s, 0, sizeof(db_State));

    s->arena = malloc(sizeof(core_Arena));
    memset(s->arena, 0, sizeof(core_Arena));

    #if 0
    /* TODO: fix this on windows */
    if(!core_file_exists(db_path)) init_database = true;
    #endif
    if(sqlite3_open(db_path, &s->db) != SQLITE_OK) CORE_FATAL_ERROR("failed to open db");
    if(init_database)
        if(!db_exec_sql_file(s->db, "src/sql/db_init.sql")) CORE_FATAL_ERROR("Failed to init db");
    if(sqlite3_exec(s->db, "insert into customers(name) values('vintage');", NULL, NULL, NULL) != SQLITE_OK) CORE_FATAL_ERROR("fail");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1000, 750, "db");
    s->gui_style_active = 3;
}

void db_application_begin_drawing(db_State * s) {
    BeginDrawing();
    if (s->gui_style_active != s->gui_style_previous) {
        // Reset to default internal style
        // NOTE: Required to unload any previously loaded font texture
        GuiLoadStyleDefault();

        switch (s->gui_style_active) {
        case 1: GuiLoadStyleJungle(); break;
        case 2: GuiLoadStyleCandy(); break;
        case 3: GuiLoadStyleLavanda(); break;
        case 4: GuiLoadStyleCyber(); break;
        case 5: GuiLoadStyleTerminal(); break;
        case 6: GuiLoadStyleAshes(); break;
        case 7: GuiLoadStyleBluish(); break;
        case 8: GuiLoadStyleDark(); break;
        case 9: GuiLoadStyleCherry(); break;
        case 10: GuiLoadStyleSunny(); break;
        case 11: GuiLoadStyleEnefete(); break;
        default: break;
        }
        s->gui_style_previous = s->gui_style_active;
    }
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
}

void db_application_end_drawing(db_State * s) {
    (void)s;
    EndDrawing();
}

void db_application_deinit(db_State ** state) {
    db_State * s = *state;
    CloseWindow();
    sqlite3_close(s->db);
    core_arena_free(s->arena);
    free(s);
    *state = NULL;
}

void menu_home(db_State * s) {
    const int width = 512;
    int x = PAD;
    int y = PAD + PAD + ROW_H;
    static ui_Address address = {0};
    y = ui_address((Rectangle) {x, y, width, -1}, &address);
    if(GuiButton((Rectangle){ x, y, width, ROW_H }, "Save")) {
        printf("y: %d\n", y);
    }
    y += PAD + ROW_H;

    GuiComboBox((Rectangle){ x, y, width, ROW_H }, "default;Jungle;Candy;Lavanda;Cyber;Terminal;Ashes;Bluish;Dark;Cherry;Sunny;Enefete", &s->gui_style_active);
}

#include "customer.c"

int main(void) {
    db_State * s;
    db_application_init(&s);

    while(!WindowShouldClose()) {
        db_application_begin_drawing(s);
        
        GuiToggleGroup((Rectangle){0, 0, GetScreenWidth() / 3, ROW_H}, "Home;Customers;Open Orders", (int*)&s->menu_tab);

        switch(s->menu_tab) {
        case DB_MENU_TAB_HOME:
            menu_home(s);
            break;
        case DB_MENU_TAB_CUSTOMERS:
            ui_customer_display_row(s, (Rectangle){PAD, 400, 512, ROW_H}, 1);
            break;
        case DB_MENU_TAB_OPEN_ORDERS:
            break;
        case DB_MENU_TAB_COUNT:
            CORE_UNREACHABLE;
        }
        db_application_end_drawing(s);
    }

    db_application_deinit(&s);
    core_exit(0);
}
