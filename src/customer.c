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

void db_customer_fprint(FILE * fp, const db_Customer * c) {
    fprintf(
        fp,
        "{"
        "\"id\": %d, "
        "\"name\": \"%s\", "
        "\"address_line1\": \"%s\", "
        "\"address_line2\": \"%s\", "
        "\"city\": \"%s\", "
        "\"state\": \"%s\", "
        "\"postal_code\": \"%s\", "
        "\"country\": \"%s\", "
        "\"phone\": \"%s\", "
        "\"email\": \"%s\", "
        "\"website\": \"%s\", "
        "\"primary_contact_name\": \"%s\", "
        "\"primary_contact_phone\": \"%s\", "
        "\"primary_contact_email\": \"%s\", "
        "\"notes\": \"%s\", "
        "\"created_at\": %lld, "
        "\"updated_at\": %lld"
        "}",
        c->id,
        c->name.buf,
        c->address_line1.buf,
        c->address_line2.buf,
        c->city.buf,
        c->state.buf,
        c->postal_code.buf,
        c->country.buf,
        c->phone.buf,
        c->email.buf,
        c->website.buf,
        c->primary_contact_name.buf,
        c->primary_contact_phone.buf,
        c->primary_contact_email.buf,
        c->notes.buf,
        (long long) c->created_at,
        (long long) c->updated_at
    );
}


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
            "id, "
            "name, "
            "address_line1, "
            "address_line2, "
            "city, "
            "state, "
            "postal_code, "
            "country, "
            "phone, "
            "email, "
            "website, "
            "primary_contact_name, "
            "primary_contact_phone, "
            "primary_contact_email, "
            "notes, "
            "created_at, "
            "updated_at "
        "from customers where id = ?;";
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
    assert(out->id == id);
    db_sql_column_editstring(stmt, index++, &out->name);
    db_sql_column_editstring(stmt, index++, &out->address_line1);
    db_sql_column_editstring(stmt, index++, &out->address_line2);
    db_sql_column_editstring(stmt, index++, &out->city);
    db_sql_column_editstring(stmt, index++, &out->state);
    db_sql_column_editstring(stmt, index++, &out->postal_code);
    db_sql_column_editstring(stmt, index++, &out->country);
    db_sql_column_editstring(stmt, index++, &out->phone);
    db_sql_column_editstring(stmt, index++, &out->email);
    db_sql_column_editstring(stmt, index++, &out->website);
    db_sql_column_editstring(stmt, index++, &out->primary_contact_name);
    db_sql_column_editstring(stmt, index++, &out->primary_contact_phone);
    db_sql_column_editstring(stmt, index++, &out->primary_contact_email);
    db_sql_column_editstring(stmt, index++, &out->notes);
    out->created_at = sqlite3_column_int(stmt, index++);
    out->updated_at = sqlite3_column_int(stmt, index++);

    sqlite3_reset(stmt);
}

void db_customer_write(db_State * s, const db_Customer * in) {
    static const char * sql =
        "update customers "
        "set "
            "name = ?, "
            "address_line1 = ?, "
            "address_line2 = ?, "
            "city = ?, "
            "state = ?, "
            "postal_code = ?, "
            "country = ?, "
            "phone = ?, "
            "email = ?, "
            "website = ?, "
            "primary_contact_name = ?, "
            "primary_contact_phone = ?, "
            "primary_contact_email = ?, "
            "notes = ?, "
            "updated_at = ? "
        "where id = ?;";
    
    static sqlite3_stmt * stmt = NULL;
    const char * unused_bytes;
    if(stmt == NULL) {
        if(sqlite3_prepare_v2(s->db, sql, strlen(sql), &stmt, &unused_bytes) != SQLITE_OK)
            DB_FATAL_SQL_ERROR(s, sql);
        core_on_exit(db_finalize_stmt, stmt);
    }
    int index = 1;
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->name.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->address_line1.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->address_line2.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->city.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->state.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->postal_code.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->country.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->phone.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->email.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->website.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->primary_contact_name.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->primary_contact_phone.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->primary_contact_email.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    if(SQLITE_OK != sqlite3_bind_text(stmt, index++, in->notes.buf, -1, SQLITE_TRANSIENT)) DB_FATAL_SQL_ERROR(s, sql);
    time_t updated_at_time;
    time(&updated_at_time);
    sqlite3_bind_int(stmt, index++, updated_at_time);

    sqlite3_bind_int(stmt, index++, in->id);

    if(sqlite3_step(stmt) != SQLITE_DONE) DB_FATAL_SQL_ERROR(s, sql);
    sqlite3_reset(stmt);

    printf("Writing customer: ");
    db_customer_fprint(stdout, in);
    puts("");
}

void menu_customers(db_State * s, int y) {
    int w = (GetScreenWidth() / 3) - PAD * 2 + PAD / 2;

    const char * sql = "select id,name from customers order by id;";
    static char buf[10000];
    unsigned long fill = 0;
    sqlite3_stmt * stmt;
    const char * unused_bytes;


    static int * id_mapping = NULL;
    static int id_mapping_len = 0;
    int num_customers = db_count_rows(s, "customers");
    if(id_mapping == NULL || num_customers != id_mapping_len) {
        if(id_mapping != NULL) {
            free(id_mapping);
        }
        id_mapping = malloc(sizeof(int) * num_customers);
    }

    if(sqlite3_prepare_v2(s->db, sql, strlen(sql), &stmt, &unused_bytes) != SQLITE_OK) DB_FATAL_SQL_ERROR(s, sql);

    int i = 0;
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        if(fill != 0) {
            core_strfmt(buf, sizeof(buf), &fill, ";");
        }
        id_mapping[i] = sqlite3_column_int(stmt, 0);
        static char num[1024];
        sqlite3_snprintf(sizeof(num), num, " [%03d] ", id_mapping[i]);
        core_strfmt(buf, sizeof(buf), &fill, num);
        core_strfmt(buf, sizeof(buf), &fill, (char*)sqlite3_column_text(stmt, 1));
        ++i;
    }
    sqlite3_finalize(stmt);
    static int scroll_index = 0;
    static int active_id = -1;
    static int old_active_id = -1;
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
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
            db_customer_read(s, id_mapping[active_id], &customer);
            old_active_id = active_id;
        }

        edit_bounds.width -= PAD * 2;

        edit_bounds.y += ROW_H + PAD;
        ui_textbox(edit_bounds, &customer.name);

        edit_bounds.y += ROW_H + PAD;
        ui_textbox(edit_bounds, &customer.address_line1);

        edit_bounds.y += ROW_H + PAD;
        ui_textbox(edit_bounds, &customer.address_line2);

        edit_bounds.y += ROW_H + PAD;
        if(GuiButton(edit_bounds, " Save")) {
            db_customer_write(s, &customer);
            old_active_id = -1;
        }
    }
        
}
