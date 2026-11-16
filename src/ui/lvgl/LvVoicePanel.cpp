#include "LvVoicePanel.h"
#include "runtime/ServiceClient.h"
#include "storage/Hex.h"
#include "Style.h"
using namespace handheld::memo;
void LvVoicePanel::begin(handheld::ServiceClient *service) {
    _service = service;
    _model.begin(this, [](void *context, const Command &command, uint32_t serial) {
        auto &self = *static_cast<LvVoicePanel *>(context);
        return self._service && self._service->memoCommand(command, [&self, serial](const handheld::Result &result) {
            self._model.acknowledge(serial, result.outcome == handheld::Outcome::Ok ? Code::Ok : result.next ? Code(result.next)
                                                                                                             : Code::Busy);
        }) != 0;
    });
}
void LvVoicePanel::start(const char *hex) {
    startMessage(hex, 0, false);
}
void LvVoicePanel::startMessage(const char *hex, uint32_t counter, bool incoming) {
    if (!_service || !hex)
        return;
    uint8_t peer[16];
    if (!handheld::storage::decodeHex(hex, strlen(hex), peer, 16))
        return;
    _model.open(peer, counter, incoming);
    build();
    render();
}
void LvVoicePanel::show() {
    _model.show();
    build();
    render();
}
void LvVoicePanel::hide() {
    _model.hide();
    _pressed = Ui::Choice::None;
    if (_root) {
        lv_obj_del(_root);
        _root = nullptr;
    }
}
void LvVoicePanel::closeConversation() {
    _model.closeConversation();
    _pressed = Ui::Choice::None;
    if (_root) {
        lv_obj_del(_root);
        _root = nullptr;
    }
}
void LvVoicePanel::action(Ui::Choice choice) {
    _pressed = Ui::Choice::None;
    _model.choose(choice);
    render();
}
void LvVoicePanel::build() {
    if (_root || !visible())
        return;
    _root = lv_obj_create(lv_layer_top());
    lv_obj_add_style(_root, Style::fullscreen, 0);
    lv_obj_add_style(_root, Style::modalColor, 0);
    lv_obj_add_style(_root, Style::modal, 0);
    lv_obj_add_style(_root, Style::modalPadding, 0);
    lv_obj_clear_flag(_root, LV_OBJ_FLAG_SCROLLABLE);

    _title = lv_label_create(_root);
    lv_obj_add_style(_title, Style::voiceTitle, 0);
    lv_obj_add_style(_title, Style::fontTitle, 0);
    lv_obj_add_style(_title, Style::textPrimary, 0);
    lv_label_set_long_mode(_title, LV_LABEL_LONG_DOT);
    lv_label_set_text(_title, "Voice message");

    _peer = lv_label_create(_root);
    lv_obj_add_style(_peer, Style::voicePeer, 0);
    lv_obj_add_style(_peer, Style::fontTitle, 0);
    lv_obj_add_style(_peer, Style::textPrimary, 0);
    lv_label_set_long_mode(_peer, LV_LABEL_LONG_DOT);
    lv_label_set_text(_peer, "");

    _label = lv_label_create(_root);
    lv_obj_add_style(_label, Style::voiceStatus, 0);
    lv_obj_add_style(_label, Style::fontTitle, 0);
    lv_obj_add_style(_label, Style::textPrimary, 0);
    lv_label_set_long_mode(_label, LV_LABEL_LONG_DOT);
    lv_label_set_text(_label, "");

    _detail = lv_label_create(_root);
    lv_obj_add_style(_detail, Style::voiceDetail, 0);
    lv_obj_add_style(_detail, Style::fontTitle, 0);
    lv_obj_add_style(_detail, Style::textPrimary, 0);
    lv_label_set_long_mode(_detail, LV_LABEL_LONG_DOT);
    lv_label_set_text(_detail, "");

    _hint = lv_label_create(_root);
    lv_obj_add_style(_hint, Style::voiceHint, 0);
    lv_obj_add_style(_hint, Style::fontTitle, 0);
    lv_obj_add_style(_hint, Style::textPrimary, 0);
    lv_label_set_long_mode(_hint, LV_LABEL_LONG_DOT);
    lv_label_set_text(_hint, "");

    lv_obj_add_style(_peer, Style::textAccent, 0);
    for (auto *l : { _detail, _hint }) {
        {
            lv_obj_remove_style(l, Style::fontBody, 0);
            lv_obj_add_style(l, Style::fontBody, 0);
        }
        {
            lv_obj_remove_style(l, Style::textSecondary, 0);
            lv_obj_add_style(l, Style::textSecondary, 0);
        }
    }

    _back = lv_btn_create(_root);
    lv_obj_add_style(_back, Style::buttonColor, 0);
    lv_obj_add_style(_back, Style::button, 0);
    lv_obj_add_style(_back, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_style(_back, Style::buttonPressed, LV_STATE_PRESSED);
    lv_obj_add_style(_back, Style::voiceBack, 0);
    auto *backCaption = lv_label_create(_back);
    lv_label_set_text(backCaption, "Back");
    lv_obj_add_style(backCaption, Style::fontTitle, 0);
    lv_obj_add_style(backCaption, Style::centered, 0);

    lv_obj_add_event_cb(_back, [](lv_event_t *e) {auto& self=*static_cast<LvVoicePanel*>(lv_event_get_user_data(e));self._pressed=Ui::Choice::None;self._model.back();self.render(); }, LV_EVENT_CLICKED, this);
    for (unsigned i = 0; i < 3; ++i) {
        _buttons[i] = lv_btn_create(_root);
        lv_obj_add_style(_buttons[i], Style::buttonColor, 0);
        lv_obj_add_style(_buttons[i], Style::button, 0);
        lv_obj_add_style(_buttons[i], Style::buttonPressedColor, LV_STATE_PRESSED);
        lv_obj_add_style(_buttons[i], Style::buttonPressed, LV_STATE_PRESSED);
        auto *l2 = lv_label_create(_buttons[i]);
        lv_label_set_text(l2, "");
        lv_obj_add_style(l2, Style::fontTitle, 0);
        lv_obj_add_style(l2, Style::centered, 0);

        lv_obj_add_event_cb(_buttons[i], [](lv_event_t *e) {
            auto& self=*static_cast<LvVoicePanel*>(lv_event_get_user_data(e));
            unsigned index=0;while(index<3 && self._buttons[index]!=lv_event_get_target(e)) ++index;
            if(index==3) return;
            const auto code=lv_event_get_code(e);
            if(code==LV_EVENT_PRESSED) {self._pressed=self._model.choice(index);self._pressedLayout=self._model.layout();}
            else if(code==LV_EVENT_PRESS_LOST || code==LV_EVENT_DELETE) self._pressed=Ui::Choice::None;
            else if(code==LV_EVENT_CLICKED) {
                const auto choice=self._pressed;self._pressed=Ui::Choice::None;
                if(choice!=Ui::Choice::None && self._pressedLayout==self._model.layout() && choice==self._model.choice(index)) self.action(choice);
            } }, LV_EVENT_ALL, this);
    }

    _quieter = lv_btn_create(_root);
    lv_obj_add_style(_quieter, Style::buttonColor, 0);
    lv_obj_add_style(_quieter, Style::button, 0);
    lv_obj_add_style(_quieter, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_style(_quieter, Style::buttonPressed, LV_STATE_PRESSED);
    lv_obj_add_style(_quieter, Style::volumeDecrease, 0);
    auto *quieterCaption = lv_label_create(_quieter);
    lv_label_set_text(quieterCaption, "-");
    lv_obj_add_style(quieterCaption, Style::fontTitle, 0);
    lv_obj_add_style(quieterCaption, Style::centered, 0);

    _louder = lv_btn_create(_root);
    lv_obj_add_style(_louder, Style::buttonColor, 0);
    lv_obj_add_style(_louder, Style::button, 0);
    lv_obj_add_style(_louder, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_style(_louder, Style::buttonPressed, LV_STATE_PRESSED);
    lv_obj_add_style(_louder, Style::volumeIncrease, 0);
    auto *louderCaption = lv_label_create(_louder);
    lv_label_set_text(louderCaption, "+");
    lv_obj_add_style(louderCaption, Style::fontTitle, 0);
    lv_obj_add_style(louderCaption, Style::centered, 0);

    lv_obj_add_event_cb(_quieter, [](lv_event_t *e) {auto& self=*static_cast<LvVoicePanel*>(lv_event_get_user_data(e));self._model.volume(-10);self.render(); }, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(_louder, [](lv_event_t *e) {auto& self=*static_cast<LvVoicePanel*>(lv_event_get_user_data(e));self._model.volume(10);self.render(); }, LV_EVENT_CLICKED, this);
    lv_obj_add_style(_detail, Style::voiceDetailWidth, 0);
}
void LvVoicePanel::render() {
    if (!_root)
        return;
    const auto &s = _model.status();
    {
        lv_obj_remove_style(_title, Style::textPrimary, 0);
        lv_obj_add_style(_title, Style::textPrimary, 0);
    }
    {
        lv_obj_remove_style(_peer, Style::textAccent, 0);
        lv_obj_add_style(_peer, Style::textAccent, 0);
    }
    for (auto *label : { _detail, _hint }) {
        lv_obj_remove_style(label, Style::textSecondary, 0);
        lv_obj_add_style(label, Style::textSecondary, 0);
    }
    char peer[33];
    handheld::storage::encodeHex(_model.peer(), 16, peer);
    const auto name = _service ? _service->nodes.lookupName(peer) : std::string();
    lv_label_set_text(_peer, name.empty() ? peer : name.c_str());
    lv_label_set_text(_label, _model.text());
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(_label, tone, 0);
        lv_obj_add_style(_label, Style::textTones[s.phase == Phase::Recording ? Style::ERROR_CLR : Style::TEXT_PRIMARY], 0);
    }
    const auto ms = (s.phase == Phase::Recording || s.phase == Phase::Playing || s.phase == Phase::Stopping) ? s.frames * 40 : uint32_t(s.length) * 10;
    const bool running = s.phase == Phase::Recording || s.phase == Phase::Playing || s.phase == Phase::Stopping;
    char       info[64];
    snprintf(info, sizeof info, "0:%02lu%s  Volume %u%%", (unsigned long) ((ms + (running ? 0 : 999)) / 1000), s.phase == Phase::Recording ? " / 0:15" : "", s.volume);
    if (s.reason == Code::UnsupportedAudio || s.reason == Code::AudioUnavailable)
        snprintf(info, sizeof info, "Volume %u%%", s.volume);
    lv_label_set_text(_detail, info);
    lv_label_set_text(_hint, _model.guidance());
    auto       shown = [](lv_obj_t *object, bool show) {if(show) lv_obj_clear_flag(object,LV_OBJ_FLAG_HIDDEN);else lv_obj_add_flag(object,LV_OBJ_FLAG_HIDDEN); };
    bool       backInActions = false;
    const bool confirmation = _model.choice(0) == Ui::Choice::ConfirmReplace || _model.choice(0) == Ui::Choice::ConfirmDiscard;
    for (unsigned i = 0; i < 3; ++i) {
        const auto choice = _model.choice(i);
        shown(_buttons[i], choice != Ui::Choice::None);
        // Confirmations are one equal row, Delete left and Cancel right.
        for (auto *layout : { Style::primaryAction, Style::secondaryActionLeft, Style::secondaryActionRight, Style::confirmActionLeft, Style::confirmActionRight })
            lv_obj_remove_style(_buttons[i], layout, 0);
        {
            lv_obj_remove_style(_buttons[i], confirmation ? (i ? Style::confirmActionRight : Style::confirmActionLeft) : (!i ? Style::primaryAction : i == 1 ? Style::secondaryActionLeft
                                                                                                                                                             : Style::secondaryActionRight),
                                0);
            lv_obj_add_style(_buttons[i], confirmation ? (i ? Style::confirmActionRight : Style::confirmActionLeft) : (!i ? Style::primaryAction : i == 1 ? Style::secondaryActionLeft
                                                                                                                                                          : Style::secondaryActionRight),
                             0);
        }

        backInActions |= choice == Ui::Choice::Back;
        lv_label_set_text(lv_obj_get_child(_buttons[i], 0), Ui::label(choice));
        const bool selected = i == _model.focus();
        const bool destructive = choice == Ui::Choice::Stop || choice == Ui::Choice::ConfirmDiscard || choice == Ui::Choice::ConfirmReplace;
        const auto color = destructive ? Style::ERROR_CLR : Style::PRIMARY;
        {
            for (auto *tone : Style::textTones)
                lv_obj_remove_style(lv_obj_get_child(_buttons[i], 0), tone, 0);
            lv_obj_add_style(lv_obj_get_child(_buttons[i], 0), Style::textTones[destructive ? Style::ERROR_CLR : Style::TEXT_PRIMARY], 0);
        }
        {
            for (auto *tone : Style::borderTones)
                lv_obj_remove_style(_buttons[i], tone, 0);
            lv_obj_add_style(_buttons[i], Style::borderTones[selected ? color : Style::BORDER], 0);
        }
        {
            for (auto *tone : Style::bgTones)
                lv_obj_remove_style(_buttons[i], tone, 0);
            lv_obj_add_style(_buttons[i], Style::bgTones[selected ? Style::PRIMARY_SUBTLE : Style::BG_SURFACE], 0);
        }
    }
    shown(_back, !backInActions && _model.count());
    shown(_quieter, (s.capabilities & 2) && _model.count());
    shown(_louder, (s.capabilities & 2) && _model.count());
    if (s.volume == 0)
        lv_obj_add_state(_quieter, LV_STATE_DISABLED);
    else
        lv_obj_clear_state(_quieter, LV_STATE_DISABLED);
    if (s.volume == 100)
        lv_obj_add_state(_louder, LV_STATE_DISABLED);
    else
        lv_obj_clear_state(_louder, LV_STATE_DISABLED);
}
void LvVoicePanel::poll(bool allowed) {
    if (!_service)
        return;
    _model.update(_service->status().memo, allowed);
    if (visible()) {
        build();
        if (millis() - _painted >= 100) {
            _painted = millis();
            render();
        }
    } else if (_root) {
        lv_obj_del(_root);
        _root = nullptr;
    }
}
bool LvVoicePanel::handleKey(const KeyEvent &event) {
    if (!visible())
        return false;
    if (event.repeat)
        return true;
    // Mixing touch with keyboard navigation must require a fresh touch press,
    // even if the user later returns to the same confirmation/layout.
    _pressed = Ui::Choice::None;
    if (event.character == 27 || event.del)
        _model.back();
    else if (event.left || event.up)
        _model.move(-1);
    else if (event.right || event.down || event.tab)
        _model.move(1);
    else if (event.enter)
        _model.choose(_model.choice(_model.focus()));
    else if (event.character == '+')
        _model.volume(10);
    else if (event.character == '-')
        _model.volume(-10);
    render();
    return true;
}
