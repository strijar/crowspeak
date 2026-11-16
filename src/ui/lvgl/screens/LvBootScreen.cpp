#include "LvBootScreen.h"
#include "Style.h"

namespace {

int progressToPercent(float progress) {
    if (progress < 0.0f)
        return 0;
    if (progress > 1.0f)
        return 100;
    return (int) (progress * 100.0f + 0.5f);
}

} // namespace

void LvBootScreen::createUI(lv_obj_t *parent) {
    _screen = parent;
    {
        lv_obj_remove_style(parent, Style::listColor, 0);
        lv_obj_add_style(parent, Style::listColor, 0);
    }
    {
        lv_obj_remove_style(parent, Style::opaque, 0);
        lv_obj_add_style(parent, Style::opaque, 0);
    }
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    _lblTitle = lv_label_create(parent);
    lv_obj_add_style(_lblTitle, Style::bootTitle, 0);
    lv_obj_add_style(_lblTitle, Style::fontHeading, 0);
    lv_obj_add_style(_lblTitle, Style::textAccent, 0);

    lv_label_set_long_mode(_lblTitle, LV_LABEL_LONG_DOT);
    lv_label_set_text(_lblTitle, "RATSPEAK");

    _bar = lv_bar_create(parent);
    lv_obj_add_style(_bar, Style::bootProgress, 0);

    lv_bar_set_range(_bar, 0, 100);
    lv_bar_set_value(_bar, 0, LV_ANIM_OFF);
    lv_obj_add_style(_bar, Style::barColor, LV_PART_MAIN);
    lv_obj_add_style(_bar, Style::bar, LV_PART_MAIN);
    lv_obj_add_style(_bar, Style::barIndicatorColor, LV_PART_INDICATOR);
    lv_obj_add_style(_bar, Style::bar, LV_PART_INDICATOR);
}

void LvBootScreen::setProgress(float progress, const char *status) {
    (void) status;
    int percent = progressToPercent(progress);
    if (_bar) {
        lv_bar_set_value(_bar, percent, LV_ANIM_OFF);
    }
    // Force LVGL to flush during boot (before main loop)
    lv_timer_handler();
}

void LvBootScreen::showError(const char *message) {
    if (_bar)
        lv_obj_add_flag(_bar, LV_OBJ_FLAG_HIDDEN);
    if (_lblTitle) {
        lv_obj_remove_style(_lblTitle, Style::errorHeading, 0);
        lv_obj_add_style(_lblTitle, Style::errorHeading, 0);
    }

    auto *detail = lv_label_create(_screen);
    lv_obj_add_style(detail, Style::errorDescription, 0);
    lv_obj_add_style(detail, Style::fontBody, 0);
    lv_obj_add_style(detail, Style::textPrimary, 0);

    lv_label_set_long_mode(detail, LV_LABEL_LONG_DOT);
    lv_label_set_text(detail, message);

    lv_label_set_long_mode(detail, LV_LABEL_LONG_WRAP);

    auto *hint = lv_label_create(_screen);
    lv_obj_add_style(hint, Style::footerHint, 0);
    lv_obj_add_style(hint, Style::fontCaption, 0);
    lv_obj_add_style(hint, Style::textSecondary, 0);

    lv_label_set_long_mode(hint, LV_LABEL_LONG_DOT);
    lv_label_set_text(hint, "Restart to retry.\nRestore a backup if this continues.");

    lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);

    lv_timer_handler();
}

void LvBootScreen::showAllocationError(const char *staticMessage) {
    if (_bar)
        lv_obj_add_flag(_bar, LV_OBJ_FLAG_HIDDEN);
    if (_lblTitle) {
        // LONG_DOT may alter text and save a suffix. Finalize that mode before
        // borrowing read-only text; WRAP requires no copied label buffer.
        lv_label_set_long_mode(_lblTitle, LV_LABEL_LONG_WRAP);
        lv_label_set_text_static(_lblTitle, staticMessage);
    }
}
