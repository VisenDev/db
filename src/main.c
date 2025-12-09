/* stdlib */
#include <unistd.h>

/* 3rdparty */
#include "3rdparty/raylib/raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "3rdparty/raygui/src/raygui.h"
#include "3rdparty/sqlite/sqlite3.h"
#define  CORE_IMPLEMENTATION
#include "3rdparty/core.h/core.h"


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

/*Local*/
#include "ui.c"
#include "sql.c"
#include "customer.c"


void db_application_init(db_State ** out) {
    bool init_database = false;
    const char * db_path = ".main.db";
    db_State * s;

    *out = malloc(sizeof(db_State));
    assert(*out != NULL);
    s = *out;
    memset(s, 0, sizeof(db_State));

    s->arena = malloc(sizeof(core_Arena));
    memset(s->arena, 0, sizeof(core_Arena));

    if(!core_file_exists(db_path)) init_database = true;
    if(sqlite3_open(db_path, &s->db) != SQLITE_OK) CORE_FATAL_ERROR("failed to open db");
    if(init_database)
        if(!db_exec_sql_file(s->db, "src/sql/db_init.sql")) CORE_FATAL_ERROR("Failed to init db");
    if(sqlite3_exec(s->db, "insert into customers(name) values('vintage');", NULL, NULL, NULL) != SQLITE_OK) CORE_FATAL_ERROR("fail");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE /*| FLAG_VSYNC_HINT*/);
    SetTargetFPS(170);
    InitWindow(1000, 750, "db");
    s->gui_style_active = 0;

    SetWindowSize(GetScreenWidth(), GetScreenHeight() - 1);
    SetWindowSize(GetScreenWidth(), GetScreenHeight() + 1);
}

void db_application_begin_drawing(db_State * s) {
    if(IsWindowResized()) {}
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

int main(void) {
    db_State * s;

    sql_Values data = {0};
    db_application_init(&s);
    core_hashmap_set(&data, s->arena, "id", ((sql_Value){.tag = SQLITE_INTEGER, .as.integer = 1000}));
    core_hashmap_set(&data, s->arena, "name", ((sql_Value){.tag = SQLITE_TEXT, .as.text = "hashmap_test"}));
    sql_table_insert(s, tbl, data);


    while(!WindowShouldClose()) {
        db_application_begin_drawing(s);
        
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        GuiToggleGroup((Rectangle){0, 0, GetScreenWidth() / 3, ROW_H}, "Home;Customers;Open Orders", (int*)&s->menu_tab);

        switch(s->menu_tab) {
        case DB_MENU_TAB_HOME:
            menu_home(s);
            break;
        case DB_MENU_TAB_CUSTOMERS:
            //            ui_customer_display_row(s, (Rectangle){0, ROW_H + PAD, 512, ROW_H}, 1);
            menu_customers(s, ROW_H);
            break;
        case DB_MENU_TAB_OPEN_ORDERS:
            break;
        case DB_MENU_TAB_COUNT:
            CORE_UNREACHABLE;
        }

        char fps_buf[1024];
        sqlite3_snprintf(sizeof(fps_buf), fps_buf, "%d FPS", GetFPS());
        GuiLabel((Rectangle){GetScreenWidth() - 55, GetScreenHeight() - 35, 50, 30}, fps_buf);

        db_application_end_drawing(s);
    }

    db_application_deinit(&s);
    core_exit(0);
}
