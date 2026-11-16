#include "LvMessageView.h"
#include "input/Utf8.h"
#include "ClockText.h"
#include "ui/FeatureAvailability.h"
#include "ui/MessageAudio.h"
#include "storage/Hex.h"
#include <new>
#include "reticulum/MessageStatusDetail.h"
#include "Style.h"
#include "LvTabBar.h"
#include "util/PerfTrace.h"
#include <Arduino.h>
#include <time.h>
#include <cmath>
#include "ui/RrcCompose.h"
#include "history/RrcHistory.h"

namespace {

constexpr const char *kComposerPlaceholder = "Message...";

bool isPendingStatus(LXMFStatus status) {
    return handheld::messaging::pendingStatus(uint8_t(status));
}

uint32_t bubbleBorderColor(LXMFStatus status) {
    if (handheld::messaging::failedStatus(uint8_t(status)))
        return Style::ERROR_CLR;
    if (status == LXMFStatus::UNCONFIRMED || status == LXMFStatus::PROP_UNCONFIRMED)
        return Style::WARNING_CLR;
    if (isPendingStatus(status))
        return Style::WARNING_CLR;
    if (status == LXMFStatus::DELIVERED)
        return Style::PRIMARY_MUTED;
    return Style::BORDER;
}

bool formatClock(double ts, bool use24h, char *out, size_t outLen) {
    if (!out || outLen == 0 || ts <= 1700000000)
        return false;
    time_t     t = (time_t) ts;
    struct tm  converted;
    struct tm *tm = localtime_r(&t, &converted);
    if (!tm)
        return false;
    formatClockText(out, outLen, *tm, use24h);
    return true;
}

} // namespace

void LvMessageView::updateHeader() {
    if (!_lblHeader)
        return;
    if (_rrcMode) {
        lv_label_set_text(_lblHeader, _rrcBinding.room[0] ? _rrcBinding.room : _rrcBinding.privateNotice() ? "Private hub notice"
                                                                                                           : "Hub notices");
        const auto &live = _service->status().rrc;
        const bool  current = !memcmp(live.hub, _rrcBinding.hub, 16);
        const auto  status = current ? live : handheld::rrc::Status {};
        const auto  action = _rrcBinding.privateNotice() ? handheld::rrc::Action::PrivateNotice : _rrcEmote ? handheld::rrc::Action::Emote
                                                                                                            : handheld::rrc::Action::Message;
        const auto  limit = handheld::ui::rrc_input::preview(status, _rrcBinding, action, nullptr, 0);
        char        text[96];
        char        hub[33];
        snprintf(hub, sizeof hub, "Hub %02x%02x%02x", _rrcBinding.hub[0], _rrcBinding.hub[1], _rrcBinding.hub[2]);
        const auto *name = current && status.name[0] ? status.name : hub;
        if (rrcWritable())
            snprintf(text, sizeof text, "%s · %s · %u/%u bytes", name, handheld::rrc::phaseName(status.phase), unsigned(_inputText.size()), limit.maximum);
        else
            snprintf(text, sizeof text, "%s · %s", name, handheld::rrc::phaseName(status.phase));
        if (_lblHeaderMeta)
            lv_label_set_text(_lblHeaderMeta, text);
        return;
    }
    if (_lblHeaderMeta && strcmp(lv_label_get_text(_lblHeaderMeta), _peerHex.c_str())) {
        lv_label_set_text(_lblHeaderMeta, _peerHex.c_str());
    }
    const auto appliedNodes = _am ? _am->revision() : (_service ? _service->status().nodeRevision : 0);
    if (_service && (_nameNodeRevision != appliedNodes ||
                     _nameIdentity != _service->status().generation)) {
        _nameNodeRevision = appliedNodes;
        _nameIdentity = _service->status().generation;
        _nameResolved = false;
    }
    const auto liveName = _am ? _am->lookupName(_peerHex) : std::string();
    if (!liveName.empty()) {
        if (!_nameResolved)
            lv_label_set_text(_lblHeader, liveName.c_str());
        _nameResolved = true;
        return;
    }
    if (_nameResolved)
        return;
    const auto fallback = _peerHex.substr(0, 12);
    if (strcmp(lv_label_get_text(_lblHeader), fallback.c_str()))
        lv_label_set_text(_lblHeader, fallback.c_str());
    if (!_service || !_entered || _nameInFlight || !windowMatches() || !_service->available())
        return;
    std::array<uint8_t, 16> peer;
    memcpy(peer.data(), _service->historyWindow().peer(), peer.size());
    const auto identity = _nameIdentity, nodes = _nameNodeRevision;
    const auto backendNodes = _service->status().nodeRevision;
    _nameInFlight = true;
    const auto id = _service->requestPeerName(_peerHex, [this, peer, identity, nodes, backendNodes](const handheld::Result &result, const char *name) {
        _nameInFlight = false;
        if (!_entered || !_screen || !_lblHeader || !_service || !windowMatches() ||
            _service->status().generation != identity || _service->status().nodeRevision != backendNodes ||
            (_am ? _am->revision() : _service->status().nodeRevision) != nodes ||
            memcmp(_service->historyWindow().peer(), peer.data(), peer.size()))
            return;
        _nameResolved = true; // Failure keeps the hash; explicit Refresh retries.
        if (result.outcome == handheld::Outcome::Ok && result.length)
            lv_label_set_text(_lblHeader, name);
    });
    if (!id) {
        _nameInFlight = false;
        _nameResolved = true;
    }
}

void LvMessageView::markVisibleConversationRead() {
    if (!_markReadPending || _readInFlight || !_service || !_entered || !_screen)
        return;
    const auto now = uint32_t(millis());
    if (_readRetry && uint32_t(now - _readRetryStarted) < 1000)
        return;
    if (!_service->available())
        return;
    if (_rrcMode) {
        if (!_readThrough || !windowMatches())
            return;
        const auto view = _rrcView;
        auto       command = rrcCommand(handheld::rrc::Action::MarkRead);
        command.counter = _readThrough;
        _markReadPending = false;
        _readInFlight = true;
        const auto id = _service->rrcCommand(command, nullptr, 0, [this, view](const handheld::Result &result) {
            _readInFlight = false;
            if (_rrcView == view && result.outcome != handheld::Outcome::Ok) {
                _markReadPending = true;
                _readRetry = true;
                _readRetryStarted = millis();
            }
        });
        if (!id) {
            _readInFlight = false;
            _markReadPending = true;
            _readRetry = true;
            _readRetryStarted = now;
        }
        return;
    }
    std::array<uint8_t, 16> peer;
    memcpy(peer.data(), _service->historyWindow().peer(), peer.size());
    const auto identity = _service->status().generation;
    _markReadPending = false;
    _readInFlight = true;
    _readRetry = false;
    const auto id = _service->action(handheld::Operation::MarkRead, _peerHex, "", 0, [this, peer, identity](const handheld::Result &result) {
        // ServiceClient owns this exact request until its terminal result.
        // Hidden/other-peer completions retire it without touching widgets
        // or clearing a newer conversation's pending read intent.
        _readInFlight = false;
        _readRequest = 0;
        const bool sameView = _entered && _service && windowMatches() &&
                              _service->status().generation == identity &&
                              !memcmp(_service->historyWindow().peer(), peer.data(), peer.size());
        if (sameView && result.outcome != handheld::Outcome::Ok &&
            result.outcome != handheld::Outcome::Cancelled &&
            result.outcome != handheld::Outcome::Stale &&
            result.outcome != handheld::Outcome::Invalid) {
            _markReadPending = true;
            _readRetry = true;
            _readRetryStarted = uint32_t(millis());
        }
    });
    if (!id) {
        _readInFlight = false;
        _markReadPending = true;
        _readRetry = true;
        _readRetryStarted = now;
    } else if (_readInFlight)
        _readRequest = id;
}

void LvMessageView::updateComposerState() {
    if (!_btnSend)
        return;
    bool hasText = !_inputText.empty();
    if (_rrcMode && (!_rrcLoaded || !rrcWritable()))
        hasText = false;
    const bool sendReady = hasText && !_sendPending;
    {
        for (auto *tone : Style::borderTones)
            lv_obj_remove_style(_btnSend, tone, 0);
        lv_obj_add_style(_btnSend, Style::borderTones[sendReady ? Style::PRIMARY : Style::BORDER], 0);
    }
    {
        for (auto *tone : Style::bgTones)
            lv_obj_remove_style(_btnSend, tone, 0);
        lv_obj_add_style(_btnSend, Style::bgTones[sendReady ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED], 0);
    }
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(lv_obj_get_child(_btnSend, 0), tone, 0);
        lv_obj_add_style(lv_obj_get_child(_btnSend, 0), Style::textTones[sendReady ? Style::PRIMARY : Style::TEXT_MUTED], 0);
    }
    if (_textarea) {
        {
            for (auto *tone : Style::borderTones)
                lv_obj_remove_style(_textarea, tone, 0);
            lv_obj_add_style(_textarea, Style::borderTones[hasText ? Style::PRIMARY_MUTED : Style::BORDER], 0);
        }
    }
#if HAS_SCROLLWHEEL
    if (_composerAction && (!composerActionAvailable(_composerAction) || !_service ||
                            _composerActionIdentity != _service->status().generation))
        _composerAction = 0;
    const bool sendFocus = _composerAction == 2;
    {
        lv_obj_remove_style(_btnSend, Style::buttonPressed, 0);
        lv_obj_remove_style(_btnSend, Style::borderStrong, 0);
        {
            lv_obj_remove_style(_btnSend, sendFocus ? Style::borderStrong : Style::buttonPressed, 0);
            lv_obj_add_style(_btnSend, sendFocus ? Style::borderStrong : Style::buttonPressed, 0);
        }
    }
    if (sendFocus) {
        lv_obj_remove_style(_btnSend, Style::borderAccent, 0);
        lv_obj_add_style(_btnSend, Style::borderAccent, 0);
    }
    if (_btnVoice) {
        const bool voiceFocus = _composerAction == 1;
        {
            lv_obj_remove_style(_btnVoice, Style::buttonPressed, 0);
            lv_obj_remove_style(_btnVoice, Style::borderStrong, 0);
            {
                lv_obj_remove_style(_btnVoice, voiceFocus ? Style::borderStrong : Style::buttonPressed, 0);
                lv_obj_add_style(_btnVoice, voiceFocus ? Style::borderStrong : Style::buttonPressed, 0);
            }
        }
        {
            for (auto *tone : Style::borderTones)
                lv_obj_remove_style(_btnVoice, tone, 0);
            lv_obj_add_style(_btnVoice, Style::borderTones[voiceFocus ? Style::ACCENT : Style::BORDER], 0);
        }
        {
            for (auto *tone : Style::bgTones)
                lv_obj_remove_style(_btnVoice, tone, 0);
            lv_obj_add_style(_btnVoice, Style::bgTones[voiceFocus ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED], 0);
        }
    }
#endif
    refreshComposerPlaceholder();
}

#if HAS_SCROLLWHEEL
bool LvMessageView::composerActionAvailable(uint8_t action) const {
    if (action == 1)
        return _btnVoice && !_rrcMode && _service && (_service->status().memo.capabilities & 1);
    // Visible footer controls keep their place even when inactive. The shared
    // send handler guards empty/pending/read-only drafts at activation time.
    return action == 2 && _btnSend;
}

void LvMessageView::setComposerAction(uint8_t action) {
    _composerAction = composerActionAvailable(action) ? action : 0;
    if (_composerAction && _service) {
        _selectAfterPage = false;
        _service->historyWindow().focusSpan(HistoryWindow::VisibleSpans);
        updateHistoryFocus();
    }
    _composerActionIdentity = _service ? _service->status().generation : 0;
    updateComposerState();
}

bool LvMessageView::handleComposerNavigation(const KeyEvent &event) {
    // This belongs to the screen's existing semantic input owner, not a second
    // LVGL encoder group. History and modal menus retain their own navigation.
    if (hasReadFocus() || _selectAfterPage)
        return false;
    if (_composerAction && (!_service || _composerActionIdentity != _service->status().generation ||
                            !composerActionAvailable(_composerAction))) {
        setComposerAction(0);
        return true; // Do not reinterpret a stale activation as sending text.
    }
    if (event.up || event.down) {
        if (!_composerAction && event.up)
            return false;
        if (event.repeat)
            return true;
        int next = int(_composerAction) + (event.up ? -1 : 1);
        while (next > 0 && next <= 2 && !composerActionAvailable(uint8_t(next)))
            next += event.up ? -1 : 1;
        if (next >= 0 && next <= 2)
            setComposerAction(uint8_t(next));
        return true; // Stop at the footer's end; never wrap to a different action.
    }
    if (!_composerAction)
        return false;
    const bool deleting = event.del || event.character == 0x08;
    if (deleting || event.character == 0x1b) {
        if (!event.repeat) {
            setComposerAction(0);
            _dismissedDelete = deleting;
        }
        return true;
    }
    if (event.enter || event.character == '\n' || event.character == '\r') {
        if (!event.repeat) {
            const auto action = _composerAction;
            setComposerAction(0);
            if (action == 1) {
                if (_ui)
                    _ui->openVoice(_peerHex.c_str());
            } else
                sendCurrentMessage(false);
        }
        return true;
    }
    if (event.tab || (event.character >= 0x20 && event.character < 0x7f))
        setComposerAction(0);
    return false; // Typing continues through the ordinary composer path.
}
#endif

void LvMessageView::composerEdited() {
    if (_nextDraftRevision != UINT64_MAX)
        ++_nextDraftRevision;
    _draftRevision = _nextDraftRevision;
    if (_rrcMode) {
        _rrcEditAt = millis();
        _rrcResendConfirmed = false;
        _rrcPreparedRevision = 0;
        return;
    }
    if (_peerHex == _retainedDraftPeer && _service &&
        _retainedDraftIdentity == _service->status().generation) {
        _retainedDraft = _inputText;
        _retainedDraftRevision = _draftRevision;
    }
}

void LvMessageView::refreshComposerPlaceholder() {
    if (!_textarea)
        return;
    bool focused = lv_obj_has_state(_textarea, LV_STATE_FOCUSED);
    lv_textarea_set_placeholder_text(_textarea, _rrcMode && !rrcWritable() ? "Hub notices are read-only" : _rrcMode && !_rrcLoaded        ? "Loading draft..."
                                                                                                       : (_inputText.empty() && !focused) ? kComposerPlaceholder
                                                                                                                                          : "");
    updateComposerText();
}

void LvMessageView::updateComposerText() {
    if (!_textarea)
        return;

    bool focused = lv_obj_has_state(_textarea, LV_STATE_FOCUSED);
    bool showCaret = focused || !_inputText.empty();
    if (!showCaret) {
        lv_textarea_set_text(_textarea, "");
        return;
    }

    std::string display = _inputText;
#if HAS_SCROLLWHEEL
    if (!_composerAction)
        display += "_";
#else
#endif
    lv_textarea_set_text(_textarea, display.c_str());
    lv_textarea_set_cursor_pos(_textarea, LV_TEXTAREA_CURSOR_LAST);
}

void LvMessageView::createUI(lv_obj_t *parent) {
    _screen = parent;
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    {
        lv_obj_remove_style(parent, Style::listColor, 0);
        lv_obj_add_style(parent, Style::listColor, 0);
    }
    {
        lv_obj_remove_style(parent, Style::conversationColumn, 0);
        lv_obj_add_style(parent, Style::conversationColumn, 0);
    }

    // Use flex column layout: header, messages (grows), input

    // Header bar (top)
    _header = lv_obj_create(parent);
    lv_obj_add_style(_header, Style::conversationHeader, 0);
    lv_obj_add_style(_header, Style::bgElevated, 0);

    lv_obj_add_style(_header, Style::borderBorder, 0);

    lv_obj_clear_flag(_header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *backLbl = lv_label_create(_header);
    lv_obj_add_style(backLbl, Style::fontTitle, 0);
    lv_obj_add_style(backLbl, Style::textActiveTabText, 0);
    lv_label_set_text(backLbl, "<");
    lv_obj_add_style(backLbl, Style::backCaption, 0);

    _lblHeader = lv_label_create(_header);
    lv_obj_add_style(_lblHeader, Style::fontTitle, 0);
    lv_obj_add_style(_lblHeader, Style::textAccent, 0);
    lv_label_set_long_mode(_lblHeader, LV_LABEL_LONG_DOT);
    lv_obj_add_style(_lblHeader, Style::conversationHeading, 0);

    _lblHeaderMeta = lv_label_create(_header);
    lv_obj_add_style(_lblHeaderMeta, Style::fontCaption, 0);
    lv_obj_add_style(_lblHeaderMeta, Style::textSecondary, 0);
    lv_label_set_long_mode(_lblHeaderMeta, LV_LABEL_LONG_DOT);
    lv_obj_add_style(_lblHeaderMeta, Style::conversationSubtitle, 0);

    // Make header tappable for back navigation
    lv_obj_add_flag(_header, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(_header, [](lv_event_t *e) {
        auto* self = (LvMessageView*)lv_event_get_user_data(e);
        self->goBack(); }, LV_EVENT_CLICKED, this);

    // Message scroll area (middle, grows to fill)
    _msgScroll = lv_obj_create(parent);
    lv_obj_clear_flag(_msgScroll, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_style(_msgScroll, Style::messageHistory, 0);

    lv_obj_add_style(_msgScroll, Style::listColor, 0);

    lv_obj_add_style(_msgScroll, Style::scrollbarColor, LV_PART_SCROLLBAR);
    lv_obj_add_style(_msgScroll, Style::scrollbar, LV_PART_SCROLLBAR);

    lv_obj_add_event_cb(_msgScroll, [](lv_event_t *e) {
        auto* self = static_cast<LvMessageView*>(lv_event_get_user_data(e));
        const auto code = lv_event_get_code(e);
        if (code == LV_EVENT_GESTURE && self->boundWindow() &&
            lv_obj_get_scroll_y(self->_msgScroll) <= 0 && lv_obj_get_scroll_bottom(self->_msgScroll) <= 1) {
            // A short final page has no scroll overflow, so LVGL reports a
            // gesture instead. It must still reach the adjacent history page.
            auto* input = lv_indev_get_act();
            if (input && lv_indev_get_type(input) == LV_INDEV_TYPE_POINTER) {
                self->dismissSelection();
                const auto direction = lv_indev_get_gesture_dir(input);
                if (direction == LV_DIR_BOTTOM) self->historyAction(0);
                else if (direction == LV_DIR_TOP) self->historyAction(1);
            }
        } else if (code == LV_EVENT_SCROLL_BEGIN) {
            auto* input = lv_indev_get_act();
            // Elastic rebound emits nested animation begin/end events. Only
            // the input-owned scroll starts a gesture; keep its original peak.
            if (!self->_touchScrolling && !self->_binding && !lv_event_get_param(e) &&
                input && lv_indev_get_type(input) == LV_INDEV_TYPE_POINTER &&
                lv_indev_get_scroll_obj(input) == self->_msgScroll) {
                self->dismissSelection();
                self->_touchScrolling = true;
                self->_touchScrollStart = lv_obj_get_scroll_y(self->_msgScroll);
                self->_touchScrollPeak = 0;
            }
        } else if (code == LV_EVENT_SCROLL) {
            if (self->_touchScrolling) {
                const auto delta = lv_obj_get_scroll_y(self->_msgScroll) - self->_touchScrollStart;
                // Keep the gesture direction through LVGL's elastic rebound.
                if (std::abs(delta) > std::abs(self->_touchScrollPeak)) self->_touchScrollPeak = delta;
            }
            self->saveScroll(self->_touchScrolling);
        } else if (code == LV_EVENT_SCROLL_END && self->_touchScrolling && lv_event_get_param(e)) {
            self->_touchScrolling = false;
            if (!self->boundWindow()) return;
            self->saveScroll(true);
            if (self->_touchScrollPeak < -8 && lv_obj_get_scroll_y(self->_msgScroll) <= 0)
                self->historyAction(0);
            else if (self->_touchScrollPeak > 8 && lv_obj_get_scroll_bottom(self->_msgScroll) <= 1)
                self->historyAction(1);
        } }, LV_EVENT_ALL, this);

    // A notice only occupies space when new arrivals or a failed read need an
    // action. Ordinary history navigation follows touch/wheel scrolling.
    _historyNotice = lv_btn_create(parent);
    lv_obj_add_style(_historyNotice, Style::historyNotice, 0);
    lv_obj_add_style(_historyNotice, Style::buttonColor, 0);
    lv_obj_add_style(_historyNotice, Style::button, 0);
    lv_obj_add_style(_historyNotice, Style::noPadding, 0);
    lv_obj_add_flag(_historyNotice, LV_OBJ_FLAG_HIDDEN);
    _historyNoticeLabel = lv_label_create(_historyNotice);
    lv_obj_add_style(_historyNoticeLabel, Style::fontBody, 0);
    lv_obj_add_style(_historyNoticeLabel, Style::centered, 0);
    lv_obj_add_event_cb(_historyNotice, [](lv_event_t *e) {
        auto* self = static_cast<LvMessageView*>(lv_event_get_user_data(e));
        self->historyAction(self->_noticeAction); }, LV_EVENT_CLICKED, this);

    // Input row (bottom, just above tab bar)
    _inputRow = lv_obj_create(parent);
    lv_obj_add_style(_inputRow, Style::composerRow, 0);
    lv_obj_add_style(_inputRow, Style::bgElevated, 0);

    lv_obj_add_style(_inputRow, Style::borderBorder, 0);

    lv_obj_clear_flag(_inputRow, LV_OBJ_FLAG_SCROLLABLE);

    _textarea = lv_textarea_create(_inputRow);
    lv_obj_add_style(_textarea, Style::messageInput, 0);

    lv_textarea_set_one_line(_textarea, true);
    lv_textarea_set_max_length(_textarea, (_rrcMode ? handheld::rrc::DraftCapacity : MAX_COMPOSER_CHARS) + 1);
    lv_textarea_set_placeholder_text(_textarea, kComposerPlaceholder);
    lv_obj_add_style(_textarea, Style::buttonColor, 0);
    lv_obj_add_style(_textarea, Style::textarea, 0);
    lv_obj_add_style(_textarea, Style::textareaFocusedColor, LV_STATE_FOCUSED);
    lv_obj_add_style(_textarea, Style::buttonPressed, LV_STATE_FOCUSED);
    lv_obj_add_style(_textarea, Style::compactInput, 0);
    lv_obj_add_style(_textarea, Style::fontBody, 0);

    lv_obj_add_event_cb(_textarea, [](lv_event_t *e) {
        auto* self = (LvMessageView*)lv_event_get_user_data(e);
        lv_event_code_t code = lv_event_get_code(e);
        if(code == LV_EVENT_READY) {
            const auto capacity=self->_rrcMode?handheld::rrc::DraftCapacity:MAX_COMPOSER_CHARS;
            self->_inputText=handheld::utf8::prefix(lv_textarea_get_text(self->_textarea),capacity);
            self->composerEdited();self->updateComposerState();self->updateComposerText();
            return;
        }
        if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED || code == LV_EVENT_PRESSED) {
            self->dismissSelection();
            self->refreshComposerPlaceholder();
        } else if (code == LV_EVENT_DEFOCUSED) {
            self->refreshComposerPlaceholder();
        } }, LV_EVENT_ALL, this);

    _btnSend = lv_btn_create(_inputRow);
    lv_obj_add_style(_btnSend, Style::sendAction, 0);

    lv_obj_add_style(_btnSend, Style::buttonColor, 0);
    lv_obj_add_style(_btnSend, Style::button, 0);
    lv_obj_add_style(_btnSend, Style::noPadding, 0);
    lv_obj_t *sendLbl = lv_label_create(_btnSend);
    lv_obj_add_style(sendLbl, Style::fontCaption, 0);
    lv_obj_add_style(sendLbl, Style::textActiveTabText, 0);
    lv_label_set_text(sendLbl, "SEND");
    lv_obj_add_style(sendLbl, Style::centered, 0);
    lv_obj_add_event_cb(_btnSend, [](lv_event_t *e) {
        auto* self = (LvMessageView*)lv_event_get_user_data(e);
        if (self->_suppressNextSendClick) {
            self->_suppressNextSendClick = false;
            return;
        }
        self->sendCurrentMessage(false); }, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(_btnSend, [](lv_event_t *e) {
        auto* self = (LvMessageView*)lv_event_get_user_data(e);
        self->_suppressNextSendClick = true;
        self->showSendModeMenu(); }, LV_EVENT_LONG_PRESSED, this);

    if (!_rrcMode && _service && (_service->status().memo.capabilities & 1)) {
#if HAS_SCROLLWHEEL
        _btnVoice = lv_btn_create(_inputRow);
        lv_obj_add_style(_btnVoice, Style::recordAction, 0);

        lv_obj_add_style(_textarea, Style::messageInputWithAudio, 0);
#else
        lv_obj_add_style(_lblHeader, Style::headingWithAudio, 0);
        lv_obj_add_style(_lblHeaderMeta, Style::headingWithAudio, 0);
        _btnVoice = lv_btn_create(_header);
        lv_obj_add_style(_btnVoice, Style::audioHeaderAction, 0);

#endif
        lv_obj_add_style(_btnVoice, Style::buttonColor, 0);
        lv_obj_add_style(_btnVoice, Style::button, 0);
        lv_obj_add_style(_btnVoice, Style::noPadding, 0);
        auto *label = lv_label_create(_btnVoice);
        lv_label_set_text(label, "Voice");
        lv_obj_add_style(label, Style::fontBody, 0);
        lv_obj_add_style(label, Style::textPrimary, 0);
        lv_obj_add_style(label, Style::centered, 0);
        lv_obj_add_event_cb(_btnVoice, [](lv_event_t *e) {
            auto& self = *static_cast<LvMessageView*>(lv_event_get_user_data(e));
            self.dismissSelection();
            if (self._ui) self._ui->openVoice(self._peerHex.c_str()); }, LV_EVENT_CLICKED, this);
    }

    updateHeader();
    updateComposerState();
}

bool LvMessageView::setPeerHex(const std::string &hex) {
    if (_rrcMode && !leaveRrcDraft())
        return false;
    const bool wasRrc = _rrcMode;
    _rrcMode = false;
    ++_rrcView;
    if (_peerHex == hex && !wasRrc)
        return true;
    dismissSelection();
    _messageTools.close();
    hideSendModeMenu();
    _selectAfterPage = false;
    if (_ui)
        _ui->closeVoiceConversation();
    clearMessages();
    if (_service)
        _service->historyWindow().acknowledgePublication(_service->historyWindow().revision());
    _peerHex = hex;
    _nameResolved = false;
    _scrollToEnd = true;
    if (_entered && _service) {
        if (_retainedDraftPeer == _peerHex && _retainedDraftIdentity == _service->status().generation) {
            _inputText = _retainedDraft;
            _draftRevision = _retainedDraftRevision;
        } else {
            _inputText.clear();
            composerEdited();
        }
        updateComposerText();
        updateComposerState();
        _service->watchHistory(_peerHex);
        _markReadPending = true;
        _readThrough = 0;
        _readRetry = false;
        refreshUI();
    }
    return true;
}

bool LvMessageView::windowMatches() const {
    if (!_service || _peerHex.size() != 32)
        return false;
    if (_service->rrcHistory() != _rrcMode)
        return false;
    const auto &window = _service->historyWindow();
    if (window.identityGeneration() != _service->status().generation)
        return false;
    static constexpr char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < 16; ++i) {
        const auto byte = window.peer()[i];
        const auto lower = [](char value) { return value >= 'A' && value <= 'F' ? char(value + 'a' - 'A') : value; };
        if (lower(_peerHex[i * 2]) != hex[byte >> 4] || lower(_peerHex[i * 2 + 1]) != hex[byte & 15])
            return false;
    }
    return true;
}

bool LvMessageView::boundWindow() const {
    if (!_entered || !windowMatches())
        return false;
    const auto &window = _service->historyWindow();
    return window.visible() && window.statusReady() && _lastHistoryRevision == window.revision() &&
           _boundMode == window.mode() && _boundIdentity == window.identityGeneration();
}

void LvMessageView::clearMessages() {
    // Deleting the labels synchronously ends every static-text pointer lease.
    _binding = true;
    if (_msgScroll)
        lv_obj_clean(_msgScroll);
    _statusLabels.fill(nullptr);
    _textLabels.fill(nullptr);
    _audioPressIndex = _messagePressIndex = HistoryWindow::VisibleSpans;
    _bubbleBoxes.fill(nullptr);
    _readButtons.fill(nullptr);
    _audioErrors.fill(nullptr);
    _emptyLabel = _queuedAudio = nullptr;
    _touchScrolling = false;
    _rowCount = 0;
    _lastHistoryRevision = 0;
    _lastStatusRevision = 0;
    _boundMode = HistoryWindow::Mode::Closed;
    _boundIdentity = 0;
    _binding = false;
}

void LvMessageView::saveScroll(bool userChange) {
    if (_binding || !_msgScroll || !boundWindow())
        return;
    const auto y = lv_obj_get_scroll_y(_msgScroll);
    _service->historyWindow().setScrollOffset(y > 0 ? uint32_t(y) : 0);
    _atBottom = lv_obj_get_scroll_bottom(_msgScroll) <= 1;
    if (userChange)
        _service->historyWindow().setViewportAtNewest(_atBottom);
}

void LvMessageView::historyAction(unsigned action) {
    if (!_service || !windowMatches())
        return;
    auto &window = _service->historyWindow();
    saveScroll();
    const bool full = window.mode() == HistoryWindow::Mode::Full;
    if (action < 2 && !boundWindow())
        return;
    bool changed = false;
    if (action == 0) {
        changed = window.older();
        if (changed)
            _scrollToEnd = !full;
    } else if (action == 1) {
        changed = window.newer();
        if (changed)
            _scrollToEnd = false;
    } else if (action == 2) {
        changed = full ? window.backToChat() : window.newest();
        if (changed)
            _scrollToEnd = !full;
    } else {
        window.refresh();
        _nameResolved = false;
        changed = true;
    }
    if (changed)
        refreshUI();
}

void LvMessageView::readFull(size_t index) {
    // Old widgets may still be visible while a replacement's status is pending.
    // Their indices cannot select rows from that unbound publication.
    if (!boundWindow())
        return;
    if (_rrcMode) {
        openRrcTools(index);
        return;
    }
    auto       &window = _service->historyWindow();
    const auto *row = window.span(index);
    if (row && row->hasAudio()) {
        auto *memo = _ui ? _ui->memoUi() : nullptr;
        if (memo && row->nativeAudio() && (memo->status().capabilities & 2))
            memo->toggleMessage(window.peer(), row->counter, row->incoming());
        else
            openMessageTools(index);
        updateAudioControls();
        return;
    }
    if (row)
        openMessageTools(index);
}

void LvMessageView::goBack() {
    if (_service && windowMatches() && _service->historyWindow().mode() == HistoryWindow::Mode::Full) {
        historyAction(2);
        return;
    }
    if (dismissSelection())
        return;
    if (_onBack)
        _onBack();
}

void LvMessageView::scrollHistory(int pixels) {
    if (!_msgScroll)
        return;
    saveScroll(true);
    if (boundWindow() && ((pixels < 0 && lv_obj_get_scroll_y(_msgScroll) <= 0) ||
                          (pixels > 0 && lv_obj_get_scroll_bottom(_msgScroll) <= 1))) {
        historyAction(pixels < 0 ? 0 : 1);
    } else {
        lv_obj_scroll_to_y(_msgScroll, lv_obj_get_scroll_y(_msgScroll) + pixels, LV_ANIM_OFF);
        saveScroll(true);
    }
}

bool LvMessageView::hasReadFocus() const {
    return boundWindow() && _service->historyWindow().mode() == HistoryWindow::Mode::Chat &&
           _service->historyWindow().focusedSpan() < _rowCount &&
           _bubbleBoxes[_service->historyWindow().focusedSpan()];
}

bool LvMessageView::dismissSelection() {
#if HAS_SCROLLWHEEL
    const bool composerSelected = _composerAction != 0;
    if (composerSelected)
        setComposerAction(0);
#else
    constexpr bool composerSelected = false;
#endif
    if (!_service)
        return composerSelected;
    auto      &window = _service->historyWindow();
    const bool selected = _selectAfterPage || window.focusedSpan() < HistoryWindow::VisibleSpans;
    _selectAfterPage = false;
    window.focusSpan(HistoryWindow::VisibleSpans);
    updateHistoryFocus();
    return selected || composerSelected;
}

void LvMessageView::selectVisibleMessage() {
    if (!boundWindow() || _service->historyWindow().loading())
        return;
    lv_area_t viewport;
    lv_obj_get_coords(_msgScroll, &viewport);
    int    best = 0;
    size_t selected = HistoryWindow::VisibleSpans;
    // Select what the user is reading, including an older page. At the
    // newest end, the first action starts with the most recent message.
    for (size_t i = 0; i < _rowCount; ++i)
        if (_bubbleBoxes[i] && !_service->historyWindow().span(i)->unavailable()) {
            lv_area_t row;
            lv_obj_get_coords(_bubbleBoxes[i], &row);
            const int overlap = std::min(row.y2, viewport.y2) - std::max(row.y1, viewport.y1) + 1;
            if (overlap > 0 && (overlap >= best || _atBottom)) {
                best = overlap;
                selected = i;
            }
        }
    if (selected == HistoryWindow::VisibleSpans)
        return;
    auto &window = _service->historyWindow();
    window.focusSpan(selected);
    window.setViewportAtNewest(false);
    revealSelection();
}

void LvMessageView::revealSelection() {
    if (!_msgScroll || !boundWindow())
        return;
    updateHistoryFocus();
    const auto focus = _service->historyWindow().focusedSpan();
    if (focus < _rowCount && _bubbleBoxes[focus]) {
        // Focus changes border thickness and therefore bubble height. Keep the
        // fixed-height rows in sync before measuring the selected message.
        lv_obj_update_layout(_msgScroll);
        for (size_t i = 0; i < _rowCount; ++i)
            if (_bubbleBoxes[i])

                lv_obj_update_layout(_msgScroll);
        // LVGL's nonrecursive reveal only scrolls the immediate parent. The
        // row is a direct child of the viewport; its nested bubble and Play
        // button both have non-scrolling parents.
        auto     *row = lv_obj_get_parent(_bubbleBoxes[focus]);
        lv_area_t viewport, item;
        lv_obj_get_content_coords(_msgScroll, &viewport);
        lv_obj_get_coords(row, &item);
        if (lv_area_get_height(&item) > lv_area_get_height(&viewport))
            lv_obj_scroll_to_y(_msgScroll, lv_obj_get_scroll_y(_msgScroll) + item.y1 - viewport.y1, LV_ANIM_OFF);
        else
            lv_obj_scroll_to_view(row, LV_ANIM_OFF);
        saveScroll();
    }
}

void LvMessageView::focusNextRead(int direction) {
    if (!boundWindow() || _service->historyWindow().mode() != HistoryWindow::Mode::Chat)
        return;
    auto &window = _service->historyWindow();
    if (!hasReadFocus()) {
        selectVisibleMessage();
        return;
    }
    if (!window.moveSelection(direction))
        return;
    if (window.loading()) {
        _selectAfterPage = true;
        return;
    }
    revealSelection();
    if (!hasReadFocus()) {
        lv_obj_scroll_to_y(_msgScroll, LV_COORD_MAX, LV_ANIM_OFF);
        saveScroll(true);
    }
}

void LvMessageView::updateHistoryFocus() {
    size_t focus = _service ? _service->historyWindow().focusedSpan() : HistoryWindow::VisibleSpans;
    if (boundWindow() && _service->historyWindow().mode() == HistoryWindow::Mode::Chat &&
        focus < HistoryWindow::VisibleSpans && (!_bubbleBoxes[focus] || _service->historyWindow().span(focus)->unavailable())) {
        _service->historyWindow().focusSpan(HistoryWindow::VisibleSpans);
        focus = HistoryWindow::VisibleSpans;
    }
    for (size_t i = 0; i < _rowCount; ++i)
        if (_bubbleBoxes[i]) {
            const auto *row = _service->historyWindow().span(i);
            const auto  normal = _rrcMode || !row || row->incoming() ? Style::BORDER : bubbleBorderColor(static_cast<LXMFStatus>(row->status));
            {
                for (auto *tone : Style::borderTones)
                    lv_obj_remove_style(_bubbleBoxes[i], tone, 0);
                lv_obj_add_style(_bubbleBoxes[i], Style::borderTones[i == focus ? Style::ACCENT : normal], 0);
            }
            {
                lv_obj_remove_style(_bubbleBoxes[i], Style::buttonPressed, 0);
                lv_obj_remove_style(_bubbleBoxes[i], Style::borderStrong, 0);
                {
                    lv_obj_remove_style(_bubbleBoxes[i], i == focus ? Style::borderStrong : Style::buttonPressed, 0);
                    lv_obj_add_style(_bubbleBoxes[i], i == focus ? Style::borderStrong : Style::buttonPressed, 0);
                }
            }
        }
    for (size_t i = 0; i < _rowCount; ++i)
        if (_readButtons[i]) {
            {
                for (auto *tone : Style::borderTones)
                    lv_obj_remove_style(_readButtons[i], tone, 0);
                lv_obj_add_style(_readButtons[i], Style::borderTones[i == focus ? Style::ACCENT : Style::BORDER], 0);
            }
            {
                for (auto *tone : Style::bgTones)
                    lv_obj_remove_style(_readButtons[i], tone, 0);
                lv_obj_add_style(_readButtons[i], Style::bgTones[i == focus ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED], 0);
            }
        }
}

void LvMessageView::updateAudioControls() {
    if (_rrcMode || !boundWindow())
        return;
    const auto &window = _service->historyWindow();
    const auto *memo = _ui ? _ui->memoUi() : nullptr;
    for (size_t i = 0; i < _rowCount; ++i) {
        const auto *row = window.span(i);
        auto       *button = _readButtons[i];
        if (!button || !row || !row->hasAudio())
            continue;
        char control[96];
        if (!row->nativeAudio())
            snprintf(control, sizeof control, "%s", row->audioLabel());
        else if (memo && !(memo->status().capabilities & 2))
            snprintf(control, sizeof control, "No speaker on this device");
        else
            handheld::ui::messageAudioControl(memo, window.peer(), *row, control, sizeof control);
        if (!row->nativeAudio() || !memo || !(memo->status().capabilities & 2))
            lv_obj_add_state(button, LV_STATE_DISABLED);
        else
            lv_obj_clear_state(button, LV_STATE_DISABLED);
        auto *label = lv_obj_get_child(button, 0);
        if (strcmp(lv_label_get_text(label), control))
            lv_label_set_text(label, control);
        if (_audioErrors[i]) {
            const char *error = handheld::ui::messageAudioError(memo, window.peer(), *row);
            const auto *text = error ? error : "";
            if (strcmp(lv_label_get_text(_audioErrors[i]), text)) {
                lv_label_set_text(_audioErrors[i], text);
                if (error)
                    lv_obj_clear_flag(_audioErrors[i], LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_add_flag(_audioErrors[i], LV_OBJ_FLAG_HIDDEN);
                lv_obj_update_layout(_bubbleBoxes[i]);
            }
        }
    }
}
void LvMessageView::updateHistoryControls() {
    updateAudioControls();
    if (!_historyNotice || !_service)
        return;
    auto      &window = _service->historyWindow();
    const bool matches = windowMatches();
    uint32_t   sentCounter = 0;
    uint16_t   sentLength = 0;
    if (matches && !_rrcMode && _ui && _ui->memoUi() && _ui->memoUi()->takeSent(window.peer(), &sentCounter, &sentLength)) {
        _messageTools.close();
        _selectAfterPage = false;
        window.queuedAudio(sentCounter, sentLength);
        _scrollToEnd = true;
    }
    const bool  ready = matches && window.visible() && window.statusReady();
    const char *notice = nullptr;
    _noticeAction = 3;
    if (matches && window.state() != HistoryWindow::State::Exhausted) {
        if (window.error() == HistoryWindow::Error::Busy)
            notice = "Storage busy; retry";
        else if (window.error() != HistoryWindow::Error::None)
            notice = "Read failed; retry";
        else if (!window.freshnessAvailable())
            notice = "Updates unavailable; refresh";
        else if (ready && window.statusRefreshDelayed())
            notice = "Status refresh delayed; retry";
        else if (ready && window.newBelow() && window.mode() == HistoryWindow::Mode::Chat) {
            notice = "New messages";
            _noticeAction = 2;
        }
    }
    if (notice) {
        if (strcmp(lv_label_get_text(_historyNoticeLabel), notice))
            lv_label_set_text(_historyNoticeLabel, notice);
        lv_obj_clear_flag(_historyNotice, LV_OBJ_FLAG_HIDDEN);
    } else
        lv_obj_add_flag(_historyNotice, LV_OBJ_FLAG_HIDDEN);

    const auto queued = matches && !_rrcMode ? window.queuedAudioSeconds() : 0;
    if (queued && !_queuedAudio) {
        {
            lv_obj_remove_style(_msgScroll, Style::column, 0);
            lv_obj_add_style(_msgScroll, Style::column, 0);
        }
        _queuedAudio = lv_obj_create(_msgScroll);
        {
            lv_obj_remove_style(_queuedAudio, Style::pendingAudioRow, 0);
            lv_obj_add_style(_queuedAudio, Style::pendingAudioRow, 0);
        }

        lv_obj_clear_flag(_queuedAudio, LV_OBJ_FLAG_SCROLLABLE);
        auto *box = lv_obj_create(_queuedAudio);
        {
            lv_obj_remove_style(box, Style::pendingAudioBubble, 0);
            lv_obj_add_style(box, Style::pendingAudioBubble, 0);
        }
        lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);
        {
            lv_obj_remove_style(box, Style::bgOutgoing, 0);
            lv_obj_add_style(box, Style::bgOutgoing, 0);
        }

        {
            lv_obj_remove_style(box, Style::borderBorder, 0);
            lv_obj_add_style(box, Style::borderBorder, 0);
        }

        auto *title = lv_label_create(box);
        lv_label_set_text(title, "Voice message");
        {
            lv_obj_remove_style(title, Style::fontBody, 0);
            lv_obj_add_style(title, Style::fontBody, 0);
        }
        {
            lv_obj_remove_style(title, Style::textAccent, 0);
            lv_obj_add_style(title, Style::textAccent, 0);
        }
        auto *detail = lv_label_create(box);
        lv_label_set_text(detail, "");
        {
            lv_obj_remove_style(detail, Style::audioDetail, 0);
            lv_obj_add_style(detail, Style::audioDetail, 0);
        }
        {
            lv_obj_remove_style(detail, Style::fontCaption, 0);
            lv_obj_add_style(detail, Style::fontCaption, 0);
        }
        {
            lv_obj_remove_style(detail, Style::textSecondary, 0);
            lv_obj_add_style(detail, Style::textSecondary, 0);
        }
        lv_obj_update_layout(_msgScroll);
        lv_obj_scroll_to_y(_msgScroll, LV_COORD_MAX, LV_ANIM_OFF);
    } else if (!queued && _queuedAudio) {
        lv_obj_del(_queuedAudio);
        _queuedAudio = nullptr;
    }
    if (queued && _queuedAudio) {
        auto *detail = lv_obj_get_child(lv_obj_get_child(_queuedAudio, 0), 1);
        char  text[40];
        snprintf(text, sizeof text, "0:%02u · Queued", unsigned(queued));
        if (strcmp(lv_label_get_text(detail), text))
            lv_label_set_text(detail, text);
    }
    if (queued && _emptyLabel) {
        lv_obj_del(_emptyLabel);
        _emptyLabel = nullptr;
    }
    if (!_rowCount && !queued) {
        if (!_emptyLabel) {
            _emptyLabel = lv_label_create(_msgScroll);
            lv_obj_add_flag(_emptyLabel, LV_OBJ_FLAG_FLOATING);
            {
                lv_obj_remove_style(_emptyLabel, Style::fontBody, 0);
                lv_obj_add_style(_emptyLabel, Style::fontBody, 0);
            }
            {
                lv_obj_remove_style(_emptyLabel, Style::textMuted, 0);
                lv_obj_add_style(_emptyLabel, Style::textMuted, 0);
            }
            {
                lv_obj_remove_style(_emptyLabel, Style::centeredCaption, 0);
                lv_obj_add_style(_emptyLabel, Style::centeredCaption, 0);
            }
        }
        const char *text = matches && window.state() == HistoryWindow::State::Exhausted ? "Reopen conversation" : matches && window.error() != HistoryWindow::Error::None ? "Messages unavailable"
                                                                                                              : !ready || window.loading()                                ? "Loading messages..."
                                                                                                                                                                          : "No messages yet";
        if (strcmp(lv_label_get_text(_emptyLabel), text))
            lv_label_set_text(_emptyLabel, text);
    }
}

void LvMessageView::destroyUI() {
    hideSendModeMenu();
    clearMessages();
    if (_service)
        _service->historyWindow().acknowledgePublication(_service->historyWindow().revision());
    _header = _lblHeader = _lblHeaderMeta = _msgScroll = nullptr;
    _historyNotice = _historyNoticeLabel = _emptyLabel = nullptr;
    _inputRow = _textarea = _btnSend = _btnVoice = nullptr;
#if HAS_SCROLLWHEEL
    _composerAction = 0;
#endif
    for (size_t i = 0; i < 3; ++i) {
        _sendRows[i] = nullptr;
        _sendLabels[i] = nullptr;
    }
    LvScreen::destroyUI();
}

void LvMessageView::onEnter() {
    dismissSelection();
    _selectAfterPage = false;
    _dismissedDelete = false;
    _entered = true;
    if (_rrcMode) {
        _markReadPending = false;
        _readThrough = 0;
        _readRetry = false;
        _scrollToEnd = true;
        _atBottom = true;
        hideSendModeMenu();
        clearMessages();
        enterRrcHistory();
        loadRrcDraft();
        updateComposerText();
        updateHeader();
        updateComposerState();
        return;
    }
    _nameResolved = false;
    _markReadPending = _service != nullptr;
    _readThrough = 0;
    _readRetry = false;
    if (_service)
        _service->watchHistory(_peerHex);
    if (_retainedDraftPeer == _peerHex && _service &&
        _retainedDraftIdentity == _service->status().generation) {
        _inputText = _retainedDraft;
        _draftRevision = _retainedDraftRevision;
    } else {
        _inputText.clear();
        composerEdited();
    }
    _scrollToEnd = true;
    _atBottom = true;
    hideSendModeMenu();
    updateComposerText();
    updateHeader();
    updateComposerState();
    clearMessages();
    refreshUI();
}

void LvMessageView::onHide() {
    dismissSelection();
    _selectAfterPage = false;
    _messageTools.close();
    _rrcTools.close();
    hideSendModeMenu();
    if (_ui)
        _ui->closeVoiceConversation();
    _rrcSendRequested = false;
    if (_rrcMode)
        saveRrcDraft(true);
    else if (_service && (!_sendPending || _retainedDraftPeer == _peerHex)) {
        _retainedDraft = _inputText;
        _retainedDraftPeer = _peerHex;
        _retainedDraftRevision = _draftRevision;
        _retainedDraftIdentity = _service->status().generation;
    }
    saveScroll();
    _entered = false;
    _markReadPending = false;
    if (_service)
        _service->pauseHistory(true);
}

void LvMessageView::onShow() {
    if (!_service || !windowMatches() ||
        _service->historyWindow().mode() == HistoryWindow::Mode::Closed) {
        onEnter();
        return;
    }
    _entered = true;
    _service->pauseHistory(false);
    updateComposerText();
    updateComposerState();
    refreshUI();
}

void LvMessageView::onExit() {
    dismissSelection();
    _selectAfterPage = false;
    _messageTools.close();
    if (_ui)
        _ui->closeVoiceConversation();
    _rrcTools.close();
    hideSendModeMenu();
    _rrcSendRequested = false; // Leaving cancels an intent still waiting for draft durability.
    if (_rrcMode)
        saveRrcDraft(true);
    else if (_service && !_inputText.empty() && (!_sendPending || _retainedDraftPeer == _peerHex)) {
        _retainedDraft = _inputText;
        _retainedDraftPeer = _peerHex;
        _retainedDraftRevision = _draftRevision;
        _retainedDraftIdentity = _service->status().generation;
    }
    saveScroll();
    _entered = false;
    _nameResolved = false;
    clearMessages();
    if (_service) {
        _service->closeHistory();
        _service->historyWindow().acknowledgePublication(_service->historyWindow().revision());
    }
    _markReadPending = false;
    hideSendModeMenu();
    if (!_rrcMode && (!_sendPending || _retainedDraftPeer == _peerHex))
        _inputText.clear();
}

void LvMessageView::refreshUI() {
    pollRrc();
    if (!_screen || !_entered || !_service)
        return;
    auto      &window = _service->historyWindow();
    const bool matches = windowMatches();
    if (!matches || !window.visible()) {
        if (!matches || _rowCount || _lastHistoryRevision)
            clearMessages();
        // This publication was abandoned by a peer/identity/mode change. No
        // label remains; release its bank even if its old status query is held.
        window.acknowledgePublication(window.revision());
    } else if (_lastHistoryRevision &&
               (_boundMode != window.mode() || _boundIdentity != window.identityGeneration()))
        clearMessages();
    if (matches && window.visible() && window.statusReady()) {
        if (_lastHistoryRevision != window.revision())
            rebuildMessages();
        else if (_lastStatusRevision != window.statusRevision()) {
            _binding = true;
            for (size_t i = 0; i < _rowCount; ++i)
                updateMessageStatus(i, *window.span(i));
            _lastStatusRevision = window.statusRevision();
            lv_obj_update_layout(_msgScroll);
            lv_obj_scroll_to_y(_msgScroll, window.scrollOffset(), LV_ANIM_OFF);
            _binding = false;
        }
    }
    updateHeader();
    updateHistoryControls();
    updateHistoryFocus();
#if HAS_SCROLLWHEEL
    if (_composerAction && (!composerActionAvailable(_composerAction) ||
                            _composerActionIdentity != _service->status().generation))
        setComposerAction(0);
#endif
    if (matches && window.visible() && window.statusReady() &&
        (window.mode() == HistoryWindow::Mode::Chat || (_rrcMode && window.mode() == HistoryWindow::Mode::Full))) {
        // New incoming tuples can request one further write while the current
        // one is held. Status changes and stale unread snapshots cannot.
        for (size_t i = 0; i < window.spanCount(); ++i) {
            const auto &row = *window.span(i);
            bool        visible = true;
            if (_rrcMode && _bubbleBoxes[i]) {
                lv_area_t item, viewport;
                lv_obj_get_coords(_bubbleBoxes[i], &item);
                lv_obj_get_coords(_msgScroll, &viewport);
                // Seeing the end also covers a message taller than the screen.
                visible = item.y2 >= viewport.y1 && item.y2 <= viewport.y2;
            }
            if (visible && row.incoming() && !row.unavailable() && row.counter > _readThrough) {
                _readThrough = row.counter;
                if (!(row.flags & Span::Read))
                    _markReadPending = true;
            }
        }
        if (window.total())
            markVisibleConversationRead();
        else
            _markReadPending = false;
    }
}

void LvMessageView::appendMessage(size_t index, const Span &span, const char *text) {
    if (!_msgScroll || index >= HistoryWindow::VisibleSpans)
        return;
    const auto status = static_cast<LXMFStatus>(span.status);

    {
        lv_obj_remove_style(_msgScroll, Style::column, 0);
        lv_obj_add_style(_msgScroll, Style::column, 0);
    }

    lv_obj_t *bubble = lv_obj_create(_msgScroll);
    lv_obj_add_style(bubble, Style::messageRow, 0);

    lv_obj_clear_flag(bubble, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *box = lv_obj_create(bubble);
    lv_obj_add_style(box, _service && _service->historyWindow().mode() == HistoryWindow::Mode::Full ? Style::fullWidth : Style::messageBubble, 0);
    lv_obj_add_style(box, Style::bubbleContent, 0);

    {
        for (auto *tone : Style::borderTones)
            lv_obj_remove_style(box, tone, 0);
        lv_obj_add_style(box, Style::borderTones[_rrcMode ? Style::BORDER : bubbleBorderColor(status)], 0);
    }
    lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE);

    if (span.incoming()) {
        lv_obj_add_style(box, Style::bgIncoming, 0);
        lv_obj_add_style(box, Style::incomingPosition, 0);
    } else {
        lv_obj_add_style(box, Style::bgOutgoing, 0);
        lv_obj_add_style(box, Style::outgoingPosition, 0);
    }
    lv_obj_add_style(box, Style::opaque, 0);

    // Message text color - incoming is plain text, outgoing reflects delivery status
    uint32_t textColor = Style::TEXT_PRIMARY; // incoming default
    if (!span.incoming() && !_rrcMode) {
        switch (status) {
            case LXMFStatus::QUEUED:
            case LXMFStatus::SENDING:
            case LXMFStatus::PROP_QUEUED:
            case LXMFStatus::PROP_SENDING:
                textColor = Style::TEXT_SECONDARY;
                break;
            case LXMFStatus::SENT:
            case LXMFStatus::DELIVERED:
                textColor = Style::TEXT_PRIMARY;
                break;
            case LXMFStatus::FAILED:
            case LXMFStatus::PROP_UNAVAILABLE:
            case LXMFStatus::RECIPIENT_UNKNOWN:
            case LXMFStatus::STAMP_UNKNOWN:
            case LXMFStatus::STAMP_COST_HIGH:
            case LXMFStatus::STAMP_FAILED:
            case LXMFStatus::PROP_TOO_LARGE:
            case LXMFStatus::PROP_REJECTED:
            case LXMFStatus::PROP_INVALID:
                textColor = Style::ERROR_CLR;
                break;
            default:
                textColor = Style::TEXT_PRIMARY;
                break;
        }
    }
    if (!_rrcMode && span.hasAudio()) {
        auto *audio = lv_label_create(box);
        lv_obj_add_style(audio, Style::fontBody, 0);
        lv_obj_add_style(audio, Style::textAccent, 0);
        lv_obj_add_style(audio, Style::fullWidth, 0);
        lv_label_set_long_mode(audio, LV_LABEL_LONG_WRAP);
        if (span.nativeAudio() || span.audioRemoved())
            lv_label_set_text(audio, "Voice message");
        else
            lv_label_set_text(audio, span.audioLabel());
    }
    lv_obj_t *lbl = lv_label_create(box);
    lv_obj_add_style(lbl, Style::fontBody, 0);
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(lbl, tone, 0);
        lv_obj_add_style(lbl, Style::textTones[span.unavailable() ? Style::TEXT_MUTED : textColor], 0);
    }
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_add_style(lbl, Style::fullWidth, 0);
    lv_label_set_text(lbl, text);
    if (!_rrcMode && handheld::ui::generatedAudioText(span, text))
        lv_obj_add_flag(lbl, LV_OBJ_FLAG_HIDDEN);

    if (_service && !span.unavailable() && !_rrcMode && span.nativeAudio()) {
        auto *button = _readButtons[index] = lv_btn_create(box);
        {
            lv_obj_remove_style(button, span.hasAudio() ? Style::historyNotice : Style::readMessageAction, 0);
            lv_obj_add_style(button, span.hasAudio() ? Style::historyNotice : Style::readMessageAction, 0);
        }
        {
            lv_obj_remove_style(button, Style::buttonColor, 0);
            lv_obj_add_style(button, Style::buttonColor, 0);
        }
        {
            lv_obj_remove_style(button, Style::button, 0);
            lv_obj_add_style(button, Style::button, 0);
        }
        {
            lv_obj_remove_style(button, Style::noPadding, 0);
            lv_obj_add_style(button, Style::noPadding, 0);
        }
        lv_obj_set_user_data(button, (void *) (uintptr_t) index);
        lv_obj_add_event_cb(button, [](lv_event_t *e) {
            auto* self = static_cast<LvMessageView*>(lv_event_get_user_data(e));
            const auto index=uintptr_t(lv_obj_get_user_data(lv_event_get_target(e)));
            if(index>=HistoryWindow::VisibleSpans || self->_readButtons[index]!=lv_event_get_target(e) || !self->boundWindow()) return;
            auto& window=self->_service->historyWindow();const auto* row=window.span(index);
            if(!row) return;
            const auto code=lv_event_get_code(e);
            if(self->_rrcMode || !row->hasAudio()) {if(code==LV_EVENT_CLICKED) self->readFull(index);return;}
            const auto* memo=self->_ui?self->_ui->memoUi():nullptr;
            const bool owns=handheld::ui::ownsMessageAudio(memo,window.peer(),*row);
            const auto phase=owns?memo->status().phase:handheld::memo::Phase::Unavailable;
            const auto owner=owns?memo->status().view:0;
            if(code==LV_EVENT_PRESSED) {
                self->_audioPressIndex=index;self->_audioPressPublication=window.revision();
                self->_audioPressOwner=owner;self->_audioPressPhase=phase;
            } else if(code==LV_EVENT_PRESS_LOST) self->_audioPressIndex=HistoryWindow::VisibleSpans;
            else if(code==LV_EVENT_CLICKED) {
                const bool valid=self->_audioPressIndex==index && self->_audioPressPublication==window.revision() &&
                    self->_audioPressOwner==owner && self->_audioPressPhase==phase;
                self->_audioPressIndex=HistoryWindow::VisibleSpans;
                if(valid) self->readFull(index);
            } }, LV_EVENT_ALL, this);
        lv_obj_add_flag(button, LV_OBJ_FLAG_EVENT_BUBBLE);
        auto *label = lv_label_create(button);
        lv_label_set_text(label, "");
        lv_obj_add_style(label, Style::fontBody, 0);
        lv_obj_add_style(label, Style::centered, 0);
        if (!_rrcMode && span.hasAudio()) {
            auto *error = _audioErrors[index] = lv_label_create(box);
            {
                lv_obj_remove_style(error, Style::fullWidth, 0);
                lv_obj_add_style(error, Style::fullWidth, 0);
            }
            {
                lv_obj_remove_style(error, Style::fontCaption, 0);
                lv_obj_add_style(error, Style::fontCaption, 0);
            }
            {
                lv_obj_remove_style(error, Style::textError, 0);
                lv_obj_add_style(error, Style::textError, 0);
            }
            lv_label_set_text(error, "");
            lv_obj_add_flag(error, LV_OBJ_FLAG_HIDDEN);
        }
    }

    if (!_rrcMode && span.audioRemoved()) {
        auto *removed = lv_label_create(box);
        lv_label_set_text(removed, "Audio removed");
        lv_obj_add_style(removed, Style::fontBody, 0);
        lv_obj_add_style(removed, Style::textMuted, 0);
    }

    char      timeBuf[8] = { 0 };
    bool      hasTime = formatClock(span.timestamp, _use24h, timeBuf, sizeof(timeBuf));
    bool      needsMeta = hasTime || (!span.incoming() && !span.unavailable());
    lv_obj_t *statusLbl = nullptr;
    if (needsMeta) {
        lv_obj_t *meta = lv_obj_create(box);
        lv_obj_add_style(meta, Style::fullWidth, 0);

        lv_obj_add_style(meta, Style::messageMetadata, 0);

        lv_obj_clear_flag(meta, LV_OBJ_FLAG_SCROLLABLE);

        if (hasTime) {
            lv_obj_t *timeLbl = lv_label_create(meta);
            lv_obj_add_style(timeLbl, Style::fontCaption, 0);
            lv_obj_add_style(timeLbl, Style::textMuted, 0);
            lv_label_set_text(timeLbl, timeBuf);
            lv_obj_add_style(timeLbl, Style::leftCentered, 0);
        }

        if (!span.incoming()) {
            statusLbl = lv_label_create(meta);
            lv_obj_add_style(statusLbl, Style::fontCaption, 0);
            applyStatusGlyph(statusLbl, span);
            lv_obj_add_style(statusLbl, Style::rightCentered, 0);
        }
    }

    _statusLabels[index] = statusLbl;
    _textLabels[index] = lbl;
    _bubbleBoxes[index] = box;
    if (_service && !span.unavailable()) {
        lv_obj_add_flag(box, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(box, reinterpret_cast<void *>(index));
        lv_obj_add_event_cb(box, [](lv_event_t *e) {
            auto& self=*static_cast<LvMessageView*>(lv_event_get_user_data(e));
            const auto index=uintptr_t(lv_obj_get_user_data(lv_event_get_current_target(e)));
            if(index>=self._rowCount || self._bubbleBoxes[index]!=lv_event_get_current_target(e) || !self.boundWindow()) return;
            const auto code=lv_event_get_code(e);
            if(code==LV_EVENT_PRESSED) {
                self._messagePressIndex=index;self._messagePressPublication=self._service->historyWindow().revision();
            } else if(code==LV_EVENT_PRESS_LOST) self._messagePressIndex=HistoryWindow::VisibleSpans;
            else if(code==LV_EVENT_LONG_PRESSED && self._messagePressIndex==index &&
                    self._messagePressPublication==self._service->historyWindow().revision()) {
                self._messagePressIndex=self._audioPressIndex=HistoryWindow::VisibleSpans;
                if(self._rrcMode) self.openRrcTools(index);else self.openMessageTools(index);
            } }, LV_EVENT_ALL, this);
    }
}

void LvMessageView::rebuildMessages() {
    if (!_msgScroll || !_service)
        return;
    auto &window = _service->historyWindow();
    if (!windowMatches() || !window.visible() || !window.statusReady())
        return;
    const bool bottom = _scrollToEnd || (window.mode() == HistoryWindow::Mode::Chat &&
                                         window.followsNewest());
    const auto scroll = window.scrollOffset();
    clearMessages();
    _binding = true;
    _rowCount = window.spanCount();
    for (size_t i = 0; i < _rowCount; ++i)
        appendMessage(i, *window.span(i), window.text(i));
    lv_obj_update_layout(_msgScroll);
    for (size_t i = 0; i < _rowCount; ++i)

        lv_obj_update_layout(_msgScroll);
    lv_obj_scroll_to_y(_msgScroll, bottom ? LV_COORD_MAX : scroll, LV_ANIM_OFF);
    const bool reveal = _selectAfterPage;
    _selectAfterPage = false;
    _lastHistoryRevision = window.revision();
    _lastStatusRevision = window.statusRevision();
    _boundMode = window.mode();
    _boundIdentity = window.identityGeneration();
    _binding = false;
    _scrollToEnd = false;
    saveScroll();
    if (reveal)
        revealSelection();
    // Labels own text copies; publication banks may be reused while the tab is hidden.
    window.acknowledgePublication(_lastHistoryRevision);
}

void LvMessageView::updateMessageStatus(size_t index, const Span &span) {
    if (index >= _rowCount)
        return;
    lv_obj_t *statusLbl = _statusLabels[index];
    lv_obj_t *textLbl = _textLabels[index];
    lv_obj_t *bubbleBox = _bubbleBoxes[index];
    if (!statusLbl)
        return; // Incoming message, no status label

    const auto status = static_cast<LXMFStatus>(span.status);
    const auto previousMetaHeight = lv_obj_get_height(lv_obj_get_parent(statusLbl));
    applyStatusGlyph(statusLbl, span);
    if (bubbleBox) {
        {
            for (auto *tone : Style::borderTones)
                lv_obj_remove_style(bubbleBox, tone, 0);
            lv_obj_add_style(bubbleBox, Style::borderTones[_rrcMode ? Style::BORDER : bubbleBorderColor(status)], 0);
        }
        // History rows pin their height after layout. A transient storage
        // caption can add or remove a line without rebuilding the history.
        if (lv_obj_get_style_height(lv_obj_get_parent(statusLbl), 0) != previousMetaHeight) {
            lv_obj_update_layout(bubbleBox);
        }
    }

    // Update text color to match status
    if (textLbl) {
        uint32_t textColor = Style::TEXT_PRIMARY;
        if (!_rrcMode && (status == LXMFStatus::QUEUED || status == LXMFStatus::SENDING)) {
            textColor = Style::TEXT_SECONDARY;
        } else if (!_rrcMode && status == LXMFStatus::FAILED) {
            textColor = Style::ERROR_CLR;
        }
        {
            for (auto *tone : Style::textTones)
                lv_obj_remove_style(textLbl, tone, 0);
            lv_obj_add_style(textLbl, Style::textTones[textColor], 0);
        }
    }
}

void LvMessageView::applyStatusGlyph(lv_obj_t *lbl, const Span &span) {
    if (!lbl)
        return;
    if (_rrcMode) {
        lv_label_set_text(lbl, handheld::history::rrc::statusName(span.status));
        {
            for (auto *tone : Style::textTones)
                lv_obj_remove_style(lbl, tone, 0);
            lv_obj_add_style(lbl, Style::textTones[span.status == uint8_t(handheld::storage::rrc::Status::Failed) ? Style::ERROR_CLR : span.status == uint8_t(handheld::storage::rrc::Status::Confirmed) ? Style::SUCCESS
                                                                                                                                                                                                         : Style::TEXT_MUTED],
                             0);
        }
        return;
    }
    const char *glyph = messageStatusLabel(static_cast<LXMFStatus>(span.status));
    uint32_t    color;
    switch (static_cast<LXMFStatus>(span.status)) {
        case LXMFStatus::DELIVERED:
            color = Style::SUCCESS;
            break;
        case LXMFStatus::SENT:
            color = Style::TEXT_MUTED;
            break;
        case LXMFStatus::FAILED:
        case LXMFStatus::PROP_UNAVAILABLE:
        case LXMFStatus::RECIPIENT_UNKNOWN:
        case LXMFStatus::STAMP_UNKNOWN:
        case LXMFStatus::STAMP_COST_HIGH:
        case LXMFStatus::STAMP_FAILED:
        case LXMFStatus::PROP_TOO_LARGE:
        case LXMFStatus::PROP_REJECTED:
        case LXMFStatus::PROP_INVALID:
            color = Style::ERROR_CLR;
            break;
        case LXMFStatus::UNCONFIRMED:
        case LXMFStatus::PROP_UNCONFIRMED:
            color = Style::WARNING_CLR;
            break;
        case LXMFStatus::SENDING:
        case LXMFStatus::PROP_SENDING:
            color = Style::WARNING_CLR;
            break;
        case LXMFStatus::QUEUED:
        case LXMFStatus::PROP_QUEUED:
            color = Style::WARNING_CLR;
            break;
        default:
            color = Style::TEXT_MUTED;
            break;
    }
    // A failed refresh does not invalidate the last known delivery state or
    // constitute a failed status write. The window owns its delayed notice.
    const char *detail = span.flags & Span::StatusUnavailable ? nullptr : messageStatusDetail(static_cast<LXMFStatus>(span.status), span.flags & Span::StatusPending, span.statusError, span.flags & Span::TxSuppressed);
    if (detail) {
        char text[48];
        snprintf(text, sizeof(text), "%s\n%s", glyph, detail);
        lv_label_set_text(lbl, text);
    } else {
        lv_label_set_text(lbl, glyph);
    }
    {
        lv_obj_remove_style(lbl, Style::textRight, 0);
        lv_obj_add_style(lbl, Style::textRight, 0);
    }
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(lbl, tone, 0);
        lv_obj_add_style(lbl, Style::textTones[color], 0);
    }
}

void LvMessageView::sendCurrentMessage(bool viaLink) {
    if (_rrcMode) {
        sendRrcMessage();
        return;
    }
    if (!_service || _peerHex.empty() || _inputText.empty() || _sendPending)
        return;
    if (_inputText.size() > MAX_COMPOSER_CHARS) {
        if (_ui)
            _ui->lvStatusBar().showToast("Message too long", 1500);
        return;
    }
    if (_nextDraftRevision == UINT64_MAX)
        return;
    const auto revision = _draftRevision;
    const auto identity = _service->status().generation;
    _retainedDraftPeer = _peerHex;
    _retainedDraft = _inputText;
    _retainedDraftRevision = revision;
    _retainedDraftIdentity = identity;
    _sendPending = true;
    const auto id = _service->action(handheld::Operation::Send, _peerHex, _inputText, viaLink, [this, revision, identity](const handheld::Result &result) {
        _sendPending = false;
        const bool sameView = !_rrcMode && _peerHex == _retainedDraftPeer && _service &&
                              _service->status().generation == identity;
        if (result.outcome == handheld::Outcome::Ok) {
            if (sameView && _draftRevision == revision)
                _inputText.clear();
            if (_retainedDraftRevision == revision && _retainedDraftIdentity == identity) {
                _retainedDraft.clear();
                _retainedDraftPeer.clear();
            }
        }
        // The screen object is app-owned; its widgets are not. Never touch
        // widgets after navigation destroyed this view.
        if (_screen && sameView) {
            updateComposerState();
            updateComposerText();
        }
    });
    if (!id)
        _sendPending = false;
    if (_screen)
        updateComposerState();
    if (id && _ui)
        _ui->lvStatusBar().showToast("Saving message...", 1000);
}

bool LvMessageView::handleKey(const KeyEvent &event) {
    const bool deleting = event.del || event.character == 0x08;
    if (event.repeat && deleting && _dismissedDelete)
        return true;
    if (!event.repeat && deleting)
        _dismissedDelete = false;
    _audioPressIndex = _messagePressIndex = HistoryWindow::VisibleSpans;
    if (_messageTools.visible()) {
        _messagePressedSerial = 0;
        if (event.character == 0x1B || deleting) {
            if (!event.repeat) {
                _dismissedDelete = deleting;
                _messageTools.close();
                hideSendModeMenu();
            }
            return true;
        }
        if (event.up || event.left) {
            _messageTools.move(-1);
            updateSendModeMenu();
            return true;
        }
        if (event.down || event.right || event.tab) {
            _messageTools.move(1);
            updateSendModeMenu();
            return true;
        }
        if (event.enter && !event.repeat)
            activateMessageTool();
        return true;
    }
    if (!_rrcMode && hasReadFocus() && !event.repeat && (event.character == 'm' || event.character == 'M')) {
        openMessageTools(_service->historyWindow().focusedSpan());
        return true;
    }
    if (_rrcMode && _rrcTools.visible()) {
        if (event.character == 0x1B || deleting) {
            if (!event.repeat) {
                _dismissedDelete = deleting;
                closeRrcTools();
            }
            return true;
        }
        if (event.up || event.left) {
            _rrcTools.move(-1);
            updateSendModeMenu();
            return true;
        }
        if (event.down || event.right || event.tab) {
            _rrcTools.move(1);
            updateSendModeMenu();
            return true;
        }
        if ((event.enter || event.character == '\n' || event.character == '\r') && !event.repeat)
            activateRrcTool();
        return true;
    }
    if (boundWindow() && _service->historyWindow().mode() == HistoryWindow::Mode::Full) {
        const auto *row = _service->historyWindow().span(0);
        if (event.character == 0x1B || deleting) {
            if (!event.repeat) {
                _dismissedDelete = deleting;
                goBack();
            }
        } else if (event.up || event.left)
            scrollHistory(-30);
        else if (event.down || event.right)
            scrollHistory(30);
        else if (!event.repeat && (event.enter || event.character == 'm' || event.character == 'M')) {
            if (_rrcMode)
                openRrcTools(0);
            else if (event.enter && row && row->hasAudio())
                readFull(0);
            else
                openMessageTools(0);
        }
        return true; // No key may edit or send the hidden composer in a reader.
    }
    if (_sendOverlay) {
        if (event.character == 0x1B ||
            ((event.del || event.character == 0x08) && !event.repeat)) {
            hideSendModeMenu();
            _dismissedDelete = deleting;
            return true;
        }
        if (event.up || event.left) {
            _sendMenuIdx = (_sendMenuIdx + sendMenuCount() - 1) % sendMenuCount();
            updateSendModeMenu();
            return true;
        }
        if (event.down || event.right || event.tab) {
            _sendMenuIdx = (_sendMenuIdx + 1) % sendMenuCount();
            updateSendModeMenu();
            return true;
        }
        if (event.enter || event.character == '\n' || event.character == '\r') {
            chooseSendMode(_sendMenuIdx);
            return true;
        }
        return true;
    }

#if HAS_SCROLLWHEEL
    if (handleComposerNavigation(event))
        return true;
#endif
    if (event.character == 0x1B) {
        if (!event.repeat)
            goBack();
        return true;
    }

    if (event.del || event.character == 0x08) {
        if (hasReadFocus() || _selectAfterPage) {
            if (!event.repeat) {
                _dismissedDelete = true;
                dismissSelection();
            }
            return true;
        }
        if (!_inputText.empty()) {
            handheld::utf8::eraseLast(_inputText);
            composerEdited();
            updateComposerState();
        } else if (!event.repeat) {
            // Hold-to-repeat stops at empty; only a fresh tap exits the chat.
            goBack();
        }
        return true;
    }

    if (event.enter || event.character == '\n' || event.character == '\r') {
        if (event.repeat)
            return true;
        if (hasReadFocus()) {
            readFull(_service->historyWindow().focusedSpan());
            return true;
        }
        if (event.source != InputSource::Keyboard) {
            selectVisibleMessage();
            return true;
        }
#if !HAS_TOUCH
        // An empty composer lets keyboard-only boards enter Read full
        // selection with Enter; typing returns to composing.
        if (_inputText.empty()) {
            if (_historyNotice && !lv_obj_has_flag(_historyNotice, LV_OBJ_FLAG_HIDDEN)) {
                historyAction(_noticeAction);
                return true;
            }
            focusNextRead();
            if (hasReadFocus() && _ui)
                _ui->lvStatusBar().showToast("Up/Down: select  Enter: open  Back: cancel", 2000);
            return true;
        }
#endif
        sendCurrentMessage(false);
        return true;
    }

    if (event.up || event.down) {
        if (!hasReadFocus() && event.repeat)
            return true;
        if (event.source == InputSource::Trackball && !hasReadFocus())
            scrollHistory(event.up ? -30 : 30);
        else if (boundWindow() && _service->historyWindow().mode() == HistoryWindow::Mode::Chat)
            focusNextRead(event.up ? -1 : 1);
        else
            scrollHistory(event.up ? -30 : 30);
        return true;
    }
    if (event.tab) {
        if (!event.repeat)
            focusNextRead();
        return true;
    }
    if (event.left || event.right)
        return true;

    if (event.character >= 0x20 && event.character < 0x7F) {
        if (_rrcMode && (!_rrcLoaded || !rrcWritable()))
            return true;
        if (_inputText.size() >= (_rrcMode ? handheld::rrc::DraftCapacity : MAX_COMPOSER_CHARS)) {
            if (_ui)
                _ui->lvStatusBar().showToast("Message too long", 900);
            return true;
        }
        unsigned long inputStartMs = millis();
        dismissSelection();
        _inputText += (char) event.character;
        composerEdited();
        updateComposerState();
        unsigned long elapsed = millis() - inputStartMs;
        if (PerfTrace::shouldLog(elapsed, RSDECK_PERF_UI_TRACE_MS)) {
            Serial.printf("[PERF] Chat input: peer=%s chars=%u total=%lums\n", _peerHex.substr(0, 8).c_str(), (unsigned) _inputText.size(), elapsed);
        }
        return true;
    }

    return false;
}

bool LvMessageView::handleLongPress() {
#if HAS_SCROLLWHEEL
    if (_composerAction)
        return true; // A hold must not activate a footer action.
#endif
    if (_messageTools.visible())
        return true;
    if (_rrcTools.visible())
        return true;
    if (boundWindow() && _service->historyWindow().mode() == HistoryWindow::Mode::Full) {
        if (_rrcMode)
            openRrcTools(0);
        else
            openMessageTools(0);
        return true;
    }
    if (hasReadFocus()) {
        const auto focus = _service->historyWindow().focusedSpan();
        if (_rrcMode)
            openRrcTools(focus);
        else
            openMessageTools(focus);
        return true;
    }
    if (_inputText.empty() && sendMenuCount() == 3)
        return false;
    showSendModeMenu();
    return true;
}

void LvMessageView::showSendModeMenu() {
    if (_inputText.empty() && !_rrcTools.visible() && !_messageTools.visible() && sendMenuCount() == 3)
        return;
    hideSendModeMenu();
    _sendMenuIdx = 0;
    _messagePressedSerial = 0;

    _sendOverlay = lv_obj_create(lv_layer_top());
    lv_obj_add_style(_sendOverlay, Style::actionMenu, 0);
    lv_obj_add_style(_sendOverlay, Style::centered, 0);
    lv_obj_add_style(_sendOverlay, Style::modalColor, 0);
    lv_obj_add_style(_sendOverlay, Style::modal, 0);
    lv_obj_add_style(_sendOverlay, Style::actionMenuPadding, 0);
    if (!_rrcTools.visible())
        lv_obj_clear_flag(_sendOverlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(_sendOverlay);
    lv_obj_add_style(title, Style::fontBody, 0);
    lv_obj_add_style(title, Style::textAccent, 0);
    lv_label_set_text(title, _rrcTools.visible() ? _rrcTools.title() : _messageTools.visible() ? _messageTools.title()
                                                                                               : "Chat actions");
    lv_obj_add_style(title, Style::topCentered, 0);

    const bool confirmation = _messageTools.state == handheld::ui::MessageMenu::State::Confirm;
    if (_messageTools.visible()) {
        auto *hint = lv_label_create(_sendOverlay);
        lv_label_set_text(hint, _messageTools.hint());
        lv_obj_add_style(hint, Style::actionMenuHint, 0);
        lv_obj_add_style(hint, Style::fontCaption, 0);
        lv_obj_add_style(hint, Style::textSecondary, 0);
    }
    auto *actions = lv_obj_create(_sendOverlay);
    lv_obj_add_style(actions, Style::menuActions, 0);
    if (confirmation)
        lv_obj_add_style(actions, Style::confirmationActions, 0);
    lv_obj_clear_flag(actions, LV_OBJ_FLAG_SCROLLABLE);
    static const char *labels[3] = { "Send normally", "Send as link", "Cancel" };
    for (int i = 0; i < (_rrcTools.visible() ? int(_rrcTools.count()) : sendMenuCount()); i++) {
        lv_obj_t *row = lv_obj_create(actions);
        lv_obj_add_style(row, confirmation ? Style::confirmationAction : Style::menuAction, 0);

        lv_obj_add_style(row, Style::actionSurface, 0);

        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(row, (void *) (intptr_t) i);
        lv_obj_add_event_cb(row, [](lv_event_t *e) {
            auto* self = (LvMessageView*)lv_event_get_user_data(e);
            int idx = (int)(intptr_t)lv_obj_get_user_data(lv_event_get_target(e));
            if(idx<0 || idx>=8 || self->_sendRows[idx]!=lv_event_get_target(e)) return;
            if(self->_messageTools.visible()) {
                if(lv_event_get_code(e)==LV_EVENT_PRESSED) self->_messagePressedSerial=self->_messageTools.serial;
                else if(lv_event_get_code(e)==LV_EVENT_PRESS_LOST) self->_messagePressedSerial=0;
                else if(lv_event_get_code(e)==LV_EVENT_CLICKED && self->_messagePressedSerial==self->_messageTools.serial) {
                    self->_messagePressedSerial=0;self->_messageTools.selected=uint8_t(idx);self->activateMessageTool();
                }
            } else if(lv_event_get_code(e)==LV_EVENT_CLICKED) self->chooseSendMode(idx); }, LV_EVENT_ALL, this);

        _sendLabels[i] = lv_label_create(row);
        lv_obj_add_style(_sendLabels[i], Style::fontBody, 0);
        if (_rrcTools.visible()) {
            char label[64];
            _rrcTools.label(i, label, sizeof label);
            lv_label_set_text(_sendLabels[i], label);
        } else if (_messageTools.visible())
            lv_label_set_text(_sendLabels[i], _messageTools.label(i));
        else
            lv_label_set_text(_sendLabels[i], i == sendMenuCount() - 1 ? "Cancel" : sendMenuCount() == 4 && i == 2 ? "Voice message"
                                                                                : _rrcMode && i == 1               ? "Send as action"
                                                                                                                   : labels[i]);
        lv_obj_add_style(_sendLabels[i], Style::centered, 0);
        _sendRows[i] = row;
    }

    updateSendModeMenu();
}

void LvMessageView::hideSendModeMenu() {
    if (_sendOverlay) {
        lv_obj_del_async(_sendOverlay);
        _sendOverlay = nullptr;
    }
    for (int i = 0; i < 8; i++) {
        _sendRows[i] = nullptr;
        _sendLabels[i] = nullptr;
    }
}

void LvMessageView::updateSendModeMenu() {
    for (int i = 0; i < 8; i++) {
        if (!_sendRows[i] || !_sendLabels[i])
            continue;
        bool selected = _messageTools.visible() ? i == _messageTools.selected : i == _sendMenuIdx;
        if (_rrcTools.visible()) {
            char label[64];
            _rrcTools.label(i, label, sizeof label);
            lv_label_set_text(_sendLabels[i], label);
            selected = i == _rrcTools.selected;
        }
        {
            for (auto *tone : Style::bgTones)
                lv_obj_remove_style(_sendRows[i], tone, 0);
            lv_obj_add_style(_sendRows[i], Style::bgTones[selected ? Style::PRIMARY_SUBTLE : Style::BG_SURFACE], 0);
        }
        {
            for (auto *tone : Style::borderTones)
                lv_obj_remove_style(_sendRows[i], tone, 0);
            lv_obj_add_style(_sendRows[i], Style::borderTones[selected ? Style::BORDER_ACTIVE : Style::BORDER], 0);
        }
        {
            for (auto *tone : Style::textTones)
                lv_obj_remove_style(_sendLabels[i], tone, 0);
            lv_obj_add_style(_sendLabels[i], Style::textTones[_messageTools.state == handheld::ui::MessageMenu::State::Confirm && !i ? Style::ERROR_CLR : selected ? Style::ACCENT
                                                                                                                                                                   : Style::TEXT_SECONDARY],
                             0);
        }
    }
    if (_rrcTools.visible() && _sendRows[_rrcTools.selected])
        lv_obj_scroll_to_view(_sendRows[_rrcTools.selected], LV_ANIM_OFF);
}

void LvMessageView::chooseSendMode(int idx) {
    if (_messageTools.visible()) {
        if (idx >= 0 && unsigned(idx) < _messageTools.count()) {
            _messageTools.selected = uint8_t(idx);
            activateMessageTool();
        }
        return;
    }
    if (_rrcTools.visible()) {
        if (idx < 0 || size_t(idx) >= _rrcTools.count())
            return;
        _rrcTools.selected = uint8_t(idx);
        activateRrcTool();
        return;
    }
    bool viaLink = idx == 1;
    if (idx == sendMenuCount() - 1) {
        hideSendModeMenu();
        return;
    }
    hideSendModeMenu();
    if (sendMenuCount() == 4 && idx == 2) {
        if (_ui)
            _ui->openVoice(_peerHex.c_str());
        return;
    }
    if (_rrcMode)
        _rrcEmote = idx == 1;
    sendCurrentMessage(viaLink);
    if (_rrcMode)
        _rrcEmote = false;
}

#include "LvMessageTools.inc"
#include "LvRrcChat.inc"
