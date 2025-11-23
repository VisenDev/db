/* stdlib */
#include <unistd.h>

/* 3rdparty */
#include "3rdparty/raylib/raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "3rdparty/raygui/src/raygui.h"
#include "3rdparty/raygui/examples/styles/style_amber.h"
#include "3rdparty/raygui/examples/styles/style_ashes.h"
#include "3rdparty/raygui/examples/styles/style_bluish.h"  
#include "3rdparty/raygui/examples/styles/style_candy.h"  
#include "3rdparty/raygui/examples/styles/style_cherry.h"
#include "3rdparty/raygui/examples/styles/style_cyber.h"
#include "3rdparty/raygui/examples/styles/style_dark.h"
#include "3rdparty/raygui/examples/styles/style_enefete.h" 
#include "3rdparty/raygui/examples/styles/style_jungle.h"  
#include "3rdparty/raygui/examples/styles/style_lavanda.h" 
#include "3rdparty/raygui/examples/styles/style_sunny.h"   
#include "3rdparty/raygui/examples/styles/style_terminal.h"
#include "3rdparty/sqlite/sqlite3.h"
#define  CORE_IMPLEMENTATION
#include "3rdparty/core.h/core.h"

/*Local*/
#include "ui.c"

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
    if(sqlite3_exec(db, buf, NULL, NULL, &exec_err) != 0) success = CORE_FALSE;
    core_arena_free(&arena);
    return success;
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
    s->gui_style_active = 3;
}

void db_application_deinit(db_State ** state) {
    db_State * s = *state;
    CloseWindow();
    sqlite3_close(s->db);
    core_arena_free(s->arena);
    free(s);
    *state = NULL;
}


int main(void) {
    db_State * s;
    db_application_init(&s);

    ui_Address address = {0};
    int tab = 0;
    static const char * tabs[] = {
        "Home",
        "Customers",
        "Open Orders"
    };
    int next = 0;
    // Load default style


    while(!WindowShouldClose()) {
    BeginDrawing();
    {
        const int width = 512;
        int x = PAD;
        int y = 0;

        
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        //GuiTabBar((Rectangle){x, y, width, ROW_H}, tabs, CORE_ARRAY_LEN(tabs), &next);
        //GuiPanel((Rectangle){ 0, 0, width + PAD * 2, 210 }, "Address Information");
        GuiToggleGroup((Rectangle){x, y, width / 3, ROW_H}, "Home;Customers;Open Orders", &next);
        y += PAD + ROW_H;
        
        y = ui_address((Rectangle) {x, y, width, -1}, &address);
        if(GuiButton((Rectangle){ x, y, width, ROW_H }, "Save")) {
            printf("y: %d\n", y);
        }
        y += PAD + ROW_H;


        if (s->gui_style_active != s->gui_style_previous)
            {
                // Reset to default internal style
                // NOTE: Required to unload any previously loaded font texture
                GuiLoadStyleDefault();

                switch (s->gui_style_active)
                    {
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
        GuiComboBox((Rectangle){ x, y, width, ROW_H }, "default;Jungle;Candy;Lavanda;Cyber;Terminal;Ashes;Bluish;Dark;Cherry;Sunny;Enefete", &s->gui_style_active);

    }
    EndDrawing();
    }

    db_application_deinit(&s);
    core_exit(0);
}
