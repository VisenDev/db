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
    fonts[FONT_ID_BODY_16] = LoadFontEx("src/3rdparty/clay/examples/introducing-clay-video-demo/resources/Roboto-Regular.ttf", 48, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    /*Fixes a minor bug on macos*/
    SetWindowSize(GetScreenWidth(), GetScreenHeight() - 1);
    SetWindowSize(GetScreenWidth(), GetScreenHeight() + 1);

    Clay_Sizing layoutExpand = {
        .width = CLAY_SIZING_GROW(0),
        .height = CLAY_SIZING_GROW(0)
    };

    Clay_TextElementConfig * txtConfig = CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .textColor = {200, 200, 200, 255},
            .fontSize = 16
                        
        });


    bool database_loaded = false;
    while(!WindowShouldClose()) {
        Clay_BeginLayout();
        Clay_SetLayoutDimensions(
            (Clay_Dimensions) {
                .width = GetScreenWidth(),
                .height = GetScreenHeight()
            }
        );

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
                    .backgroundColor = Clay_Hovered() ?
                    (Clay_Color){100, 50, 50, 40} : (Clay_Color){0}
                    }
                ) {
                    if(Clay_PointerOver(CLAY_ID("InitButton"))) {
                        CLAY_TEXT(CLAY_STRING("Hovered"), txtConfig);
                    } else {
                        CLAY_TEXT(CLAY_STRING("NotHovered"), txtConfig);
                    }
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
