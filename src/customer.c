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

typedef struct {
    int id;
    ui_EditString name;
    ui_EditString address_line1;
    ui_EditString address_line2;
    ui_EditString city;
    ui_EditString state;
    ui_EditString postal_code;
    ui_EditString country;
    ui_EditString phone;
    ui_EditString email;
    ui_EditString website;
    ui_EditString primary_contact_name;
    ui_EditString primary_contact_phone;
    ui_EditString primary_contact_email;
    ui_EditString notes;
    time_t created_at;
    time_t updated_at;
} db_Customer;

void db_finalize_stmt(void * stmt) {
    sqlite3_finalize(stmt);
}

void db_sql_column_editstring(sqlite3_stmt * stmt, int index, ui_EditString * out) {
    const char * str = (char*)sqlite3_column_text(stmt, index);
    if(str == NULL) {
        out->len = 0;
        out->buf[0] = 0;
        return;
    }
    assert(str);
    size_t len = strlen(str);
    size_t count = CORE_MIN(len, EDIT_STRING_CAP - 1);
    
    out->active = 0;
    out->len = count;
    memcpy(out->buf, str, count + 1);
}

void db_customer_read(db_State * s, int id, db_Customer * out) {
    const char * sql =
        "select "
            "id,"
            "name,"
            "address_line1,"
            "address_line2,"
            "city,"
            "state,"
            "postal_code,"
            "country,"
            "phone,"
            "email,"
            "website,"
            "primary_contact_name,"
            "primary_contact_phone,"
            "primary_contact_email,"
            "notes,"
            "created_at,"
            "updated_at "
        "from customers where id = ?;";
    puts("");
    puts(sql);
    static sqlite3_stmt * stmt = NULL;
    const char * unused_bytes;
    if(stmt == NULL) {
        if(sqlite3_prepare_v2(s->db, sql, strlen(sql), &stmt, &unused_bytes) != SQLITE_OK)
            DB_FATAL_SQL_ERROR(s, sql);
        core_on_exit(db_finalize_stmt, stmt);
    }
    if(sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) DB_FATAL_SQL_ERROR(s, sql);
    sqlite3_step(stmt);


    int index = 0;
    out->id = sqlite3_column_int(stmt, index++);
    db_sql_column_editstring(    stmt, index++, &out->name);
    db_sql_column_editstring(    stmt, index++, &out->address_line1);
    db_sql_column_editstring(    stmt, index++, &out->address_line2);
    db_sql_column_editstring(    stmt, index++, &out->city);
    db_sql_column_editstring(    stmt, index++, &out->state);

    sqlite3_reset(stmt);
}

void menu_customers(db_State * s, int y) {
    int w = (GetScreenWidth() / 3) - PAD * 2 + PAD / 2;

    const char * sql = "select name from customers;";
    char buf[1024];
    unsigned long fill = 0;
    sqlite3_stmt * stmt;
    const char * unused_bytes;
    if(sqlite3_prepare_v2(s->db, sql, strlen(sql), &stmt, &unused_bytes) != SQLITE_OK) DB_FATAL_SQL_ERROR(s, sql);

    //TODO:
    //   This way of constructing the ui isn't preserving ids correctly
    //   I need to redo this so that active_id and the database customer id can be matched correctly
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        if(fill != 0) {
            core_strfmt(buf, sizeof(buf), &fill, ";");
        }
        core_strfmt(buf, sizeof(buf), &fill, (char*)sqlite3_column_text(stmt, 0));
    }
    sqlite3_finalize(stmt);
    static int scroll_index = 0;
    static int active_id = -1;
    static int old_active_id = -1;
    GuiListView((Rectangle){PAD, y + PAD, w, GetScreenHeight() - y - 2 * PAD}, buf, &scroll_index, &active_id);

    Rectangle edit_bounds = (Rectangle){(GetScreenWidth() / 3) + PAD - PAD / 2, y + PAD, w * 2 + PAD + PAD, GetScreenHeight() - y - 2 * PAD};
    GuiPanel(edit_bounds, "Edit Customer");
    if(active_id != -1) {
        edit_bounds.y = y + PAD + ROW_H;
        edit_bounds.x += PAD;
        edit_bounds.height = ROW_H;

        GuiLabel(edit_bounds, "Edit Customer Information Here");
        static db_Customer customer = {0};
        if(old_active_id != active_id) {
            db_customer_read(s, active_id, &customer);
            old_active_id = active_id;
        }

        edit_bounds.y += ROW_H + PAD;
        ui_textbox(edit_bounds, &customer.name);

        edit_bounds.y += ROW_H + PAD;
        ui_textbox(edit_bounds, &customer.address_line1);

        edit_bounds.y += ROW_H + PAD;
        ui_textbox(edit_bounds, &customer.address_line2);
    }
        
}
