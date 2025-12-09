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
#include "schema.c"


db_State * db_application_init(void) {
    bool init_database = false;
    const char * schema_path[] = {"schema.c"};
    const char * db_path = ".main.db";
    db_State * s = calloc(1, sizeof(*s));
    assert(s);

    s->arena = calloc(1, sizeof(s->arena));

    init_database = core_file_needs_update(db_path, schema_path, CORE_ARRAY_LEN(schema_path));
    if(init_database) {
        unsigned long i;
        for(i = 0; i < CORE_ARRAY_LEN(schemas); ++i) {
            sql_table_create();
        }
    }
        if(sqlite3_open(db_path, &s->db) != SQLITE_OK) {
            CORE_FATAL_ERROR("failed to open db");
        }
    /* if(sqlite3_exec(s->db, "insert into customers(name) values('vintage');", NULL, NULL, NULL) != SQLITE_OK) CORE_FATAL_ERROR("fail"); */

}


void db_update_style(db_State * s) {
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
    db_State s = {0};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE /*| FLAG_VSYNC_HINT*/);
    SetTargetFPS(170);
    InitWindow(1000, 750, "db");
    s.gui_style_active = 0;

    SetWindowSize(GetScreenWidth(), GetScreenHeight() - 1);
    SetWindowSize(GetScreenWidth(), GetScreenHeight() + 1);


    while(!WindowShouldClose()) {
        BeginDrawing();
        db_update_style(&s);
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        
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
        
        EndDrawing();
    }

    CloseWindow();
    sqlite3_close(s->db);
    core_arena_free(s->arena);


    core_exit(0);
}
