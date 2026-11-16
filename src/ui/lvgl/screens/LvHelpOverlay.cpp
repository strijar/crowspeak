#include "LvHelpOverlay.h"
#include "Style.h"
#include "config/BoardConfig.h"
#include <lvgl.h>

void LvHelpOverlay::create() {
    if (_overlay)
        return;

    _overlay = lv_obj_create(lv_layer_top());
    lv_obj_add_style(_overlay, Style::helpDialog, 0);

    lv_obj_add_style(_overlay, Style::modalColor, 0);
    lv_obj_add_style(_overlay, Style::modal, 0);
    lv_obj_add_style(_overlay, Style::helpColumn, 0);

    lv_obj_clear_flag(_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_overlay, [](lv_event_t *e) {
        auto* self = static_cast<LvHelpOverlay*>(lv_event_get_user_data(e));
        if (self) self->hide(); }, LV_EVENT_CLICKED, this);

    lv_obj_t *header = lv_obj_create(_overlay);
    lv_obj_add_style(header, Style::helpHeader, 0);

    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *headerCaption2 = lv_label_create(header);
    lv_obj_add_style(headerCaption2, Style::helpTitle, 0);
    lv_obj_add_style(headerCaption2, Style::fontBody, 0);
    lv_obj_add_style(headerCaption2, Style::textAccent, 0);

    lv_label_set_long_mode(headerCaption2, LV_LABEL_LONG_DOT);
    lv_label_set_text(headerCaption2, "HELP");

    lv_obj_t *headerCaption = lv_label_create(header);
    lv_obj_add_style(headerCaption, Style::helpBrand, 0);
    lv_obj_add_style(headerCaption, Style::fontCaption, 0);
    lv_obj_add_style(headerCaption, Style::textSecondary, 0);

    lv_label_set_long_mode(headerCaption, LV_LABEL_LONG_DOT);
    lv_label_set_text(headerCaption, "Ratspeak.org");

    lv_obj_t *row4 = lv_obj_create(_overlay);
    lv_obj_add_style(row4, Style::helpRow, 0);

    lv_obj_clear_flag(row4, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *rowElement1025Caption2 = lv_label_create(row4);
    lv_obj_add_style(rowElement1025Caption2, Style::helpKey, 0);
    lv_obj_add_style(rowElement1025Caption2, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1025Caption2, Style::textAccent, 0);

    lv_label_set_long_mode(rowElement1025Caption2, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1025Caption2, "Tap field");

    lv_obj_t *rowElement1025Caption = lv_label_create(row4);
    lv_obj_add_style(rowElement1025Caption, Style::helpAction, 0);
    lv_obj_add_style(rowElement1025Caption, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1025Caption, Style::textPrimary, 0);

    lv_label_set_long_mode(rowElement1025Caption, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1025Caption, "Open text keyboard");

    lv_obj_t *row = lv_obj_create(_overlay);
    lv_obj_add_style(row, Style::helpRow, 0);

    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *rowElement1029Caption2 = lv_label_create(row);
    lv_obj_add_style(rowElement1029Caption2, Style::helpKey, 0);
    lv_obj_add_style(rowElement1029Caption2, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1029Caption2, Style::textAccent, 0);

    lv_label_set_long_mode(rowElement1029Caption2, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1029Caption2, "Lang");

    lv_obj_t *rowElement1029Caption = lv_label_create(row);
    lv_obj_add_style(rowElement1029Caption, Style::helpAction, 0);
    lv_obj_add_style(rowElement1029Caption, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1029Caption, Style::textPrimary, 0);

    lv_label_set_long_mode(rowElement1029Caption, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1029Caption, "Switch EN / RU layout");

    lv_obj_t *row3 = lv_obj_create(_overlay);
    lv_obj_add_style(row3, Style::helpRow, 0);

    lv_obj_clear_flag(row3, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *rowElement1033Caption2 = lv_label_create(row3);
    lv_obj_add_style(rowElement1033Caption2, Style::helpKey, 0);
    lv_obj_add_style(rowElement1033Caption2, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1033Caption2, Style::textAccent, 0);

    lv_label_set_long_mode(rowElement1033Caption2, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1033Caption2, "Shift / 123");

    lv_obj_t *rowElement1033Caption = lv_label_create(row3);
    lv_obj_add_style(rowElement1033Caption, Style::helpAction, 0);
    lv_obj_add_style(rowElement1033Caption, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1033Caption, Style::textPrimary, 0);

    lv_label_set_long_mode(rowElement1033Caption, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1033Caption, "Case / numbers and symbols");

    lv_obj_t *row2 = lv_obj_create(_overlay);
    lv_obj_add_style(row2, Style::helpRow, 0);

    lv_obj_clear_flag(row2, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *rowElement1037Caption2 = lv_label_create(row2);
    lv_obj_add_style(rowElement1037Caption2, Style::helpKey, 0);
    lv_obj_add_style(rowElement1037Caption2, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1037Caption2, Style::textAccent, 0);

    lv_label_set_long_mode(rowElement1037Caption2, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1037Caption2, "Done");

    lv_obj_t *rowElement1037Caption = lv_label_create(row2);
    lv_obj_add_style(rowElement1037Caption, Style::helpAction, 0);
    lv_obj_add_style(rowElement1037Caption, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1037Caption, Style::textPrimary, 0);

    lv_label_set_long_mode(rowElement1037Caption, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1037Caption, "Apply text to the field");

    lv_obj_t *row5 = lv_obj_create(_overlay);
    lv_obj_add_style(row5, Style::helpRow, 0);

    lv_obj_clear_flag(row5, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *rowElement1041Caption2 = lv_label_create(row5);
    lv_obj_add_style(rowElement1041Caption2, Style::helpKey, 0);
    lv_obj_add_style(rowElement1041Caption2, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1041Caption2, Style::textAccent, 0);

    lv_label_set_long_mode(rowElement1041Caption2, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1041Caption2, "Cancel");

    lv_obj_t *rowElement1041Caption = lv_label_create(row5);
    lv_obj_add_style(rowElement1041Caption, Style::helpAction, 0);
    lv_obj_add_style(rowElement1041Caption, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1041Caption, Style::textPrimary, 0);

    lv_label_set_long_mode(rowElement1041Caption, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1041Caption, "Keep the original text");

    lv_obj_t *row7 = lv_obj_create(_overlay);
    lv_obj_add_style(row7, Style::helpRow, 0);

    lv_obj_clear_flag(row7, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *rowElement1045Caption2 = lv_label_create(row7);
    lv_obj_add_style(rowElement1045Caption2, Style::helpKey, 0);
    lv_obj_add_style(rowElement1045Caption2, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1045Caption2, Style::textAccent, 0);

    lv_label_set_long_mode(rowElement1045Caption2, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1045Caption2, "SEND");

    lv_obj_t *rowElement1045Caption = lv_label_create(row7);
    lv_obj_add_style(rowElement1045Caption, Style::helpAction, 0);
    lv_obj_add_style(rowElement1045Caption, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1045Caption, Style::textPrimary, 0);

    lv_label_set_long_mode(rowElement1045Caption, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1045Caption, "Send the prepared message");

    lv_obj_t *row8 = lv_obj_create(_overlay);
    lv_obj_add_style(row8, Style::helpRow, 0);

    lv_obj_clear_flag(row8, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *rowElement1049Caption2 = lv_label_create(row8);
    lv_obj_add_style(rowElement1049Caption2, Style::helpKey, 0);
    lv_obj_add_style(rowElement1049Caption2, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1049Caption2, Style::textAccent, 0);

    lv_label_set_long_mode(rowElement1049Caption2, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1049Caption2, "Chat");

    lv_obj_t *rowElement1049Caption = lv_label_create(row8);
    lv_obj_add_style(rowElement1049Caption, Style::helpAction, 0);
    lv_obj_add_style(rowElement1049Caption, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1049Caption, Style::textPrimary, 0);

    lv_label_set_long_mode(rowElement1049Caption, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1049Caption, "Swipe history; hold message");

    lv_obj_t *row6 = lv_obj_create(_overlay);
    lv_obj_add_style(row6, Style::helpRow, 0);

    lv_obj_clear_flag(row6, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *rowElement1053Caption2 = lv_label_create(row6);
    lv_obj_add_style(rowElement1053Caption2, Style::helpKey, 0);
    lv_obj_add_style(rowElement1053Caption2, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1053Caption2, Style::textAccent, 0);

    lv_label_set_long_mode(rowElement1053Caption2, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1053Caption2, "Wake");

    lv_obj_t *rowElement1053Caption = lv_label_create(row6);
    lv_obj_add_style(rowElement1053Caption, Style::helpAction, 0);
    lv_obj_add_style(rowElement1053Caption, Style::fontCaption, 0);
    lv_obj_add_style(rowElement1053Caption, Style::textPrimary, 0);

    lv_label_set_long_mode(rowElement1053Caption, LV_LABEL_LONG_DOT);
    lv_label_set_text(rowElement1053Caption, "Tap once; then tap an action");

    lv_obj_t *footer = lv_label_create(_overlay);
    lv_obj_add_style(footer, Style::helpFooter, 0);
    lv_obj_add_style(footer, Style::fontCaption, 0);
    lv_obj_add_style(footer, Style::textMuted, 0);

    lv_label_set_long_mode(footer, LV_LABEL_LONG_DOT);
    lv_label_set_text(footer, "Tap to close");

    lv_obj_add_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
}

void LvHelpOverlay::show() {
    if (!_overlay)
        create();
    lv_obj_clear_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
    _visible = true;
}

void LvHelpOverlay::hide() {
    if (_overlay)
        lv_obj_add_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
    _visible = false;
}

void LvHelpOverlay::toggle() {
    if (_visible)
        hide();
    else
        show();
}

bool LvHelpOverlay::handleKey(const KeyEvent &event) {
    if (_visible) {
        if (event.repeat)
            return true; // dismiss only on a fresh keypress
        hide();
        return true;
    }
    return false;
}
