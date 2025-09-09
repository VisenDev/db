#include "./sqlite/sqlite3.h"

#define CORE_IMPLEMENTATION
#include "core.h"

void create_new_database(const char * filepath) {
    sqlite3 * db = NULL;
    if(sqlite3_open(filepath, &db) != 0) CORE_FATAL_ERROR("failed to create new db");
}


int main() {
    create_new_database("main.db");
}
