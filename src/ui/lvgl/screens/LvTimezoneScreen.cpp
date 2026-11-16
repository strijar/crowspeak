#include "LvTimezoneScreen.h"
#include "Style.h"
#include "LvInput.h"
#include <Arduino.h>

namespace {

int clampTimezoneIndex(int idx) {
    if (idx < 0 || idx >= TIMEZONE_COUNT)
        return 6;
    return idx;
}

} // namespace

void LvTimezoneScreen::createUI(lv_obj_t *parent) {
    _screen = parent;
    _roller = nullptr;
    _upButton = nullptr;
    _downButton = nullptr;
    _doneButton = nullptr;
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    {
        lv_obj_remove_style(parent, Style::listColor, 0);
        lv_obj_add_style(parent, Style::listColor, 0);
    }
    {
        lv_obj_remove_style(parent, Style::opaque, 0);
        lv_obj_add_style(parent, Style::opaque, 0);
    }
    _selectedIdx = clampTimezoneIndex(_selectedIdx);

    lv_obj_t *eyebrow = lv_label_create(parent);
    lv_obj_add_style(eyebrow, Style::regionEyebrow, 0);
    lv_obj_add_style(eyebrow, Style::fontCaption, 0);
    lv_obj_add_style(eyebrow, Style::textSecondary, 0);

    lv_label_set_long_mode(eyebrow, LV_LABEL_LONG_DOT);
    lv_label_set_text(eyebrow, "REGION SETUP");

    lv_obj_t *title = _saveTitle = lv_label_create(parent);
    {
        lv_obj_remove_style(title, Style::setupTitle, 0);
        lv_obj_add_style(title, Style::setupTitle, 0);
    }
    {
        lv_obj_remove_style(title, Style::fontHeading, 0);
        lv_obj_add_style(title, Style::fontHeading, 0);
    }
    {
        lv_obj_remove_style(title, Style::textAccent, 0);
        lv_obj_add_style(title, Style::textAccent, 0);
    }

    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_label_set_text(title, "TIMEZONE");

    lv_obj_t *sub = lv_label_create(parent);
    lv_obj_add_style(sub, Style::regionDescription, 0);
    lv_obj_add_style(sub, Style::fontBody, 0);
    lv_obj_add_style(sub, Style::textPrimary, 0);

    lv_label_set_long_mode(sub, LV_LABEL_LONG_WRAP);
    lv_label_set_text(sub, "Pick the nearest city for time and radio-region hints.");

    // Build roller options string: "City  UTC+X\nCity  UTC-Y\n..."
    static char opts[640];
    int         pos = 0;
    for (int i = 0; i < TIMEZONE_COUNT; i++) {
        if (i > 0 && pos < (int) sizeof(opts) - 1)
            opts[pos++] = '\n';
        pos += snprintf(opts + pos, sizeof(opts) - pos, "%-13s UTC%+d", TIMEZONE_TABLE[i].label, TIMEZONE_TABLE[i].baseOffset);
    }

    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_add_style(card, Style::selectionCard, 0);

    lv_obj_add_style(card, Style::bgElevated, 0);

    lv_obj_add_style(card, Style::textareaFocusedColor, 0);

    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    _roller = lv_roller_create(card);
    lv_roller_set_options(_roller, opts, LV_ROLLER_MODE_NORMAL);
    lv_obj_add_style(_roller, Style::regionRoller, 0);

    lv_roller_set_selected(_roller, _selectedIdx, LV_ANIM_OFF);

    lv_obj_add_style(_roller, Style::rollerColor, LV_PART_MAIN);
    lv_obj_add_style(_roller, Style::roller, LV_PART_MAIN);
    lv_obj_add_style(_roller, Style::fontTitle, LV_PART_MAIN);
    lv_obj_add_style(_roller, Style::textCentered, LV_PART_MAIN);
    lv_obj_add_style(_roller, Style::bgHover, LV_PART_SELECTED);
    lv_obj_add_style(_roller, Style::selectedTextCentered, LV_PART_SELECTED);
    lv_obj_add_style(_roller, Style::textAccent, LV_PART_SELECTED);
    lv_obj_add_style(_roller, Style::fontTitle, LV_PART_SELECTED);

    // Add to focus group for trackball navigation
    lv_group_add_obj(LvInput::group(), _roller);
    lv_group_focus_obj(_roller);

#if HAS_TOUCH

    _upButton = lv_btn_create(parent);
    lv_obj_add_style(_upButton, Style::stepButton, 0);
    lv_obj_add_style(_upButton, Style::stepPrevious, 0);

    lv_obj_add_style(_upButton, Style::buttonColor, 0);
    lv_obj_add_style(_upButton, Style::button, 0);
    lv_obj_add_style(_upButton, Style::buttonFocusedColor, LV_STATE_FOCUSED);
    lv_obj_add_style(_upButton, Style::buttonFocused, LV_STATE_FOCUSED);
    lv_obj_add_style(_upButton, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_style(_upButton, Style::buttonPressed, LV_STATE_PRESSED);
    {
        for (auto *tone : Style::bgTones)
            lv_obj_remove_style(_upButton, tone, 0);
        lv_obj_add_style(_upButton, Style::bgTones[false ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED], 0);
    }
    {
        for (auto *tone : Style::borderTones)
            lv_obj_remove_style(_upButton, tone, 0);
        lv_obj_add_style(_upButton, Style::borderTones[false ? Style::PRIMARY : Style::BORDER], 0);
    }
    lv_obj_add_style(_upButton, Style::flatAction, 0);

    lv_obj_t *upButtonCaption = lv_label_create(_upButton);
    lv_obj_add_style(upButtonCaption, Style::fontBody, 0);
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(upButtonCaption, tone, 0);
        lv_obj_add_style(upButtonCaption, Style::textTones[false ? Style::ACCENT : Style::TEXT_PRIMARY], 0);
    }
    lv_label_set_long_mode(upButtonCaption, LV_LABEL_LONG_DOT);
    lv_obj_add_style(upButtonCaption, Style::buttonCaption, 0);

    lv_label_set_text(upButtonCaption, "UP");

    lv_obj_add_event_cb(_upButton, [](lv_event_t *e) {
        auto* self = static_cast<LvTimezoneScreen*>(lv_event_get_user_data(e));
        if (!self) return;
        self->stepSelection(-1); }, LV_EVENT_CLICKED, this);

    _doneButton = lv_btn_create(parent);
    lv_obj_add_style(_doneButton, Style::stepAccept, 0);

    lv_obj_add_style(_doneButton, Style::buttonColor, 0);
    lv_obj_add_style(_doneButton, Style::button, 0);
    lv_obj_add_style(_doneButton, Style::buttonFocusedColor, LV_STATE_FOCUSED);
    lv_obj_add_style(_doneButton, Style::buttonFocused, LV_STATE_FOCUSED);
    lv_obj_add_style(_doneButton, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_style(_doneButton, Style::buttonPressed, LV_STATE_PRESSED);
    {
        for (auto *tone : Style::bgTones)
            lv_obj_remove_style(_doneButton, tone, 0);
        lv_obj_add_style(_doneButton, Style::bgTones[true ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED], 0);
    }
    {
        for (auto *tone : Style::borderTones)
            lv_obj_remove_style(_doneButton, tone, 0);
        lv_obj_add_style(_doneButton, Style::borderTones[true ? Style::PRIMARY : Style::BORDER], 0);
    }
    lv_obj_add_style(_doneButton, Style::flatAction, 0);

    lv_obj_t *doneButtonCaption = lv_label_create(_doneButton);
    lv_obj_add_style(doneButtonCaption, Style::fontBody, 0);
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(doneButtonCaption, tone, 0);
        lv_obj_add_style(doneButtonCaption, Style::textTones[true ? Style::ACCENT : Style::TEXT_PRIMARY], 0);
    }
    lv_label_set_long_mode(doneButtonCaption, LV_LABEL_LONG_DOT);
    lv_obj_add_style(doneButtonCaption, Style::buttonCaption, 0);

    lv_label_set_text(doneButtonCaption, "DONE");

    lv_obj_add_event_cb(_doneButton, [](lv_event_t *e) {
        auto* self = static_cast<LvTimezoneScreen*>(lv_event_get_user_data(e));
        if (!self) return;
        self->submit(false); }, LV_EVENT_CLICKED, this);

    _downButton = lv_btn_create(parent);
    lv_obj_add_style(_downButton, Style::stepButton, 0);
    lv_obj_add_style(_downButton, Style::stepNext, 0);

    lv_obj_add_style(_downButton, Style::buttonColor, 0);
    lv_obj_add_style(_downButton, Style::button, 0);
    lv_obj_add_style(_downButton, Style::buttonFocusedColor, LV_STATE_FOCUSED);
    lv_obj_add_style(_downButton, Style::buttonFocused, LV_STATE_FOCUSED);
    lv_obj_add_style(_downButton, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_style(_downButton, Style::buttonPressed, LV_STATE_PRESSED);
    {
        for (auto *tone : Style::bgTones)
            lv_obj_remove_style(_downButton, tone, 0);
        lv_obj_add_style(_downButton, Style::bgTones[false ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED], 0);
    }
    {
        for (auto *tone : Style::borderTones)
            lv_obj_remove_style(_downButton, tone, 0);
        lv_obj_add_style(_downButton, Style::borderTones[false ? Style::PRIMARY : Style::BORDER], 0);
    }
    lv_obj_add_style(_downButton, Style::flatAction, 0);

    lv_obj_t *downButtonCaption = lv_label_create(_downButton);
    lv_obj_add_style(downButtonCaption, Style::fontBody, 0);
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(downButtonCaption, tone, 0);
        lv_obj_add_style(downButtonCaption, Style::textTones[false ? Style::ACCENT : Style::TEXT_PRIMARY], 0);
    }
    lv_label_set_long_mode(downButtonCaption, LV_LABEL_LONG_DOT);
    lv_obj_add_style(downButtonCaption, Style::buttonCaption, 0);

    lv_label_set_text(downButtonCaption, "DOWN");

    lv_obj_add_event_cb(_downButton, [](lv_event_t *e) {
        auto* self = static_cast<LvTimezoneScreen*>(lv_event_get_user_data(e));
        if (!self) return;
        self->stepSelection(1); }, LV_EVENT_CLICKED, this);
#endif
    setSaving(_saving);
}

void LvTimezoneScreen::stepSelection(int delta) {
    if (_saving)
        return;
    if (!_roller)
        return;

    int sel = lv_roller_get_selected(_roller);
    sel += delta;
    if (sel < 0)
        sel = 0;
    if (sel >= TIMEZONE_COUNT)
        sel = TIMEZONE_COUNT - 1;
    lv_roller_set_selected(_roller, sel, LV_ANIM_ON);
}

void LvTimezoneScreen::setSaving(bool saving) {
    _saving = saving;
    if (!_screen)
        return; // Terminal completion may arrive after UIManager destroyed the widgets.
    if (_saveTitle)
        lv_label_set_text(_saveTitle, saving ? "SAVING TIMEZONE" : "TIMEZONE");
    if (_roller) {
        if (saving)
            lv_obj_add_state(_roller, LV_STATE_DISABLED);
        else
            lv_obj_clear_state(_roller, LV_STATE_DISABLED);
    }
    if (_upButton) {
        if (saving)
            lv_obj_add_state(_upButton, LV_STATE_DISABLED);
        else
            lv_obj_clear_state(_upButton, LV_STATE_DISABLED);
    }
    if (_downButton) {
        if (saving)
            lv_obj_add_state(_downButton, LV_STATE_DISABLED);
        else
            lv_obj_clear_state(_downButton, LV_STATE_DISABLED);
    }
    if (_doneButton) {
        if (saving)
            lv_obj_add_state(_doneButton, LV_STATE_DISABLED);
        else
            lv_obj_clear_state(_doneButton, LV_STATE_DISABLED);
    }
    if (_doneButton) {
        lv_obj_t *label = lv_obj_get_child(_doneButton, 0);
        if (label)
            lv_label_set_text(label, saving ? "SAVING" : "DONE");
    }
}

void LvTimezoneScreen::submit(bool enforceEnterGuard) {
    if (_saving)
        return;
    if (!_roller)
        return;
    if (enforceEnterGuard && millis() - _enterTime < ENTER_GUARD_MS)
        return;

    _selectedIdx = lv_roller_get_selected(_roller);
    if (_doneCb)
        _doneCb(_selectedIdx);
}

bool LvTimezoneScreen::handleKey(const KeyEvent &event) {
    if (_saving)
        return true;
    if (!_roller)
        return true;
    if (event.enter || event.character == '\n' || event.character == '\r') {
        submit(true);
        return true;
    }
    // Trackball maps to PREV/NEXT (focus group nav), but roller needs
    // explicit selection changes since it's the only focusable widget
    if (event.up) {
        stepSelection(-1);
        return true;
    }
    if (event.down) {
        stepSelection(1);
        return true;
    }
    return true; // Consume all keys on this screen
}
