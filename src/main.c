/* stdlib */
#include <unistd.h>

/* 3rdparty */
char *realpath(const char *restrict path,
                      char *restrict resolved_path);
#define  CORE_IMPLEMENTATION
#include "3rdparty/core.h/core.h"
#include "3rdparty/webui/include/webui.h"
#include "3rdparty/sqlite/sqlite3.h"

typedef struct {
    sqlite3 * db;
    core_Arena arena;
    size_t win;
} db_State;

/*Local*/
#include "sql.c"
/* #include "customer.c" */
#include "schema.c"

const char * db_get_database_path(db_State * s) {
    (void)s;
    return ".main.db";
}

void db_reset_database_file(db_State * s) {
    bool loaded = s->db;
    if(loaded) {
        sqlite3_close_v2(s->db);
    }
    remove(db_get_database_path(s));
    if(sqlite3_open(db_get_database_path(s), &s->db) != SQLITE_OK) 
        CORE_FATAL_ERROR("Failed to open db");
    unsigned long i;
    for(i = 0; i < CORE_ARRAY_LEN(schemas); ++i) {
        sql_table_create(s, schemas[i]);
    }
    if(!loaded)
        sqlite3_close_v2(s->db);

}

void say_hello(webui_event_t * e) {
    printf("Hello!\n");
}

const char * generate_ui(void) {
    return
        "<!DOCTYPE html>"
        "<html>"
        "    <head>"
        "        <script src=\"webui.js\"></script>"
        "    </head>"
        "    <body>"
        "        <div id=\"header\"> HI! </div>"
        "        <button onclick=\"say_hello()\">Hello</button>"
        "    </body>"
        "</html>"
        ;
}

int main(void) {
    db_State s = {0};

    db_reset_database_file(&s);
    if(sqlite3_open(db_get_database_path(&s), &s.db) != SQLITE_OK) CORE_FATAL_ERROR("Failed to open db");
    s.win = webui_new_window();

    webui_bind(s.win, "say_hello", say_hello);
    webui_show(s.win, generate_ui());
    webui_wait();

    sqlite3_close(s.db);
    core_arena_free(&s.arena);
    core_exit(0);
}
