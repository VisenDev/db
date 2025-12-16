/* stdlib */
#include <unistd.h>

/* 3rdparty */
#include "3rdparty/raylib/raylib.h"
#define CLAY_IMPLEMENTATION
#include "3rdparty/clay/clay.h"
#include "3rdparty/clay/renderers/raylib/clay_renderer_raylib.c"
/* #define RAYGUI_IMPLEMENTATION */
/* #include "3rdparty/raygui/src/raygui.h" */
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
    sqlite3 * db;
    core_Arena arena;
    db_MenuTab menu_tab;
    int gui_style_active;
    int gui_style_previous;
} db_State;

/*Local*/
/*#include "ui.c"*/
#include "sql.c"
/* #include "customer.c" */
#include "schema.c"

void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

#define FONT_ID_BODY_16 0

const Clay_Color COLOR_LIGHT = (Clay_Color) {244, 235, 230, 255};
const Clay_Color COLOR_LIGHT_HOVER = (Clay_Color) {224, 215, 210, 255};
const Clay_Color COLOR_RED = (Clay_Color) {168, 66, 28, 255};
const Clay_Color COLOR_RED_HOVER = (Clay_Color) {148, 46, 8, 255};
const Clay_Color COLOR_ORANGE = (Clay_Color) {225, 138, 50, 255};
const Clay_Color COLOR_BLUE = (Clay_Color) {111, 173, 162, 255};
Clay_TextElementConfig headerTextConfig = (Clay_TextElementConfig) { .fontId = 2, .fontSize = 24, .textColor = {61, 26, 5, 255} };

int main(void) {
    db_State s = {0};

    bool create_tables = core_file_exists(".main.db") ? false : true;
    if(sqlite3_open(".main.db", &s.db) != SQLITE_OK) CORE_FATAL_ERROR("Failed to open db");
    if(create_tables) {
        unsigned long i;
        for(i = 0; i < CORE_ARRAY_LEN(schemas); ++i) {
            sql_table_create(&s, schemas[i]);
        }
    }

    Clay_Raylib_Initialize(1024, 768, "db", FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT); // Extra parameters to this function are new since the video was published

    uint64_t clayRequiredMemory = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(clayRequiredMemory, malloc(clayRequiredMemory));
    Clay_Initialize(clayMemory, (Clay_Dimensions) {
            .width = GetScreenWidth(),
            .height = GetScreenHeight()
        }, (Clay_ErrorHandler) { HandleClayErrors, NULL }); // This final argument is new since the video was published
    Font fonts[1];
    fonts[0] = GetFontDefault();
    /* fonts[FONT_ID_BODY_16] = LoadFontEx("src/3rdparty/clay/examples/introducing-clay-video-demo/resources/Roboto-Regular.ttf", 16, 0, 400); */
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    /*Fixes a minor bug on macos*/

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)
    };

    Clay_TextElementConfig * txtConfig = CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .textColor = {200, 200, 200, 255},
            .fontSize = 17
                        
        });

    SetWindowSize(GetScreenWidth(), GetScreenHeight() - 10);
    SetWindowSize(GetScreenWidth(), GetScreenHeight() + 10);


    bool database_loaded = false;
    while(!WindowShouldClose()) {
        Vector2 mousePosition = GetMousePosition();
        Vector2 scrollDelta = GetMouseWheelMoveV();
        Clay_SetPointerState(
            (Clay_Vector2) { mousePosition.x, mousePosition.y },
            IsMouseButtonDown(0)
        );
        Clay_UpdateScrollContainers(
            true,
            (Clay_Vector2) { scrollDelta.x, scrollDelta.y },
            GetFrameTime()
        );

        Clay_BeginLayout();
        Clay_SetLayoutDimensions(
            (Clay_Dimensions) {
                .width = GetScreenWidth(),
                .height = GetScreenHeight()
            }
        );

        CLAY(CLAY_ID("OuterContainer"), {
                .backgroundColor = {43, 41, 51, 255 },
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = layoutExpand,
                    .padding = CLAY_PADDING_ALL(16),
                    .childGap = 16
                }
            }) {
            static int count = 0;
            if(true) {
                CLAY(
                    CLAY_ID("InitButton"), {
                        .backgroundColor = (Clay_Color){100, 50, 50, 40},
                    }
                ) {
                    CLAY_TEXT(Clay_Hovered() ? CLAY_STRING("Hovered") : CLAY_STRING("NotHovered"), txtConfig);
                }
            }
            char buf[1024];
            snprintf(buf, sizeof(buf), "Count: %d", count++);
            CLAY_TEXT(((Clay_String){.chars = buf, .length = strlen(buf), .isStaticallyAllocated = false}), txtConfig);
        }
                        
        

        /* db_update_style(&s); */
        /* ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR))); */
        
        /* GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER); */
        /* GuiToggleGroup((Rectangle){0, 0, GetScreenWidth() / 3, ROW_H}, "Home;Customers;Open Orders", (int*)&s.menu_tab); */

        switch(s.menu_tab) {
        case DB_MENU_TAB_HOME:
            /* menu_home(&s); */
            break;
        case DB_MENU_TAB_CUSTOMERS:
            //            ui_customer_display_row(s, (Rectangle){0, ROW_H + PAD, 512, ROW_H}, 1);
            /* menu_customers(&s, ROW_H); */
            break;
        case DB_MENU_TAB_OPEN_ORDERS:
            break;
        default:
            CORE_UNREACHABLE;
            break;
        }

        /* char fps_buf[1024]; */
        /* sqlite3_snprintf(sizeof(fps_buf), fps_buf, "%d FPS", GetFPS()); */
        /* GuiLabel((Rectangle){GetScreenWidth() - 55, GetScreenHeight() - 35, 50, 30}, fps_buf); */

        Clay_RenderCommandArray renderCommands = Clay_EndLayout();
            
        BeginDrawing();
        ClearBackground(BLACK);
        Clay_Raylib_Render(renderCommands, fonts);
        EndDrawing();
    }

    CloseWindow();
    sqlite3_close(s.db);
    core_arena_free(&s.arena);


    core_exit(0);
}
