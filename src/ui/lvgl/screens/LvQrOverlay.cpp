#include "LvQrOverlay.h"
#include "Style.h"
#include "LvInput.h"
#include "util/ContactCard.h"

void LvQrOverlay::create() {
    if (_overlay)
        return;
    _group = lv_group_create();
    _overlay = lv_obj_create(lv_layer_top());
    lv_obj_add_style(_overlay, Style::screenOverlay, 0);

    lv_obj_add_style(_overlay, Style::theme, 0);
    lv_obj_add_style(_overlay, Style::screen, 0);
    lv_obj_add_style(_overlay, Style::plainPanel, 0);

    lv_obj_clear_flag(_overlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(_overlay);
    lv_obj_add_style(title, Style::textAccent, 0);
    lv_obj_add_style(title, Style::fontTitle, 0);
    lv_obj_add_style(title, Style::fontTitle, 0);
    lv_label_set_text(title, "Scan to add me");
    lv_obj_add_style(title, Style::codeHeading, 0);

    // Landscape layout leaves room for real controls on both release screens.
    // Keep at least four white modules around the unbranded QR code.
    lv_style_value_t codeWidth;
    lv_style_get_prop(Style::contactCode, LV_STYLE_WIDTH, &codeWidth);
    const int qrSize = codeWidth.num;
    auto     *paper = lv_obj_create(_overlay);
    lv_obj_add_style(paper, Style::codePaper, 0);

    lv_obj_add_style(paper, Style::bgWhite, 0);

    lv_obj_clear_flag(paper, LV_OBJ_FLAG_SCROLLABLE);
    _qr = lv_qrcode_create(paper, qrSize, lv_color_hex(0), lv_color_hex(0xFFFFFF));
    _qrSize = qrSize;
    lv_obj_add_style(_qr, Style::centered, 0);

    _lblFormat = lv_label_create(_overlay);
    lv_obj_add_style(_lblFormat, Style::textAccent, 0);
    lv_obj_add_style(_lblFormat, Style::fontTitle, 0);
    lv_obj_add_style(_lblFormat, Style::fontBody, 0);
    lv_obj_add_style(_lblFormat, Style::codeFormat, 0);
    _lblAddr = lv_label_create(_overlay);
    lv_obj_add_style(_lblAddr, Style::textSecondary, 0);
    lv_obj_add_style(_lblAddr, Style::fontBody, 0);
    lv_obj_add_style(_lblAddr, Style::fontCaption, 0);
    lv_obj_add_style(_lblAddr, Style::codeAddress, 0);

    _toggle = lv_btn_create(_overlay);
    lv_obj_add_style(_toggle, Style::codeToggle, 0);

    lv_obj_add_style(_toggle, Style::buttonColor, 0);
    lv_obj_add_style(_toggle, Style::button, 0);
    lv_obj_add_style(_toggle, Style::buttonFocusedColor, LV_STATE_FOCUSED);
    lv_obj_add_style(_toggle, Style::buttonFocused, LV_STATE_FOCUSED);
    lv_obj_add_style(_toggle, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_style(_toggle, Style::buttonPressed, LV_STATE_PRESSED);
    lv_group_add_obj(_group, _toggle);
    auto *toggleCaption = lv_label_create(_toggle);
    lv_obj_add_style(toggleCaption, Style::fontBody, 0);
    lv_label_set_text(toggleCaption, "Legacy");
    lv_obj_add_style(toggleCaption, Style::centered, 0);
    if (&_lblToggle)
        _lblToggle = toggleCaption;

    lv_obj_add_event_cb(_toggle, [](lv_event_t *e) { static_cast<LvQrOverlay *>(lv_event_get_user_data(e))->toggleFormat(); }, LV_EVENT_CLICKED, this);

    auto *close = lv_btn_create(_overlay);
    lv_obj_add_style(close, Style::codeClose, 0);

    lv_obj_add_style(close, Style::buttonColor, 0);
    lv_obj_add_style(close, Style::button, 0);
    lv_obj_add_style(close, Style::buttonFocusedColor, LV_STATE_FOCUSED);
    lv_obj_add_style(close, Style::buttonFocused, LV_STATE_FOCUSED);
    lv_obj_add_style(close, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_style(close, Style::buttonPressed, LV_STATE_PRESSED);
    lv_group_add_obj(_group, close);
    auto *closeCaption = lv_label_create(close);
    lv_obj_add_style(closeCaption, Style::fontBody, 0);
    lv_label_set_text(closeCaption, "Close");
    lv_obj_add_style(closeCaption, Style::centered, 0);

    lv_obj_add_event_cb(close, [](lv_event_t *e) { static_cast<LvQrOverlay *>(lv_event_get_user_data(e))->hide(); }, LV_EVENT_CLICKED, this);
    lv_obj_add_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
}

bool LvQrOverlay::renderCode(bool legacy) {
    const auto &payload = legacy ? _legacyPayload : _payload;
    // Address-only codes have larger modules. Give them a wider white quiet
    // zone without shrinking the denser rich contact card.
    lv_style_value_t codeWidth;
    lv_style_get_prop(legacy ? Style::legacyContactCode : Style::contactCode, LV_STYLE_WIDTH, &codeWidth);
    const int size = codeWidth.num;
    if (_qrSize != size) {
        auto *paper = lv_obj_get_parent(_qr);
        lv_qrcode_delete(_qr);
        _qr = lv_qrcode_create(paper, size, lv_color_hex(0), lv_color_hex(0xFFFFFF));
        _qrSize = size;
        {
            lv_obj_remove_style(_qr, Style::centered, 0);
            lv_obj_add_style(_qr, Style::centered, 0);
        }
    }
    if (lv_qrcode_update(_qr, payload.data(), payload.size()) != LV_RES_OK)
        return false;
    _legacy = legacy;
    lv_label_set_text(_lblFormat, legacy ? "Legacy" : "Ratspeak");
    lv_label_set_text(_lblToggle, legacy ? "Ratspeak" : "Legacy");
    return true;
}

bool LvQrOverlay::show(const String &name, const String &destHashHex, const String &identityHashHex, const String &publicKeyHex) {
    hide();
    if (!_overlay)
        create();
    _payload = rs::contactCardPayload(name.c_str(), destHashHex.c_str(), identityHashHex.c_str(), publicKeyHex.c_str());
    if (_payload.empty())
        return false;
    _legacyPayload = "lxma://" + std::string(destHashHex.c_str());
    if (!renderCode(false))
        return false;
    lv_label_set_text_fmt(_lblAddr, "%.8s\n%.8s\n%.8s\n%.8s", destHashHex.c_str(), destHashHex.c_str() + 8, destHashHex.c_str() + 16, destHashHex.c_str() + 24);
    lv_obj_clear_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(_overlay);
    // Preserve the underlying screen's group and focus while wheel/keypad
    // events belong exclusively to this overlay.
    for (auto *input = lv_indev_get_next(nullptr); input; input = lv_indev_get_next(input)) {
        if (input->group == LvInput::group())
            lv_indev_set_group(input, _group);
    }
    lv_group_set_editing(_group, false);
    lv_group_focus_obj(_toggle);
    _visible = true;
    return true;
}

void LvQrOverlay::hide() {
    if (_overlay)
        lv_obj_add_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
    if (_group) {
        for (auto *input = lv_indev_get_next(nullptr); input; input = lv_indev_get_next(input)) {
            if (input->group == _group)
                lv_indev_set_group(input, LvInput::group());
        }
    }
    _visible = false;
}

void LvQrOverlay::toggleFormat() {
    if (!renderCode(!_legacy))
        hide();
}

bool LvQrOverlay::handleKey(const KeyEvent &event) {
    if (!_visible)
        return false;
    if (event.repeat)
        return true;
    if (event.up || event.left)
        lv_group_focus_prev(_group);
    else if (event.down || event.right)
        lv_group_focus_next(_group);
    else if (event.enter || event.character == '\n' || event.character == '\r') {
        auto *focused = lv_group_get_focused(_group);
        if (focused)
            lv_event_send(focused, LV_EVENT_CLICKED, nullptr);
    } else if (event.character == 'l' || event.character == 'L')
        toggleFormat();
    else
        hide();
    return true;
}
