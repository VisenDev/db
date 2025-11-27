//#include "3rdparty/raygui/examples/styles/style_amber.h"
#include "3rdparty/raygui/examples/styles/style_ashes.h"
#include "3rdparty/raygui/examples/styles/style_bluish.h"  
#include "3rdparty/raygui/examples/styles/style_candy.h"  
#include "3rdparty/raygui/examples/styles/style_cherry.h"
#include "3rdparty/raygui/examples/styles/style_cyber.h"
#include "3rdparty/raygui/examples/styles/style_dark.h"
#include "3rdparty/raygui/examples/styles/style_enefete.h" 
#include "3rdparty/raygui/examples/styles/style_jungle.h"  
#include "3rdparty/raygui/examples/styles/style_lavanda.h" 
#include "3rdparty/raygui/examples/styles/style_sunny.h"   
#include "3rdparty/raygui/examples/styles/style_terminal.h"

#define EDIT_STRING_CAP 1024

typedef struct {
    char buf[EDIT_STRING_CAP];
    int len;
    bool active;
} ui_EditString;

void ui_textbox(Rectangle bounds, ui_EditString * out) {
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    if (GuiTextBox(bounds, out->buf, EDIT_STRING_CAP, out->active)) {
        out->active = !out->active;
    }
}

void ui_labelled_textbox(Rectangle bounds, const char * label, ui_EditString * out) {
    Rectangle label_bounds = bounds;
    label_bounds.width = 150;
    Rectangle textbox = bounds;
    textbox.x += label_bounds.width;
    textbox.width -= label_bounds.width;
    
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiLabel(label_bounds, label);


    ui_textbox(textbox, out);
}

typedef struct {
    ui_EditString street;
    ui_EditString street_extra;
    ui_EditString city;
    ui_EditString state;
    ui_EditString postal;
    ui_EditString country;
} ui_Address;

#define ROW_H 25
#define PAD 10

int ui_address(Rectangle bounds, ui_Address * out) {
    assert(bounds.height == -1 && "The address widget has a fixed height");

    const int x = bounds.x;
    const int w = bounds.width;
    const int h = ROW_H;
    const int qw = w / 4; /*quarter width*/

    int y = bounds.y;

    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_RIGHT);

    GuiLabel((Rectangle){x, y, qw, h}, "Street:");
    ui_textbox((Rectangle){x + qw, y, 3 * qw, h}, &out->street);
    y += ROW_H + PAD;

    GuiLabel((Rectangle){x, y, qw, h}, "PO/Apt #:");
    ui_textbox((Rectangle){x + qw, y, qw, h}, &out->street_extra);
    GuiLabel((Rectangle){x + 2 * qw, y, qw, h}, "City:");
    ui_textbox((Rectangle){x + 3 * qw, y, qw, h}, &out->city);
    y += ROW_H + PAD;

    GuiLabel((Rectangle){x, y, qw, h}, "State:");
    ui_textbox((Rectangle){x + qw, y, qw, h}, &out->state);
    GuiLabel((Rectangle){x + 2 * qw, y, qw, h}, "Postal:");
    ui_textbox((Rectangle){x + 3 * qw, y, qw, h}, &out->postal);
    y += ROW_H + PAD;

    GuiLabel((Rectangle){x, y, qw, h}, "Country:");
    ui_textbox((Rectangle){x + qw, y, 3 * qw, h}, &out->country);
    y += ROW_H + PAD;

    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);

    return y;
}
