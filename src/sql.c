/**** THIS FILE CONTAINS SQL UTILITIES ****/

#define SQL_FATAL_ERROR(state, sql_code) do {                            \
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
        SQL_FATAL_ERROR(s, sql);
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
    char * buf;
    core_Bool success = CORE_TRUE;
    buf = core_file_read_all_arena(&arena, sql_file_path);
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
    if(stmt == NULL) SQL_FATAL_ERROR(s, sql);
    return sqlite3_column_int(stmt, 0);
}


/* void db_bind_editstring(db_State * s, sqlite3_stmt * stmt, int index, const ui_EditString * in) { */
/*     if(SQLITE_OK != sqlite3_bind_text(stmt, index, in->buf, -1, SQLITE_TRANSIENT))  */
/*         SQL_FATAL_ERROR(s, ""); */
/* } */





//META

typedef struct {
    int tag; /*SQLITE BACKING TYPE TAG*/
    union {
        int integer;
        const char * text;
    } as;
} sql_Value;


typedef struct {
    const char * name;
    const char * type;
    int sqlite_backing_type;
} sql_FieldSchema;

typedef struct {
    const sql_FieldSchema * fields;
    unsigned int nfields;
    const char * name;
} sql_TableSchema;
typedef core_Hashmap(sql_Value) sql_Values;

bool sql_tableschema_has_field(sql_TableSchema tbl, const char * field_name) {
    size_t i;
    for(i = 0; i < tbl.nfields; ++i) {
        if(strcmp(tbl.fields[i].name, field_name) == 0) return true;
    }
    return false;
}

bool sql_table_create(db_State * s, sql_TableSchema tbl) {
    (void)s;
    static char buf[10000];
    size_t fill = 0;

#   define add(str) core_strfmt(buf, sizeof(buf), &fill, str)

    add("create table ");
    add(tbl.name);
    add(" (\n");
    

    size_t i;
    for(i = 0; i < tbl.nfields; ++i) {
        if(i != 0) {
            add(",\n");
        }
        add("\t");
        add(tbl.fields[i].name);
        add(" ");
        add(tbl.fields[i].type);
    }
    
    add("\n);");

#   undef add
    if(sqlite3_exec(s->db, buf, NULL, NULL, NULL) != SQLITE_OK) return false;
    return true;
}

core_Bool sql_table_insert(db_State * s, sql_TableSchema tbl, sql_Values data) {
    (void)s;
    static char buf[10000];
    size_t fill = 0;
    bool prepend_comma = false;
    size_t i;
    sqlite3_stmt * stmt;

#   define add(str) core_strfmt(buf, sizeof(buf), &fill, str)

    add("insert into ");
    add(tbl.name);
    add("(");

    prepend_comma = false;
    for(i = 0; i < data.keys.len; ++i) {
        if(prepend_comma) {
            add(", ");
        } else prepend_comma = true;
        const char * key = data.keys.items[i];
        if(!sql_tableschema_has_field(tbl, key)) {
            CORE_LOG("Attempted to insert non-existant field");
            CORE_LOG(key);
            CORE_LOG(buf);
            return false;
        }
        add(key);
    }


    add(") values(");

    prepend_comma = false;
    for(i = 0; i < data.keys.len; ++i) {
        if(prepend_comma) {
            add(", ");
        } else prepend_comma = true;
        {
            const char * key = data.keys.items[i];
            add(":");
            add(key);
        }
    }
   
    add(");");

#   undef add
    
    if(sqlite3_prepare_v2(s->db, buf, -1, &stmt, NULL) != SQLITE_OK) return false;
    for(i = 0; i < data.keys.len; ++i) {
        const char * key = data.keys.items[i];
        const sql_Value val = data.values.items[i];
        char buf[1024];
        int index;
        sqlite3_snprintf(sizeof(buf), buf, ":%s", key);
        index = sqlite3_bind_parameter_index(stmt, buf);
        if(index == 0) {
            CORE_FATAL_ERROR("Failed to bind key");
        }
        switch(val.tag) {
        case SQLITE_INTEGER:
            printf("Bound %s to %d at index %d\n", key, val.as.integer, index);
            sqlite3_bind_int(stmt, index, val.as.integer);
            break;
        case SQLITE_TEXT:
            printf("Bound %s to %s at index %d\n", key, val.as.text, index);
            sqlite3_bind_text(stmt, index, val.as.text, -1, SQLITE_TRANSIENT);
            break;
        default:
            CORE_FATAL_ERROR("invalid tag");
            break;
        }
    }
    
    if(sqlite3_step(stmt) != SQLITE_DONE) {
        SQL_FATAL_ERROR(s, buf);
    }
    sqlite3_finalize(stmt);
    return true;
}
