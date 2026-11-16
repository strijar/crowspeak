#include "LvDataCleanScreen.h"
#include "Style.h"
#include "config/Config.h"

namespace {

} // namespace

void LvDataCleanScreen::createUI(lv_obj_t *parent) {
    _screen = parent;
    _yesBox = nullptr;
    _noBox = nullptr;
    _yesLabel = nullptr;
    _noLabel = nullptr;
    _hintLabel = nullptr;
    _confirmLabel = nullptr;
    _statusLabel = nullptr;
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    {
        lv_obj_remove_style(parent, Style::listColor, 0);
        lv_obj_add_style(parent, Style::listColor, 0);
    }
    {
        lv_obj_remove_style(parent, Style::opaque, 0);
        lv_obj_add_style(parent, Style::opaque, 0);
    }
    _selectedYes = false;
    _confirmWipe = false;

    lv_obj_t *eyebrow = lv_label_create(parent);
    lv_obj_add_style(eyebrow, Style::setupEyebrow, 0);
    lv_obj_add_style(eyebrow, Style::fontCaption, 0);
    lv_obj_add_style(eyebrow, Style::textSecondary, 0);

    lv_label_set_long_mode(eyebrow, LV_LABEL_LONG_DOT);
    lv_label_set_text(eyebrow, "STORAGE CHECK");

    lv_obj_t *title = lv_label_create(parent);
    lv_obj_add_style(title, Style::storageTitle, 0);
    lv_obj_add_style(title, Style::fontHeading, 0);
    lv_obj_add_style(title, Style::textAccent, 0);

    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_label_set_text(title, "SD DATA FOUND");

    lv_obj_t *msg = lv_label_create(parent);
    lv_obj_add_style(msg, Style::storageDescription, 0);
    lv_obj_add_style(msg, Style::fontBody, 0);
    lv_obj_add_style(msg, Style::textPrimary, 0);

    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_label_set_text(msg, DEVICE_NAME " found older data on the SD card.");

    lv_obj_t *prompt = lv_label_create(parent);
    lv_obj_add_style(prompt, Style::storagePrompt, 0);
    lv_obj_add_style(prompt, Style::fontBody, 0);
    lv_obj_add_style(prompt, Style::textSecondary, 0);

    lv_label_set_long_mode(prompt, LV_LABEL_LONG_WRAP);
    lv_label_set_text(prompt, "Erase affects SD copies only. Imported device data is kept.");

    _noBox = lv_obj_create(parent);
    lv_obj_add_style(_noBox, Style::choiceButton, 0);
    lv_obj_add_style(_noBox, Style::choiceLeft, 0);

    lv_obj_add_style(_noBox, Style::bgElevated, 0);

    lv_obj_add_style(_noBox, Style::borderBorder, 0);

    lv_obj_clear_flag(_noBox, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(_noBox, LV_OBJ_FLAG_CLICKABLE);

    _noLabel = lv_label_create(_noBox);
    lv_obj_add_style(_noLabel, Style::buttonCaption, 0);
    lv_obj_add_style(_noLabel, Style::fontBody, 0);
    lv_obj_add_style(_noLabel, Style::textPrimary, 0);

    lv_label_set_long_mode(_noLabel, LV_LABEL_LONG_DOT);
    lv_label_set_text(_noLabel, "KEEP DATA");

    lv_obj_add_event_cb(_noBox, [](lv_event_t *e) {
        auto* self = static_cast<LvDataCleanScreen*>(lv_event_get_user_data(e));
        if (!self) return;
        self->_selectedYes = false;
        self->_confirmWipe = false;
        self->updateSelection();
        if (self->_doneCb) self->_doneCb(false); }, LV_EVENT_CLICKED, this);

    _yesBox = lv_obj_create(parent);
    lv_obj_add_style(_yesBox, Style::choiceButton, 0);
    lv_obj_add_style(_yesBox, Style::choiceRight, 0);

    lv_obj_add_style(_yesBox, Style::bgElevated, 0);

    lv_obj_add_style(_yesBox, Style::borderBorder, 0);

    lv_obj_clear_flag(_yesBox, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(_yesBox, LV_OBJ_FLAG_CLICKABLE);

    _yesLabel = lv_label_create(_yesBox);
    lv_obj_add_style(_yesLabel, Style::buttonCaption, 0);
    lv_obj_add_style(_yesLabel, Style::fontBody, 0);
    lv_obj_add_style(_yesLabel, Style::textPrimary, 0);

    lv_label_set_long_mode(_yesLabel, LV_LABEL_LONG_DOT);
    lv_label_set_text(_yesLabel, "ERASE");

    lv_obj_add_event_cb(_yesBox, [](lv_event_t *e) {
        auto* self = static_cast<LvDataCleanScreen*>(lv_event_get_user_data(e));
        if (!self) return;
        self->_selectedYes = true;
        if (!self->_confirmWipe) {
            self->_confirmWipe = true;
            self->updateSelection();
            return;
        }
        if (self->_doneCb) self->_doneCb(true); }, LV_EVENT_CLICKED, this);

    _confirmLabel = lv_label_create(parent);
    lv_obj_add_style(_confirmLabel, Style::confirmationHint, 0);
    lv_obj_add_style(_confirmLabel, Style::fontBody, 0);
    lv_obj_add_style(_confirmLabel, Style::textWarning, 0);

    lv_label_set_long_mode(_confirmLabel, LV_LABEL_LONG_WRAP);
    lv_label_set_text(_confirmLabel, "");

#if HAS_SCROLLWHEEL
    const char *inputHint = "Wheel chooses  Enter continues";
#else
    const char *inputHint = "Left/Right choose  Enter continues";
#endif

    _hintLabel = lv_label_create(parent);
    lv_obj_add_style(_hintLabel, Style::inputHint, 0);
    lv_obj_add_style(_hintLabel, Style::fontBody, 0);
    lv_obj_add_style(_hintLabel, Style::textAccent, 0);

    lv_label_set_long_mode(_hintLabel, LV_LABEL_LONG_DOT);
    lv_label_set_text(_hintLabel, inputHint);

    _statusLabel = lv_label_create(parent);
    lv_obj_add_style(_statusLabel, Style::operationStatus, 0);
    lv_obj_add_style(_statusLabel, Style::fontBody, 0);
    lv_obj_add_style(_statusLabel, Style::textActiveTabText, 0);

    lv_label_set_long_mode(_statusLabel, LV_LABEL_LONG_WRAP);
    lv_label_set_text(_statusLabel, "");

    lv_obj_add_flag(_statusLabel, LV_OBJ_FLAG_HIDDEN);
    updateSelection();

    lv_obj_t *ver = lv_label_create(parent);
    lv_obj_add_style(ver, Style::versionText, 0);
    lv_obj_add_style(ver, Style::fontCaption, 0);
    lv_obj_add_style(ver, Style::textMuted, 0);

    lv_label_set_long_mode(ver, LV_LABEL_LONG_DOT);
    lv_label_set_text(ver, "");

    char verBuf[32];
    snprintf(verBuf, sizeof(verBuf), "v%s", RSDECK_VERSION_STRING);
    lv_label_set_text(ver, verBuf);
    // Pager's shorter boot surface needs this footer below the action hint.
    lv_obj_add_style(ver, Style::versionFooter, 0);
}

void LvDataCleanScreen::updateSelection() {
    {
        if (_noBox) {
            uint32_t border = (!_selectedYes) ? (false && false ? Style::ERROR_CLR : Style::PRIMARY) : Style::BORDER;
            uint32_t bg = (!_selectedYes) ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED;
            if (false && false)
                bg = Style::ERROR_SUBTLE;
            {
                for (auto *tone : Style::bgTones)
                    lv_obj_remove_style(_noBox, tone, 0);
                lv_obj_add_style(_noBox, Style::bgTones[bg], 0);
            }
            {
                for (auto *tone : Style::borderTones)
                    lv_obj_remove_style(_noBox, tone, 0);
                lv_obj_add_style(_noBox, Style::borderTones[border], 0);
            }
            {
                lv_obj_remove_style(_noBox, Style::buttonPressed, 0);
                lv_obj_remove_style(_noBox, Style::borderStrong, 0);
                {
                    lv_obj_remove_style(_noBox, (!_selectedYes) ? Style::borderStrong : Style::buttonPressed, 0);
                    lv_obj_add_style(_noBox, (!_selectedYes) ? Style::borderStrong : Style::buttonPressed, 0);
                }
            }
        }
    }
    {
        if (_yesBox) {
            uint32_t border = _selectedYes ? (true && _confirmWipe ? Style::ERROR_CLR : Style::PRIMARY) : Style::BORDER;
            uint32_t bg = _selectedYes ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED;
            if (true && _confirmWipe)
                bg = Style::ERROR_SUBTLE;
            {
                for (auto *tone : Style::bgTones)
                    lv_obj_remove_style(_yesBox, tone, 0);
                lv_obj_add_style(_yesBox, Style::bgTones[bg], 0);
            }
            {
                for (auto *tone : Style::borderTones)
                    lv_obj_remove_style(_yesBox, tone, 0);
                lv_obj_add_style(_yesBox, Style::borderTones[border], 0);
            }
            {
                lv_obj_remove_style(_yesBox, Style::buttonPressed, 0);
                lv_obj_remove_style(_yesBox, Style::borderStrong, 0);
                {
                    lv_obj_remove_style(_yesBox, _selectedYes ? Style::borderStrong : Style::buttonPressed, 0);
                    lv_obj_add_style(_yesBox, _selectedYes ? Style::borderStrong : Style::buttonPressed, 0);
                }
            }
        }
    }

    if (_selectedYes) {
        {
            for (auto *tone : Style::textTones)
                lv_obj_remove_style(_yesLabel, tone, 0);
            lv_obj_add_style(_yesLabel, Style::textTones[_confirmWipe ? Style::ERROR_CLR : Style::WARNING_CLR], 0);
        }
        {
            lv_obj_remove_style(_noLabel, Style::textMuted, 0);
            lv_obj_add_style(_noLabel, Style::textMuted, 0);
        }
        if (_confirmLabel) {
            lv_label_set_text(_confirmLabel, _confirmWipe ? "Erase armed. Press Enter again to wipe." : "Erase will remove old SD data.");
        }
        if (_hintLabel) {
            lv_label_set_text(_hintLabel,
#if HAS_SCROLLWHEEL
                              _confirmWipe ? "Enter: erase  Wheel up: cancel" : "Enter: arm erase  Wheel up: cancel");
#else
                              _confirmWipe ? "Enter confirms erase  Left cancels" : "Enter arms erase  Left cancels");
#endif
        }
    } else {
        {
            lv_obj_remove_style(_yesLabel, Style::textMuted, 0);
            lv_obj_add_style(_yesLabel, Style::textMuted, 0);
        }
        {
            lv_obj_remove_style(_noLabel, Style::textAccent, 0);
            lv_obj_add_style(_noLabel, Style::textAccent, 0);
        }
        if (_confirmLabel)
            lv_label_set_text(_confirmLabel, "Recommended: keep existing data.");
        if (_hintLabel)
            lv_label_set_text(_hintLabel,
#if HAS_SCROLLWHEEL
                              "Enter: keep  Wheel down: erase");
#else
                              "Enter keeps data  Right selects erase");
#endif
    }
}

void LvDataCleanScreen::showStatus(const char *msg) {
    if (_yesBox)
        lv_obj_add_flag(_yesBox, LV_OBJ_FLAG_HIDDEN);
    if (_noBox)
        lv_obj_add_flag(_noBox, LV_OBJ_FLAG_HIDDEN);
    if (_yesLabel)
        lv_obj_add_flag(_yesLabel, LV_OBJ_FLAG_HIDDEN);
    if (_noLabel)
        lv_obj_add_flag(_noLabel, LV_OBJ_FLAG_HIDDEN);
    if (_hintLabel)
        lv_obj_add_flag(_hintLabel, LV_OBJ_FLAG_HIDDEN);
    if (_confirmLabel)
        lv_obj_add_flag(_confirmLabel, LV_OBJ_FLAG_HIDDEN);
    if (_statusLabel) {
        lv_label_set_text(_statusLabel, msg ? msg : "");
        lv_obj_clear_flag(_statusLabel, LV_OBJ_FLAG_HIDDEN);
    }
    lv_timer_handler();
}

bool LvDataCleanScreen::handleKey(const KeyEvent &event) {
    if (event.left || event.up) {
        _selectedYes = false;
        _confirmWipe = false;
        updateSelection();
        return true;
    }
    if (event.right || event.down) {
        _selectedYes = true;
        _confirmWipe = false;
        updateSelection();
        return true;
    }
    if (event.enter || event.character == '\n' || event.character == '\r') {
        if (_selectedYes && !_confirmWipe) {
            _confirmWipe = true;
            updateSelection();
            return true;
        }
        if (_doneCb)
            _doneCb(_selectedYes);
        return true;
    }
    return true; // Consume all keys
}
