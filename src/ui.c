
#define EDIT_STRING_CAP 1024

typedef struct {
    char buf[EDIT_STRING_CAP];
    int len;
} db_EditString;

typedef struct {
    db_EditString street;
    db_EditString street_extra;
    db_EditString city;
    db_EditString state;
    db_EditString postal;
    db_EditString country;
} db_Address;


void db_ui_label(db_State * s, const char * label) {
    nk_label(s->ctx, label, NK_TEXT_LEFT);
}

#define PADDING 5

void db_ui_edit_string(db_State * s, const char * label, db_EditString * out) {
    unsigned long len = s->ctx->style.font->width(s->ctx->style.font->userdata, s->ctx->style.font->height, label, strlen(label));
    if(nk_group_begin(s->ctx, "Edit String", NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_template_begin(s->ctx, s->ctx->style.font->height + s->ctx->style.window.padding.y * 2);
        nk_layout_row_template_push_static(s->ctx, len);
        nk_layout_row_template_push_variable(s->ctx, 100);
        nk_layout_row_template_end(s->ctx);
        db_ui_label(s, label);
        nk_edit_string(s->ctx, NK_EDIT_FIELD, out->buf, &out->len, sizeof(out->buf), NULL);
        nk_group_end(s->ctx);
    }

}

void db_ui_address(db_State * s, db_Address * addr) {
    nk_layout_row_dynamic(s->ctx, s->ctx->style.font->height * 3 + s->ctx->style.window.padding.y * 19, 1);
    if(nk_group_begin_titled(s->ctx, "Address", "Address Information", NK_WINDOW_DYNAMIC)) {

        nk_layout_row_dynamic(s->ctx, s->ctx->style.font->height + s->ctx->style.window.padding.y * 4, 1);
        db_ui_edit_string(s, "Street: ",  &addr->street);

        nk_layout_row_dynamic(s->ctx, s->ctx->style.font->height + s->ctx->style.window.padding.y * 4, 2);
        db_ui_edit_string(s, "PO/APT #: ", &addr->street_extra);
        db_ui_edit_string(s, "City: ", &addr->city);

        nk_layout_row_dynamic(s->ctx, s->ctx->style.font->height + s->ctx->style.window.padding.y * 4, 3);
        db_ui_edit_string(s, "State: ", &addr->state);
        db_ui_edit_string(s, "Postal: ", &addr->postal);
        db_ui_edit_string(s, "Country: ", &addr->country);

        nk_group_end(s->ctx);
    }
}
