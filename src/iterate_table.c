
void db_iterate_table(sqlite3 * db, const char * table_name) {
    sqlite3_stmt * stmt;
    char sql[256];
    const char * unused;
    int code;
    sqlite3_snprintf(sizeof(sql), sql, "pragma table_info(%s);", table_name);
    
    if((code = sqlite3_prepare_v2(db, sql, -1, &stmt, &unused)) != SQLITE_OK) {
        fprintf(stderr, "\n%s\n", sqlite3_errstr(code));
        CORE_FATAL_ERROR("Failed to prepare statement");
    }

    while((code = sqlite3_step(stmt)) == SQLITE_ROW) {
        int col;
        int count = sqlite3_column_count(stmt);
        for(col = 0; col < count; ++col) {
            int type = sqlite3_column_type(stmt, col);
            switch(type) {
            case SQLITE_INTEGER:
                printf("%d,", sqlite3_column_int(stmt, col));
                break;
            case SQLITE_FLOAT:
                printf("%lf,", sqlite3_column_double(stmt, col));
                break;
            case SQLITE_TEXT:
                printf("%s,", sqlite3_column_text(stmt, col));
                break;
            case SQLITE_BLOB: {
                int bytes = sqlite3_column_bytes(stmt, col);
                (void)bytes;
                printf("%p,", sqlite3_column_blob(stmt, col));
            } break;
            case SQLITE_NULL:
                printf("NULL,");
                break;
            }
        }
        printf("\n");
    }
    if(code != SQLITE_DONE) {
        CORE_FATAL_ERROR("step returned error");
    }
    sqlite3_finalize(stmt);
}


sqlite3_stmt * db_table_info(sqlite3 * db, const char * table_name) {
    sqlite3_stmt * stmt;
    char sql[256];
    const char * unused;
    int code;
    sqlite3_snprintf(sizeof(sql), sql, "pragma table_info(%s);", table_name);
    
    if((code = sqlite3_prepare_v2(db, sql, -1, &stmt, &unused)) != SQLITE_OK) {
        fprintf(stderr, "\n%s\n", sqlite3_errstr(code));
        return NULL;
    }
    return stmt;
}


/*
typedef struct {
    const char * name;
    int type;
} db_TableMetadataEntry;

typedef core_Vec(db_TableMetadataEntry) db_TableMetadata;

core_Bool db_table_metadata(sqlite3 * db, core_Arena * a, const char * table_name, db_TableMetadata * out) {
    sqlite3_stmt * stmt;
    char sql[256];
    const char * unused;
    int code;
    db_TableMetadata meta = {0};
    (void)meta;
    sqlite3_snprintf(sizeof(sql), sql, "pragma table_info(%s);", table_name);
    
    if((code = sqlite3_prepare_v2(db, sql, -1, &stmt, &unused)) != SQLITE_OK) {
        fprintf(stderr, "\n%s\n", sqlite3_errstr(code));
        return CORE_FALSE;
    }

    while((code = sqlite3_step(stmt)) == SQLITE_ROW) {
        db_TableMetadataEntry entry = {0};
        char * typestr = sqlite3_column_str(stmt, 2);
        if(strcmp("TEXT", typestr) == 0) {
            typestr = SQLITE_TEXT;
        }
        entry.name
    }
}
*/
