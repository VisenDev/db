/* stdlib */
#include <unistd.h>

/* 3rdparty */
#include <raylib/raylib.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui/src/raygui.h>
#include <raygui/examples/styles/style_amber.h>
#include <raygui/examples/styles/style_ashes.h>
#include <raygui/examples/styles/style_bluish.h>  
#include <raygui/examples/styles/style_candy.h>  
#include <raygui/examples/styles/style_cherry.h>
#include <raygui/examples/styles/style_cyber.h>
#include <raygui/examples/styles/style_dark.h>
#include <raygui/examples/styles/style_enefete.h> 
#include <raygui/examples/styles/style_jungle.h>  
#include <raygui/examples/styles/style_lavanda.h> 
#include <raygui/examples/styles/style_sunny.h>   
#include <raygui/examples/styles/style_terminal.h>

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
    //    GuiLoadStyleDark();
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
    int tab = 0;
    static const char * tabs[] = {
        "Home",
        "Customers",
        "Open Orders"
    };
    int next = 0;
    // Load default style
    GuiLoadStyleDefault();
    int visualStyleActive = 0;
    int prevVisualStyleActive = 0;


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


        if (visualStyleActive != prevVisualStyleActive)
            {
                // Reset to default internal style
                // NOTE: Required to unload any previously loaded font texture
                GuiLoadStyleDefault();

                switch (visualStyleActive)
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

                prevVisualStyleActive = visualStyleActive;
            }
        GuiComboBox((Rectangle){ x, y, width, ROW_H }, "default;Jungle;Candy;Lavanda;Cyber;Terminal;Ashes;Bluish;Dark;Cherry;Sunny;Enefete", &visualStyleActive);

    }
    EndDrawing();
    }

    db_application_deinit(&s);
    core_exit(0);
}
