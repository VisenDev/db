
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
        id_mapping_len = num_customers;
    }

    if(sqlite3_prepare_v2(s->db, sql, strlen(sql), &stmt, &unused_bytes) != SQLITE_OK) SQL_FATAL_ERROR(s, sql);

    int i = 0;
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        if(fill != 0) {
            core_strfmt(buf, sizeof(buf), &fill, ";");
        }
        assert(i < id_mapping_len);
        id_mapping[i] = sqlite3_column_int(stmt, 0);
        if(id_mapping[i] == 0) CORE_FATAL_ERROR("Null id");
        static char num[1024];
        sqlite3_snprintf(sizeof(num), num, " [%03d] ", sqlite3_column_int(stmt, 0));
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

    Rectangle edit_bounds = (Rectangle){(GetScreenWidth() / 3) + PAD - PAD / 2, y + PAD, w * 2 + PAD + (PAD * 0.75), GetScreenHeight() - y - 2 * PAD};
    GuiPanel(edit_bounds, "Edit Customer");
    if(active_id != -1) {
        edit_bounds.y = y + PAD + ROW_H;
        edit_bounds.x += PAD;
        edit_bounds.height = ROW_H;

        char msg[1024];
        GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        sqlite3_snprintf(sizeof(msg), msg, "Editing Customer Id [%003d]", id_mapping[active_id]);
        GuiLabel(edit_bounds, msg);
        static db_Customer customer = {0};
        if(old_active_id != active_id) {
            db_customer_read(s, id_mapping[active_id], &customer);
            old_active_id = active_id;
        }
        edit_bounds.width -= PAD * 2;

        edit_bounds.y += ROW_H + PAD;
        ui_labelled_textbox(edit_bounds, "Name:", &customer.name);

        edit_bounds.y += ROW_H + PAD;
        ui_labelled_textbox(edit_bounds, "Street Address:", &customer.address_line1);

        edit_bounds.y += ROW_H + PAD;
        ui_labelled_textbox(edit_bounds, "PO Box/Apt/etc...:", &customer.address_line2);

        edit_bounds.y += ROW_H + PAD;
        edit_bounds.width -= (ROW_H * 4 + PAD);

        //TODO: autosave
        if(GuiButton(edit_bounds, " Save")) {
            db_customer_write(s, &customer);
            old_active_id = -1;
        }
        edit_bounds.x += edit_bounds.width + PAD;
        edit_bounds.width = ROW_H * 4;
        static bool show_confirm_delete = false;
        if(GuiButton(edit_bounds, " Delete")) {
            show_confirm_delete = true;
            old_active_id = -1;
        }

        if (show_confirm_delete) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(RED, 0.8f));
            int result = GuiMessageBox((Rectangle){ (float)GetScreenWidth()/2 - 125, (float)GetScreenHeight()/2 - 50, 250, 100 }, GuiIconText(ICON_WARNING, "Confirm Deletetion"), "Do you really want delete this customer?", "Yes;No");

            if ((result == 0) || (result == 2)) {
                show_confirm_delete = false;
            } else if (result == 1) {
                sqlite3_stmt * delete_stmt = db_prepare_and_bind(s, "delete from customers where id = ?;", "%d", id_mapping[active_id]);
                sqlite3_step(delete_stmt);
                sqlite3_finalize(delete_stmt);

                active_id = -1;
                show_confirm_delete = false;
            }
        }
    }
}
