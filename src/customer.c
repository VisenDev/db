const char * customer_sql =
"-- This table stores one row per customer (company). Think: company name, address, primary contact, phone, etc.\n"
"CREATE TABLE customers (                                                                                       \n"
"    id              INTEGER PRIMARY KEY AUTOINCREMENT, -- internal unique id for each customer                 \n"
"    name            TEXT NOT NULL,                     -- company or customer name (required)                  \n"
"    tax_id          TEXT,                              -- optional tax or VAT id                               \n"
"    address_line1   TEXT,                              -- street address (line 1)                              \n"
"    address_line2   TEXT,                              -- street address (line 2, optional)                    \n"
"    city            TEXT,                                                                                      \n"
"    state           TEXT,                                                                                      \n"
"    postal_code     TEXT,                                                                                      \n"
"    country         TEXT,                                                                                      \n"
"    phone           TEXT,                                                                                      \n"
"    email           TEXT,                                                                                      \n"
"    website         TEXT,                                                                                      \n"
"    primary_contact_name  TEXT,                        -- a person to contact at that company                  \n"
"    primary_contact_phone TEXT,                                                                                \n"
"    primary_contact_email TEXT,                                                                                \n"
"    notes           TEXT,                              -- free-form notes about the customer                   \n"
"    created_at      DATETIME DEFAULT (datetime('now')),-- when this row was created                            \n"
"    updated_at      DATETIME                           -- last update time (set by your app)                   \n"
");                                                                                                             \n"
;

__attribute__((format (printf, 3, 4)))
sqlite3_stmt * db_prepare_and_bind(db_State * s, const char * sql, const char * fmt, ...) {
    
    sqlite3_stmt * result = NULL;
    const char * unused_bytes;
    unsigned long i = 0;
    const unsigned long len = strlen(sql);
    if(sqlite3_prepare_v2(s->db, sql, len, &result, &unused_bytes) != SQLITE_OK) CORE_FATAL_ERROR("1");//return NULL;

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

void ui_customer_display_row(db_State * s, Rectangle bounds, int id) {
    const char * sql = "select * from customers where id = ?;";
    sqlite3_stmt * stmt = db_prepare_and_bind(s, sql, "%d", id);
    if(stmt == NULL) CORE_FATAL_ERROR("Failed to prepare statement");
    sqlite3_step(stmt);

    static char buf[1024];
    sqlite3_snprintf(sizeof(buf), buf, "%d", sqlite3_column_int(stmt, 1));
    GuiLabel(bounds, buf);
    (void)bounds;
    if(stmt != NULL) {
        sqlite3_finalize(stmt);
    }
}
