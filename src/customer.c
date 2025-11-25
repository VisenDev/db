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


void menu_customers(db_State * s, int y) {
    int count = db_count_rows(s, "customers");
    int i;
    int w = (GetScreenWidth() / 3) - PAD * 2;
    static int edit_id = -1;
    static Vector2 scroll = {0};
    static Rectangle v = {0};

    //TODO: swap the current scroll panel with a GuiListView
    //    GuiListView();

    GuiScrollPanel((Rectangle){PAD, y + PAD, w, GetScreenHeight() - y - 2 * PAD }, "Customers", (Rectangle){0, y, w - 14, count * ROW_H}, &scroll, &v);

    BeginScissorMode(v.x, v.y, v.width, v.height);
    for(i = 0; i < count; ++i) {
        const char * sql = "select * from customers where id = ?;";
        Rectangle bounds = (Rectangle){v.x + scroll.x, v.y + (i * ROW_H) + scroll.y, v.width / 2, ROW_H};
        sqlite3_stmt * stmt = db_prepare_and_bind(s, sql, "%d", i + 1);
        if(stmt == NULL) CORE_FATAL_ERROR("Failed to prepare statement");
        sqlite3_step(stmt);

        static char buf[1024];
        sqlite3_snprintf(sizeof(buf), buf, "Name: %s", sqlite3_column_text(stmt, 1));
        if(GuiButton(bounds, buf)) {
            edit_id = i + 1;
        }
        /* sqlite3_snprintf(sizeof(buf), buf, "Id: %d", sqlite3_column_int(stmt, 0)); */
        /* GuiGroupBox(bounds, NULL); */
        /* GuiLabel(bounds, buf); */

        /* bounds.x += bounds.width; */
        /* sqlite3_snprintf(sizeof(buf), buf, "Name: %s", sqlite3_column_text(stmt, 1)); */
        /* GuiGroupBox(bounds, NULL); */
        /* GuiLabel(bounds, buf); */

        if(stmt != NULL) {
            sqlite3_finalize(stmt);
        }
    }
    EndScissorMode();

    Rectangle edit_bounds = (Rectangle){(GetScreenWidth() / 3) + PAD, y + PAD, w * 2 + PAD + PAD, 256};
    GuiPanel(edit_bounds, "Edit Customer");
    if(edit_id != -1) {
        edit_bounds.y = y + PAD + ROW_H;
        edit_bounds.height = ROW_H;
        GuiLabel(edit_bounds, "Edit Customer Information Here");
    }

}
