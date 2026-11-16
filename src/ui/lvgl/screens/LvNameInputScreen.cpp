#include "LvNameInputScreen.h"
#include "Style.h"
#include "LvInput.h"
#include "config/Config.h"

void LvNameInputScreen::createUI(lv_obj_t *parent) {
    _screen = parent;
    _textarea = nullptr;
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
    lv_label_set_text(title, "DISPLAY NAME");

    lv_obj_t *line = lv_obj_create(parent);
    lv_obj_add_style(line, Style::setupDivider, 0);

    lv_obj_add_style(line, Style::bgDivider, 0);

    lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *prompt = lv_label_create(parent);
    lv_obj_add_style(prompt, Style::setupPrompt, 0);
    lv_obj_add_style(prompt, Style::fontBody, 0);
    lv_obj_add_style(prompt, Style::textPrimary, 0);

    lv_label_set_long_mode(prompt, LV_LABEL_LONG_WRAP);
    lv_label_set_text(prompt, "Choose the name other operators see on announces.");

    _textarea = lv_textarea_create(parent);
    lv_obj_add_style(_textarea, Style::singleLineInput, 0);

    lv_textarea_set_max_length(_textarea, MAX_NAME_LEN);
    lv_textarea_set_one_line(_textarea, true);
    lv_obj_remove_local_style_prop(_textarea, LV_STYLE_HEIGHT, 0);
    lv_textarea_set_placeholder_text(_textarea, "Optional");
    lv_obj_add_style(_textarea, Style::buttonColor, 0);
    lv_obj_add_style(_textarea, Style::textarea, 0);
    lv_obj_add_style(_textarea, Style::textareaFocusedColor, LV_STATE_FOCUSED);
    lv_obj_add_style(_textarea, Style::buttonPressed, LV_STATE_FOCUSED);
    lv_obj_add_style(_textarea, Style::fontTitle, 0);
    lv_group_add_obj(LvInput::group(), _textarea);
    lv_group_focus_obj(_textarea);

    _doneButton = lv_btn_create(parent);
    lv_obj_add_style(_doneButton, Style::setupAccept, 0);

    lv_obj_add_style(_doneButton, Style::buttonColor, 0);
    lv_obj_add_style(_doneButton, Style::button, 0);
    lv_obj_add_style(_doneButton, Style::buttonFocusedColor, LV_STATE_FOCUSED);
    lv_obj_add_style(_doneButton, Style::buttonFocused, LV_STATE_FOCUSED);
    lv_obj_add_style(_doneButton, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_style(_doneButton, Style::buttonPressed, LV_STATE_PRESSED);
    lv_obj_add_style(_doneButton, Style::bgSelection, 0);
    lv_obj_add_style(_doneButton, Style::textareaFocusedColor, 0);
    lv_obj_add_style(_doneButton, Style::flatAction, 0);

    lv_obj_t *doneButtonCaption = lv_label_create(_doneButton);
    lv_obj_add_style(doneButtonCaption, Style::fontBody, 0);
    lv_obj_add_style(doneButtonCaption, Style::textAccent, 0);
    lv_label_set_long_mode(doneButtonCaption, LV_LABEL_LONG_DOT);
    lv_obj_add_style(doneButtonCaption, Style::buttonCaption, 0);

    lv_label_set_text(doneButtonCaption, "DONE");

    lv_obj_add_event_cb(_doneButton, [](lv_event_t *e) {
        auto* self = static_cast<LvNameInputScreen*>(lv_event_get_user_data(e));
        if (!self) return;
        self->submit(false); }, LV_EVENT_CLICKED, this);

    lv_obj_t *ver = lv_label_create(parent);
    lv_obj_add_style(ver, Style::versionFooter, 0);
    lv_obj_add_style(ver, Style::fontCaption, 0);
    lv_obj_add_style(ver, Style::textMuted, 0);

    lv_label_set_long_mode(ver, LV_LABEL_LONG_DOT);
    lv_label_set_text(ver, "");

    char verBuf[32];
    snprintf(verBuf, sizeof(verBuf), "v%s", RSDECK_VERSION_STRING);
    lv_label_set_text(ver, verBuf);

    setSaving(_saving);
}

void LvNameInputScreen::setSaving(bool saving) {
    _saving = saving;
    if (!_screen)
        return; // Terminal completion may arrive after UIManager destroyed the widgets.
    if (_saveTitle)
        lv_label_set_text(_saveTitle, saving ? "SAVING NAME" : "DISPLAY NAME");
    if (_textarea) {
        if (saving)
            lv_obj_add_state(_textarea, LV_STATE_DISABLED);
        else
            lv_obj_clear_state(_textarea, LV_STATE_DISABLED);
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

void LvNameInputScreen::submit(bool enforceEnterGuard) {
    if (_saving)
        return;
    if (!_textarea)
        return;
    if (enforceEnterGuard && millis() - _enterTime < ENTER_GUARD_MS)
        return;

    const char *text = lv_textarea_get_text(_textarea);
    if (_doneCb) {
        _doneCb(String(text && strlen(text) > 0 ? text : ""));
    }
}

bool LvNameInputScreen::handleKey(const KeyEvent &event) {
    if (_saving)
        return true;
    if (!_textarea)
        return false;

    if (event.enter || event.character == '\n' || event.character == '\r') {
        submit(true);
        return true;
    }
    if (event.del || event.character == 8) {
        lv_textarea_del_char(_textarea);
        return true;
    }
    if (event.character >= 0x20 && event.character <= 0x7E) {
        char buf[2] = { event.character, 0 };
        lv_textarea_add_text(_textarea, buf);
        return true;
    }
    return true; // Consume all keys
}
