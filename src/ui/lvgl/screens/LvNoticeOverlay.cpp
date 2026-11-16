#include "LvNoticeOverlay.h"
#include "Style.h"
#include "config/BoardConfig.h"
#include <lvgl.h>

void LvNoticeOverlay::show(const char *title, const char *text, void (*dismissed)()) {
    if (_overlay)
        lv_obj_del(_overlay);
    _dismissed = dismissed;
    _overlay = lv_obj_create(lv_layer_top());
    lv_obj_add_style(_overlay, Style::noticeDialog, 0);

    lv_obj_add_style(_overlay, Style::modalColor, 0);
    lv_obj_add_style(_overlay, Style::modal, 0);
    lv_obj_add_style(_overlay, Style::noticeColumn, 0);

    lv_obj_clear_flag(_overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_overlay, [](lv_event_t *e) { static_cast<LvNoticeOverlay *>(lv_event_get_user_data(e))->hide(); }, LV_EVENT_CLICKED, this);

    lv_obj_t *overlayCaption2 = lv_label_create(_overlay);
    lv_obj_add_style(overlayCaption2, Style::fullWidth, 0);
    lv_label_set_long_mode(overlayCaption2, LV_LABEL_LONG_WRAP);
    lv_obj_add_style(overlayCaption2, Style::fontBody, 0);
    lv_obj_add_style(overlayCaption2, Style::textAccent, 0);
    lv_label_set_text(overlayCaption2, title);

    lv_obj_t *overlayCaption = lv_label_create(_overlay);
    lv_obj_add_style(overlayCaption, Style::fullWidth, 0);
    lv_label_set_long_mode(overlayCaption, LV_LABEL_LONG_WRAP);
    lv_obj_add_style(overlayCaption, Style::fontBody, 0);
    lv_obj_add_style(overlayCaption, Style::textPrimary, 0);
    lv_label_set_text(overlayCaption, text);

#if HAS_TOUCH

    lv_obj_t *footer = lv_label_create(_overlay);
    lv_obj_add_style(footer, Style::fullWidth, 0);
    lv_label_set_long_mode(footer, LV_LABEL_LONG_WRAP);
    lv_obj_add_style(footer, Style::fontCaption, 0);
    lv_obj_add_style(footer, Style::textMuted, 0);
    lv_label_set_text(footer, "Any key or tap closes");

#else

    lv_obj_t *footer = lv_label_create(_overlay);
    lv_obj_add_style(footer, Style::centeredText, 0);
    lv_label_set_long_mode(footer, LV_LABEL_LONG_WRAP);
    lv_obj_add_style(footer, Style::fontCaption, 0);
    lv_obj_add_style(footer, Style::textMuted, 0);
    lv_label_set_text(footer, "Any key closes");

#endif

    _visible = true;
}

void LvNoticeOverlay::hide() {
    if (!_visible)
        return;
    // A tap closes the notice from its own event; deletion waits for LVGL.
    lv_obj_del_async(_overlay);
    _overlay = nullptr;
    _visible = false;
    if (_dismissed)
        _dismissed();
}

bool LvNoticeOverlay::handleKey(const KeyEvent &event) {
    if (!_visible)
        return false;
    if (!event.repeat)
        hide();
    return true;
}
