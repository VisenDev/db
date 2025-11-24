/**** THIS FILE CONTAINS SQL UTILITIES ****/

#define DB_FATAL_SQL_ERROR(state, sql_code) do {                            \
    if(sql_code != NULL) fprintf(stderr, "Offending sql code: \"%s\"\n", sql_code); \
    CORE_FATAL_ERROR(sqlite3_errmsg((state)->db));                          \
} while(0)


__attribute__((format (printf, 3, 4)))
sqlite3_stmt * db_prepare_and_bind(db_State * s, const char * sql, const char * fmt, ...) {
    
    sqlite3_stmt * result = NULL;
    const char * unused_bytes;
    unsigned long i = 0;
    const unsigned long len = strlen(sql);
    if(sqlite3_prepare_v2(s->db, sql, len, &result, &unused_bytes) != SQLITE_OK) {
        DB_FATAL_SQL_ERROR(s, sql);
    }

    core_Bool expect_fmt_char = false;
    int binding_index = 1;
    va_list args;
    va_start(args, fmt);
    for(i = 0; i < len; ++i) {
        if(expect_fmt_char) {
            switch(fmt[i]) {
            case 'd': if(sqlite3_bind_int(result, binding_index++, va_arg(args, int)) != SQLITE_OK) goto err; break;
            case 's': if(sqlite3_bind_text(result, binding_index++, va_arg(args, char *), -1, NULL) != SQLITE_OK) goto err; break;
            case 'l': if(sqlite3_bind_int64(result, binding_index++, va_arg(args, sqlite_int64)) != SQLITE_OK) goto err; break;
            default: 
                CORE_FATAL_ERROR("Unrecognized fmt character");
            }
            expect_fmt_char = CORE_FALSE;
        } else {
            if(fmt[i] == '%') expect_fmt_char = CORE_TRUE;
        }
    }
    assert(expect_fmt_char == CORE_FALSE && "Stray % in fmt string");

    if(0) {
        err:
        sqlite3_finalize(result);
        result = NULL;
    }
    va_end(args);
    return result;
}

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
    //    printf("exec error: %s\n", exec_err);
    core_arena_free(&arena);
    return success;
}

sqlite3_stmt * db_exec_sql_str(db_State * s, const char * sql) {
    const char * unused_bytes;
    sqlite3_stmt * stmt;
    if(sqlite3_prepare_v2(s->db, sql, strlen(sql), &stmt, &unused_bytes) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    sqlite3_step(stmt);
    return stmt;
}

int db_count_rows(db_State * s, const char * table_name) {
    char sql[1024];
    sqlite3_snprintf(sizeof(sql), sql, "select count (*) from %s;", table_name);
    sqlite3_stmt * stmt = db_exec_sql_str(s, sql);
    if(stmt == NULL) DB_FATAL_SQL_ERROR(s, sql);
    return sqlite3_column_int(stmt, 0);
}
