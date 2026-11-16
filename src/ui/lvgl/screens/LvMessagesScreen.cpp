#include "LvMessagesScreen.h"
#include "ui/FeatureAvailability.h"
#include "Style.h"
#include "ClockText.h"
#include "LvInput.h"
#include "PageNavigation.h"
#include "LxmFaceAvatar.h"
#include "reticulum/MessageStatusDetail.h"
#include "util/PerfTrace.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <ctime>
#include <limits>

namespace {
void peerText(const uint8_t *peer, char (&out)[33]) {
    constexpr char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < 16; ++i) {
        out[2 * i] = hex[peer[i] >> 4];
        out[2 * i + 1] = hex[peer[i] & 15];
    }
    out[32] = 0;
}
const char *statusText(uint8_t status) {
    switch (static_cast<LXMFStatus>(status)) {
        case LXMFStatus::QUEUED:
            return "QUEUED";
        case LXMFStatus::SENDING:
            return "SENDING";
        case LXMFStatus::SENT:
            return "SENT";
        case LXMFStatus::DELIVERED:
            return "DELIVERED";
        case LXMFStatus::FAILED:
            return "FAILED";
        case LXMFStatus::UNCONFIRMED:
            return "UNCONFIRMED";
        case LXMFStatus::PROP_QUEUED:
            return "PROP QUEUED";
        case LXMFStatus::PROP_SENDING:
            return "PROP SENDING";
        case LXMFStatus::PROPAGATED:
            return "PROPAGATED";
        case LXMFStatus::PROP_UNAVAILABLE:
            return "PROP UNAVAILABLE";
        case LXMFStatus::PROP_UNCONFIRMED:
            return "PROP UNCONFIRMED";
        case LXMFStatus::RECIPIENT_UNKNOWN:
            return "RECIPIENT UNKNOWN";
        case LXMFStatus::STAMP_UNKNOWN:
            return "STAMP UNKNOWN";
        case LXMFStatus::STAMP_COST_HIGH:
            return "STAMP COST HIGH";
        case LXMFStatus::STAMP_FAILED:
            return "STAMP FAILED";
        case LXMFStatus::PROP_TOO_LARGE:
            return "PROP TOO LARGE";
        case LXMFStatus::PROP_REJECTED:
            return "PROP REJECTED";
        case LXMFStatus::PROP_INVALID:
            return "PROP INVALID";
        default:
            return "";
    }
}

}

bool LvMessagesScreen::bound() const {
    if (!_active || !_screen || !_service || !_rrc.navigation.direct())
        return false;
    const auto &window = _service->conversationWindow();
    return window.visible() && window.statusReady() && _boundRevision == window.revision() &&
           _boundIdentity == window.identityGeneration() && _boundIdentity == _service->status().generation;
}
void LvMessagesScreen::notice(const char *text) {
    if (_ui)
        _ui->lvStatusBar().showToast(text, 1500);
}
void LvMessagesScreen::createUI(lv_obj_t *parent) {
    _screen = parent;
    {
        lv_obj_remove_style(parent, Style::absoluteLayout, 0);
        lv_obj_add_style(parent, Style::absoluteLayout, 0);
    }
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    {
        lv_obj_remove_style(parent, Style::listColor, 0);
        lv_obj_add_style(parent, Style::listColor, 0);
    }
    if (handheld::ui::RrcEnabled)
        for (size_t n = 0; n < 2; ++n) {
            auto *button = _family[n] = lv_btn_create(parent);
            lv_obj_add_style(button, n ? Style::familyRight : Style::familyLeft, 0);
            lv_obj_add_style(button, Style::listItemColor, 0);
            lv_obj_add_style(button, Style::listItem, 0);
            lv_obj_add_style(button, Style::listItemFocusedColor, LV_STATE_FOCUSED);
            lv_obj_add_style(button, Style::listItemFocused, LV_STATE_FOCUSED);
            lv_obj_add_style(button, Style::noPadding, 0);

            auto *text = lv_label_create(button);
            lv_obj_add_style(text, Style::fontBody, 0);
            lv_obj_add_style(text, Style::textPrimary, 0);
            lv_obj_add_style(text, Style::familyCaption, 0);
            lv_label_set_long_mode(text, LV_LABEL_LONG_CLIP);

            lv_label_set_text(text, n ? "Hubs" : "Direct");
            lv_obj_set_user_data(button, reinterpret_cast<void *>(n));
            lv_obj_add_event_cb(button, [](lv_event_t *e) { static_cast<LvMessagesScreen *>(lv_event_get_user_data(e))->switchFamily(!reinterpret_cast<uintptr_t>(lv_obj_get_user_data(lv_event_get_target(e)))); }, LV_EVENT_CLICKED, this);
            lv_group_add_obj(LvInput::group(), button);
        }
    _list = lv_obj_create(parent);
    lv_obj_add_style(_list, handheld::ui::RrcEnabled ? Style::conversationFamilyList : Style::conversationList, 0);
    lv_obj_add_style(_list, Style::listColor, 0);
    lv_obj_add_style(_list, Style::list, 0);
    lv_obj_add_style(_list, Style::scrollbarColor, LV_PART_SCROLLBAR);
    lv_obj_add_style(_list, Style::scrollbar, LV_PART_SCROLLBAR);
    lv_obj_add_style(_list, Style::column, 0);
    lv_obj_add_event_cb(_list, [](lv_event_t *event) {
        auto* self=static_cast<LvMessagesScreen*>(lv_event_get_user_data(event));
        // Programmatic layout/restore must never turn an anchored view into a
        // following view. Pointer, wheel and keyboard scrolling have an indev.
        if (lv_indev_get_act()) self->reportViewport(); }, LV_EVENT_SCROLL, this);

    _empty = lv_label_create(parent);
    lv_obj_add_style(_empty, Style::fontTitle, 0);
    lv_obj_add_style(_empty, Style::textSecondary, 0);
    lv_obj_add_style(_empty, Style::emptyDescription, 0);
    lv_label_set_long_mode(_empty, LV_LABEL_LONG_CLIP);

    // Match Contacts' centered icon/title/hint composition.
    _emptyState = lv_obj_create(parent);
    lv_obj_add_style(_emptyState, Style::emptyState, 0);

    lv_obj_clear_flag(_emptyState, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    auto *bubble = lv_obj_create(_emptyState);
    lv_obj_add_style(bubble, Style::conversationIcon, 0);

    lv_obj_add_style(bubble, Style::textareaFocusedColor, 0);

    lv_obj_clear_flag(bubble, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    static const lv_point_t tailPoints[] = {
        { 0, 0 },
        { 0, 8 },
        { 8, 0 }
    };
    auto *tail = lv_line_create(_emptyState);
    lv_line_set_points(tail, tailPoints, 3);
    lv_obj_add_style(tail, Style::conversationIconTail, 0);
    lv_obj_add_style(tail, Style::linePrimary, 0);
    for (int i = 0; i < 3; ++i) {
        auto       *dot = lv_obj_create(_emptyState);
        lv_style_t *positions[] = { Style::messageDot0, Style::messageDot1, Style::messageDot2 };
        lv_obj_add_style(dot, positions[i], 0);
        lv_obj_add_style(dot, Style::conversationIconDot, 0);

        lv_obj_add_style(dot, Style::barIndicatorColor, 0);

        lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    }

    auto *emptyStateCaption2 = lv_label_create(_emptyState);
    lv_obj_add_style(emptyStateCaption2, Style::fontTitle, 0);
    lv_obj_add_style(emptyStateCaption2, Style::textSecondary, 0);
    lv_obj_add_style(emptyStateCaption2, Style::emptyTitle, 0);
    lv_label_set_long_mode(emptyStateCaption2, LV_LABEL_LONG_CLIP);

    lv_label_set_text_static(emptyStateCaption2, "No conversations");

    auto *emptyStateCaption = lv_label_create(_emptyState);
    lv_obj_add_style(emptyStateCaption, Style::fontCaption, 0);
    lv_obj_add_style(emptyStateCaption, Style::textMuted, 0);
    lv_obj_add_style(emptyStateCaption, Style::emptyHint, 0);
    lv_label_set_long_mode(emptyStateCaption, LV_LABEL_LONG_CLIP);

    lv_label_set_text_static(emptyStateCaption, "Your chats appear here");

    auto *navigation = lv_obj_create(parent);
    lv_obj_add_style(navigation, Style::pageNavigation, 0);
    lv_obj_clear_flag(navigation, LV_OBJ_FLAG_SCROLLABLE);
    _update = lv_btn_create(navigation);
    lv_obj_add_style(_update, Style::recoveryAction, 0);
    lv_obj_add_style(_update, Style::listItemColor, 0);
    lv_obj_add_style(_update, Style::listItem, 0);
    lv_obj_add_style(_update, Style::listItemFocusedColor, LV_STATE_FOCUSED);
    lv_obj_add_style(_update, Style::listItemFocused, LV_STATE_FOCUSED);
    lv_obj_add_style(_update, Style::noPadding, 0);

    auto *updateText = lv_label_create(_update);
    lv_obj_add_style(updateText, Style::fontCaption, 0);
    lv_obj_add_style(updateText, Style::textPrimary, 0);
    lv_obj_add_style(updateText, Style::buttonCaption, 0);
    lv_label_set_long_mode(updateText, LV_LABEL_LONG_CLIP);

    lv_obj_add_event_cb(_update, [](lv_event_t *event) { static_cast<LvMessagesScreen *>(lv_event_get_user_data(event))->applyUpdate(); }, LV_EVENT_CLICKED, this);
    lv_group_add_obj(LvInput::group(), _update);
    for (size_t i = 0; i < 4; ++i) {
        auto *button = lv_btn_create(navigation);
        _navigation[i] = button;

        lv_obj_add_style(button, Style::pageNavigationButton, 0);
        lv_obj_add_style(button, Style::textMuted, LV_STATE_DISABLED);
        lv_obj_add_style(button, Style::listItemColor, 0);
        lv_obj_add_style(button, Style::listItem, 0);
        lv_obj_add_style(button, Style::listItemFocusedColor, LV_STATE_FOCUSED);
        lv_obj_add_style(button, Style::listItemFocused, LV_STATE_FOCUSED);
        lv_obj_add_style(button, Style::noPadding, 0);
        lv_obj_set_user_data(button, reinterpret_cast<void *>(i));
        lv_obj_add_event_cb(button, handheld::drawPageNavigation, LV_EVENT_DRAW_MAIN, nullptr);
        lv_obj_add_event_cb(button, [](lv_event_t *event) {
            auto* self=static_cast<LvMessagesScreen*>(lv_event_get_user_data(event));
            self->navigate(static_cast<Navigation>(reinterpret_cast<uintptr_t>(lv_obj_get_user_data(lv_event_get_target(event))))); }, LV_EVENT_CLICKED, this);
        lv_group_add_obj(LvInput::group(), button);
    }
    lv_obj_move_to_index(_update, 2);
    updateCaptions();
}
void LvMessagesScreen::onEnter() {
    _active = true;
    if (!handheld::ui::RrcEnabled)
        _rrc.navigation.direct(true);
    _rrc.visible(handheld::ui::RrcEnabled);
    _rrc.notice = [this](const char *text) { notice(text); };
    _rrc.openDirect = [this](const char *peer) {if(_onOpen) _onOpen(peer); };
    _rrc.send = [this](const handheld::rrc::Command &command, const uint8_t *body, size_t length) {
        return _service && _service->rrcCommand(command, body, length) ? handheld::rrc::Code::Ok : handheld::rrc::Code::Busy;
    };
    _loadStarted = millis();
    _loadPending = true;
#if HAS_TOUCH
    _focusActive = false;
#endif
    hideActionMenu();
    if (_service && _rrc.navigation.direct())
        _service->watchConversations();
    refreshUI();
}
void LvMessagesScreen::detachRows() {
    _binding = true;
    if (_screen && _list)
        lv_obj_clean(_list);
    for (auto &row : _rows)
        row = {};
    _rowCount = 0;
    _boundRevision = 0;
    _boundIdentity = 0;
    _namesResolved = 0;
    _nameFailed = false;
    _binding = false;
}
void LvMessagesScreen::onHide() {
    if (bound())
        reportViewport();
    _active = false;
    _rrc.visible(false);
    hideActionMenu();
    if (_service)
        _service->pauseConversations(true);
}

void LvMessagesScreen::onShow() {
    _active = true;
    _rrc.visible(handheld::ui::RrcEnabled);
    if (_service) {
        auto &window = _service->conversationWindow();
        if (window.state() == handheld::history::ConversationList::State::Closed ||
            window.identityGeneration() != _service->status().generation) {
            if (_rrc.navigation.direct())
                _service->watchConversations();
        }
        _service->pauseConversations(false);
    }
    refreshUI();
}

void LvMessagesScreen::onExit() {
    if (bound())
        reportViewport();
    _active = false;
    _rrc.visible(false);
    clearRrc();
    hideActionMenu();
    detachRows();
    if (_service) {
        auto &window = _service->conversationWindow();
        _service->closeConversations();
        window.acknowledgePublication(window.revision());
    }
}
void LvMessagesScreen::destroyUI() {
    _list = nullptr;
    _empty = nullptr;
    _update = nullptr;
    _emptyState = nullptr;
    for (auto &button : _navigation)
        button = nullptr;
    for (auto &button : _family)
        button = nullptr;
    LvScreen::destroyUI();
}
void LvMessagesScreen::refreshUI() {
    if (!_screen || !_service)
        return;
    if (handheld::ui::RrcEnabled) {
        pollRrc();
        lv_label_set_text(lv_obj_get_child(_family[1], 0), _rrc.navigation.secondLabel());
        for (size_t n = 0; n < 2; ++n) {
            for (auto *tone : Style::bgTones)
                lv_obj_remove_style(_family[n], tone, 0);
            lv_obj_add_style(_family[n], Style::bgTones[(_rrc.navigation.direct() == (n == 0)) ? Style::PRIMARY_SUBTLE : Style::BG], 0);
        }
    }
    if (!_rrc.navigation.direct()) {
        renderRrc();
        return;
    }
    auto &window = _service->conversationWindow();
    if (!_active || !window.visible() || window.identityGeneration() != _service->status().generation) {
        hideActionMenu();
        detachRows();
        window.acknowledgePublication(window.revision());
    } else if (window.statusReady() && (_boundRevision != window.revision() || _boundIdentity != window.identityGeneration())) {
        hideActionMenu();
        bindRows();
    }
    if (bound() && _statusRevision != window.statusRevision())
        updateStatuses();
    if (bound() && _nodeRevision != (_am ? _am->revision() : 0))
        updateNames();
    if (bound() && !window.loading() && _lpState == LP_NONE)
        pollName();
    updateCaptions();
    if (_loadPending && bound() && !window.loading()) {
        _loadPending = false;
        const auto elapsed = PerfTrace::elapsedMs(_loadStarted);
        if (PerfTrace::shouldLog(elapsed, RSDECK_PERF_UI_TRACE_MS))
            Serial.printf("[PERF] Chats page=%u rows=%u ready in %lums\n", unsigned(_boundPage), unsigned(_rowCount), elapsed);
    }
}
void LvMessagesScreen::bindRows() {
    auto &window = _service->conversationWindow();
    detachRows();
    _binding = true;
    _boundRevision = window.revision();
    _boundIdentity = window.identityGeneration();
    _boundPage = window.pageNumber();
    _nodeRevision = _am ? _am->revision() : 0;
    _rowCount = static_cast<uint8_t>(window.count());
    for (size_t i = 0; i < _rowCount; ++i) {
        const auto &value = *window.row(i);
        auto       &widgets = _rows[i];
        char        peer[33];
        peerText(value.peer, peer);
        widgets.row = lv_obj_create(_list);
        auto *row = widgets.row;
        {
            lv_obj_remove_style(row, Style::conversationRow, 0);
            lv_obj_add_style(row, Style::conversationRow, 0);
        }
        {
            lv_obj_remove_style(row, Style::listItemColor, 0);
            lv_obj_add_style(row, Style::listItemColor, 0);
        }
        {
            lv_obj_remove_style(row, Style::listItem, 0);
            lv_obj_add_style(row, Style::listItem, 0);
        }
        {
            lv_obj_remove_style(row, Style::listItemFocusedColor, LV_STATE_FOCUSED);
            lv_obj_add_style(row, Style::listItemFocusedColor, LV_STATE_FOCUSED);
        }
        {
            lv_obj_remove_style(row, Style::listItemFocused, LV_STATE_FOCUSED);
            lv_obj_add_style(row, Style::listItemFocused, LV_STATE_FOCUSED);
        }
        {
            lv_obj_remove_style(row, Style::noPadding, 0);
            lv_obj_add_style(row, Style::noPadding, 0);
        }
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        {
            for (auto *tone : Style::bgTones)
                lv_obj_remove_style(row, tone, 0);
            lv_obj_add_style(row, Style::bgTones[value.unreadCount ? Style::PRIMARY_SUBTLE : Style::BG], 0);
        }
        lv_obj_set_user_data(row, reinterpret_cast<void *>(i));
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        // The first child is the existing unread/pending/contact status rail.
        auto *rail = lv_obj_create(row);
        {
            lv_obj_remove_style(rail, Style::conversationAccent, 0);
            lv_obj_add_style(rail, Style::conversationAccent, 0);
        }

        lv_obj_clear_flag(rail, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(row, [](lv_event_t *event) {
            auto* self=static_cast<LvMessagesScreen*>(lv_event_get_user_data(event));
            self->openRow(reinterpret_cast<uintptr_t>(lv_obj_get_user_data(lv_event_get_target(event)))); }, LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(row, [](lv_event_t *event) {
            auto* self=static_cast<LvMessagesScreen*>(lv_event_get_user_data(event));
            self->showActionMenu(reinterpret_cast<uintptr_t>(lv_obj_get_user_data(lv_event_get_target(event)))); }, LV_EVENT_LONG_PRESSED, this);
        lv_obj_add_event_cb(row, [](lv_event_t *event) {
            auto* self=static_cast<LvMessagesScreen*>(lv_event_get_user_data(event));
            if (self->_binding || !self->bound()) return;
            auto* target=lv_event_get_target(event);
            self->_service->conversationWindow().select(reinterpret_cast<uintptr_t>(lv_obj_get_user_data(target)));
            lv_obj_scroll_to_view(target,LV_ANIM_OFF);self->reportViewport(); }, LV_EVENT_FOCUSED, this);
        lv_group_add_obj(LvInput::group(), row);
        // LVGL owns each bounded avatar buffer until its canvas is deleted.
        lv_style_value_t avatarWidth;
        lv_style_get_prop(Style::conversationAvatar, LV_STYLE_WIDTH, &avatarWidth);
        const lv_coord_t AvatarSize = avatarWidth.num;
        if (auto *pixels = lv_mem_alloc(LxmFaceAvatar::bufferSize(AvatarSize))) {
            LxmFaceAvatar::View avatar;
            avatar.box = lv_obj_create(row);
            {
                lv_obj_remove_style(avatar.box, Style::conversationAvatar, 0);
                lv_obj_add_style(avatar.box, Style::conversationAvatar, 0);
            }
            {
                lv_obj_remove_style(avatar.box, Style::avatarFrame, 0);
                lv_obj_add_style(avatar.box, Style::avatarFrame, 0);
            }
            lv_obj_add_style(avatar.box, Style::bgTones[Style::PRIMARY_SUBTLE], 0);
            lv_obj_add_style(avatar.box, Style::borderTones[value.unreadCount ? Style::PRIMARY : Style::BORDER], 0);
            lv_obj_clear_flag(avatar.box, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
            avatar.canvas = lv_canvas_create(avatar.box);
            lv_canvas_set_buffer(avatar.canvas, pixels, AvatarSize, AvatarSize, LV_IMG_CF_TRUE_COLOR_ALPHA);
            {
                lv_obj_remove_style(avatar.canvas, Style::avatarCanvas, 0);
                lv_obj_add_style(avatar.canvas, Style::avatarCanvas, 0);
            }
            lv_img_set_antialias(avatar.canvas, false);
            lv_obj_clear_flag(avatar.canvas, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

            lv_obj_add_event_cb(avatar.canvas, [](lv_event_t *event) { lv_mem_free(lv_event_get_user_data(event)); }, LV_EVENT_DELETE, pixels);
            LxmFaceAvatar::render(avatar.canvas, String(peer));
        }

        widgets.name = lv_label_create(row);
        {
            lv_obj_remove_style(widgets.name, Style::fontTitle, 0);
            lv_obj_add_style(widgets.name, Style::fontTitle, 0);
        }
        {
            lv_obj_remove_style(widgets.name, Style::textPrimary, 0);
            lv_obj_add_style(widgets.name, Style::textPrimary, 0);
        }
        {
            lv_obj_remove_style(widgets.name, Style::conversationName, 0);
            lv_obj_add_style(widgets.name, Style::conversationName, 0);
        }
        lv_label_set_long_mode(widgets.name, LV_LABEL_LONG_CLIP);

        // DOT only truncates after exhausting the label's height; keep the preview's line clear.

        lv_label_set_long_mode(widgets.name, LV_LABEL_LONG_DOT);
        const auto *node = _am ? _am->findNodeByHex(peer) : nullptr;
        if (node && !node->name.empty()) {
            lv_label_set_text(widgets.name, node->name.c_str());
            _namesResolved |= uint64_t { 1 } << i;
        } else {
            char fallback[13];
            std::memcpy(fallback, peer, 12);
            fallback[12] = 0;
            lv_label_set_text(widgets.name, fallback);
        }
        if (value.timestamp > 1700000000 && value.timestamp < static_cast<double>(std::numeric_limits<time_t>::max())) {
            const time_t stamp = static_cast<time_t>(value.timestamp);
            struct tm    result;
            if (localtime_r(&stamp, &result)) {
                char clock[8];
                formatClockText(clock, sizeof(clock), result, _use24h);

                auto *time = lv_label_create(row);
                {
                    lv_obj_remove_style(time, Style::fontCaption, 0);
                    lv_obj_add_style(time, Style::fontCaption, 0);
                }
                {
                    lv_obj_remove_style(time, Style::textMuted, 0);
                    lv_obj_add_style(time, Style::textMuted, 0);
                }
                {
                    lv_obj_remove_style(time, Style::conversationTime, 0);
                    lv_obj_add_style(time, Style::conversationTime, 0);
                }
                lv_label_set_long_mode(time, LV_LABEL_LONG_CLIP);

                lv_label_set_text(time, clock);
            }
        }

        widgets.preview = lv_label_create(row);
        {
            lv_obj_remove_style(widgets.preview, Style::fontBody, 0);
            lv_obj_add_style(widgets.preview, Style::fontBody, 0);
        }
        {
            lv_obj_remove_style(widgets.preview, Style::textSecondary, 0);
            lv_obj_add_style(widgets.preview, Style::textSecondary, 0);
        }
        {
            lv_obj_remove_style(widgets.preview, Style::conversationPreview, 0);
            lv_obj_add_style(widgets.preview, Style::conversationPreview, 0);
        }
        lv_label_set_long_mode(widgets.preview, LV_LABEL_LONG_CLIP);

        // Retained widgets own their text even while the service publishes a new bank.
        lv_label_set_text(widgets.preview, (value.flags & Row::Unavailable) ? "Message unavailable" : value.previewLength ? value.preview
                                                                                                                          : "No preview");
        if (value.flags & Row::PreviewTruncated) {
            {
                lv_obj_remove_style(widgets.preview, Style::truncatedPreview, 0);
                lv_obj_add_style(widgets.preview, Style::truncatedPreview, 0);
            }

            auto *more = lv_label_create(row);
            {
                lv_obj_remove_style(more, Style::fontBody, 0);
                lv_obj_add_style(more, Style::fontBody, 0);
            }
            {
                lv_obj_remove_style(more, Style::textMuted, 0);
                lv_obj_add_style(more, Style::textMuted, 0);
            }
            {
                lv_obj_remove_style(more, Style::previewContinuation, 0);
                lv_obj_add_style(more, Style::previewContinuation, 0);
            }
            lv_label_set_long_mode(more, LV_LABEL_LONG_CLIP);

            lv_label_set_text_static(more, "...");
        }

        widgets.status = lv_label_create(row);
        {
            lv_obj_remove_style(widgets.status, Style::fontCaption, 0);
            lv_obj_add_style(widgets.status, Style::fontCaption, 0);
        }
        {
            lv_obj_remove_style(widgets.status, Style::textMuted, 0);
            lv_obj_add_style(widgets.status, Style::textMuted, 0);
        }
        {
            lv_obj_remove_style(widgets.status, Style::conversationStatus, 0);
            lv_obj_add_style(widgets.status, Style::conversationStatus, 0);
        }
        lv_label_set_long_mode(widgets.status, LV_LABEL_LONG_CLIP);
    }
    updateStatuses();
    lv_obj_update_layout(_list);
    const auto selected = window.selectedIndex();
    if (selected < _rowCount)
        lv_group_focus_obj(_rows[selected].row);
    lv_obj_scroll_to_y(_list, window.scrollOffset(), LV_ANIM_OFF);
#if HAS_TOUCH
    if (!_focusActive) {
        auto *focused = lv_group_get_focused(LvInput::group());
        if (focused)
            lv_obj_clear_state(focused, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
    }
#endif
    _binding = false;
    window.acknowledgePublication(_boundRevision);
}
void LvMessagesScreen::updateStatuses() {
    if (!_service)
        return;
    const auto &window = _service->conversationWindow();
    for (size_t i = 0; i < _rowCount; ++i) {
        const auto *value = window.row(i);
        if (!value)
            continue;
        // The status fields describe the last outgoing record, which may be
        // older than this preview. Never attach them to a newer received message.
        const bool  outgoing = (value->flags & Row::HasOutgoing) && !(value->flags & Row::LastIncoming);
        char        text[128] = {};
        const char *detail = nullptr;
        if (value->flags & Row::Unavailable)
            detail = "Read failed; tap Retry";
        else if (!outgoing || (value->flags & Row::StatusUnavailable))
            detail = nullptr;
        else
            detail = messageStatusDetail(static_cast<LXMFStatus>(value->status), value->flags & Row::StatusPending, value->error, value->flags & Row::TxSuppressed);
        const char *state = outgoing ? statusText(value->status) : "";
        if (detail)
            snprintf(text, sizeof(text), "%s%s%s", state, *state ? " · " : "", detail);
        else if (value->unreadCount)
            snprintf(text, sizeof(text), "%u new%s%s", unsigned(value->unreadCount), *state ? " · " : "", state);
        else
            snprintf(text, sizeof(text), "%s", state);
        lv_label_set_text(_rows[i].status, text);
        const auto color = (value->flags & Row::Unavailable) ? Style::WARNING_CLR : outgoing && value->status == static_cast<uint8_t>(LXMFStatus::FAILED)  ? Style::ERROR_CLR
                                                                                : outgoing && value->status == static_cast<uint8_t>(LXMFStatus::DELIVERED) ? Style::SUCCESS
                                                                                                                                                           : Style::TEXT_MUTED;
        {
            for (auto *tone : Style::textTones)
                lv_obj_remove_style(_rows[i].status, tone, 0);
            lv_obj_add_style(_rows[i].status, Style::textTones[color], 0);
        }
        char peer[33];
        peerText(value->peer, peer);
        const auto *node = _am ? _am->findNodeByHex(peer) : nullptr;
        const auto  rail = (value->flags & Row::Unavailable) ? Style::WARNING_CLR : outgoing && value->status == static_cast<uint8_t>(LXMFStatus::FAILED) ? Style::ERROR_CLR
                                                                                : outgoing && value->pendingCount                                         ? Style::WARNING_CLR
                                                                                : value->unreadCount                                                      ? Style::PRIMARY
                                                                                : node && node->saved                                                     ? Style::TEXT_SECONDARY
                                                                                : node                                                                    ? Style::TEXT_MUTED
                                                                                                                                                          : Style::BORDER;
        {
            for (auto *tone : Style::bgTones)
                lv_obj_remove_style(lv_obj_get_child(_rows[i].row, 0), tone, 0);
            lv_obj_add_style(lv_obj_get_child(_rows[i].row, 0), Style::bgTones[rail], 0);
        }
    }
    _statusRevision = window.statusRevision();
}
void LvMessagesScreen::updateCaptions() {
    if (!_screen)
        return;
    const auto *window = _service ? &_service->conversationWindow() : nullptr;
    const bool  emptyReady = window && _active && !_rowCount && window->state() == Window::State::Ready &&
                            !window->loading() && window->statusReady();
    if (emptyReady)
        lv_obj_clear_flag(_emptyState, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(_emptyState, LV_OBJ_FLAG_HIDDEN);
    if (!_rowCount && !emptyReady) {
        const char *empty = !window || !_active ? "" : window->state() == Window::State::Retrying || window->state() == Window::State::Exhausted ? "Unable to read conversations"
                                                   : window->loading() || !window->statusReady()                                                 ? "Loading conversations..."
                                                                                                                                                 : "No conversations";
        lv_label_set_text(_empty, empty);
        lv_obj_clear_flag(_empty, LV_OBJ_FLAG_HIDDEN);
    } else
        lv_obj_add_flag(_empty, LV_OBJ_FLAG_HIDDEN);
    bool retry = _nameFailed || (window && window->statusRefreshDelayed());
    if (window && bound())
        for (size_t i = 0; i < window->count(); ++i)
            retry |= bool(window->row(i)->flags & Row::Unavailable);
    const bool update = window && _active && window->state() != Window::State::Exhausted &&
                        (retry || !window->freshnessAvailable());
    lv_label_set_text_static(lv_obj_get_child(_update, 0), retry ? "Retry" : "Check updates");
    if (update)
        lv_obj_clear_flag(_update, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(_update, LV_OBJ_FLAG_HIDDEN);
    for (size_t i = 0; i < 4; ++i) {
        const bool enabled = window && _active && !window->pageLoading() && (i < 2 ? window->canPrevious() : window->canNext());
        const bool changed = bool(lv_obj_has_state(_navigation[i], LV_STATE_DISABLED)) == enabled;
        if (enabled)
            lv_obj_clear_state(_navigation[i], LV_STATE_DISABLED);
        else
            lv_obj_add_state(_navigation[i], LV_STATE_DISABLED);
        // The arrow is custom-drawn from state, not an LVGL style property.
        if (changed)
            lv_obj_invalidate(_navigation[i]);
    }
}
void LvMessagesScreen::reportViewport() {
    if (_binding || !bound())
        return;
    auto      &window = _service->conversationWindow();
    const auto offset = std::max<int>(0, lv_obj_get_scroll_y(_list));
    window.setScrollOffset(offset);
}
void LvMessagesScreen::navigate(Navigation navigation) {
    if (!_active || !_service || _lpState != LP_NONE)
        return;
    auto &window = _service->conversationWindow();
    if (bound())
        reportViewport();
    _loadStarted = millis();
    _loadPending = true;
    switch (navigation) {
        case Navigation::Previous:
            window.previous();
            break;
        case Navigation::Next:
            window.nextPage();
            break;
        case Navigation::First:
            if (window.canPrevious())
                window.first();
            break;
        case Navigation::Last:
            window.last();
            break;
    }
    refreshUI();
}
void LvMessagesScreen::applyUpdate() {
    if (!_active || !_service || _lpState != LP_NONE || lv_obj_has_flag(_update, LV_OBJ_FLAG_HIDDEN))
        return;
    auto &window = _service->conversationWindow();
    if (bound())
        reportViewport();
    _loadStarted = millis();
    _loadPending = true;
    window.refresh();
    _namesResolved = 0;
    _nameFailed = false;
    refreshUI();
}
void LvMessagesScreen::openRow(size_t index) {
    if (!bound() || _lpState != LP_NONE || index >= _rowCount || !_onOpen)
        return;
    auto &window = _service->conversationWindow();
    window.select(index);
    reportViewport();
    char peer[33];
    peerText(window.row(index)->peer, peer);
    _onOpen(std::string(peer));
}
int LvMessagesScreen::focusedIndex() const {
    auto *focus = lv_group_get_focused(LvInput::group());
    for (size_t i = 0; i < _rowCount; ++i)
        if (_rows[i].row == focus)
            return static_cast<int>(i);
    return -1;
}
void LvMessagesScreen::pollName() {
    if (_nameRequest || !bound())
        return;
    auto &window = _service->conversationWindow();
    for (size_t i = 0; i < _rowCount; ++i) {
        const uint64_t bit = uint64_t { 1 } << i;
        if (_namesResolved & bit)
            continue;
        const auto *row = window.row(i);
        char        peer[33];
        peerText(row->peer, peer);
        const uint32_t revision = _boundRevision, identity = _boundIdentity, nodes = _nodeRevision;
        const uint32_t advertisedNodes = _service->status().nodeRevision;
        // The callback's fixed peer value belongs to the single admitted query.
        std::array<uint8_t, 16> expected;
        std::memcpy(expected.data(), row->peer, 16);
        _nameRequest = _service->requestPeerName(peer, [this, i, revision, identity, nodes, advertisedNodes, expected](const handheld::Result &result, const char *name) {
            _nameRequest = 0;
            if (!bound() || _boundRevision != revision || _boundIdentity != identity ||
                _service->status().nodeRevision != advertisedNodes || (_am ? _am->revision() : 0) != nodes || i >= _rowCount ||
                std::memcmp(_service->conversationWindow().row(i)->peer, expected.data(), 16))
                return;
            _namesResolved |= uint64_t { 1 } << i;
            if (result.outcome == handheld::Outcome::Ok && name && *name)
                lv_label_set_text(_rows[i].name, name);
            if (result.outcome != handheld::Outcome::Ok)
                _nameFailed = true;
        });
        // Admission pressure waits for explicit Retry; never spin per frame.
        if (!_nameRequest) {
            _namesResolved |= bit;
            _nameFailed = true;
        }
        return;
    }
}
void LvMessagesScreen::updateNames() {
    _nodeRevision = _am ? _am->revision() : 0;
    _namesResolved = 0;
    _nameFailed = false;
    if (!_am)
        return;
    const auto &window = _service->conversationWindow();
    for (size_t i = 0; i < _rowCount; ++i) {
        char peer[33];
        peerText(window.row(i)->peer, peer);
        const auto *node = _am->findNodeByHex(peer);
        if (node && !node->name.empty()) {
            lv_label_set_text(_rows[i].name, node->name.c_str());
            _namesResolved |= uint64_t { 1 } << i;
        }
    }
    updateStatuses(); // Contact status also contributes to the existing rail.
}
void LvMessagesScreen::showActionMenu(size_t index) {
    if (!bound() || index >= _rowCount || _actionRequest)
        return;
    auto &window = _service->conversationWindow();
    window.select(index);
    std::memcpy(_actionPeer, window.row(index)->peer, 16);
    _actionIdentity = _boundIdentity;
    _focusActive = true;
    _lpState = LP_MENU;
    _menuIdx = 0;
    rebuildActionOverlay();
}
void LvMessagesScreen::hideActionMenu() {
    if (_actionOverlay)
        lv_obj_del(_actionOverlay);
    _actionOverlay = nullptr;
    for (auto &row : _actionRows)
        row = nullptr;
    _lpState = LP_NONE;
    _menuIdx = 0;
    _actionIdentity = 0;
    std::memset(_actionPeer, 0, 16);
}
void LvMessagesScreen::rebuildActionOverlay() {
    if (_actionOverlay)
        lv_obj_del(_actionOverlay);
    _actionOverlay = lv_obj_create(lv_layer_top());
    lv_obj_add_style(_actionOverlay, Style::actionMenu, 0);
    lv_obj_add_style(_actionOverlay, Style::modalColor, 0);
    lv_obj_add_style(_actionOverlay, Style::modal, 0);
    lv_obj_add_style(_actionOverlay, Style::compactModalPadding, 0);
    lv_obj_clear_flag(_actionOverlay, LV_OBJ_FLAG_SCROLLABLE);
    const bool confirm = _lpState == LP_CONFIRM_DELETE;

    auto *title = lv_label_create(_actionOverlay);
    lv_obj_add_style(title, Style::fontBody, 0);
    lv_obj_add_style(title, Style::textAccent, 0);
    lv_obj_add_style(title, Style::actionMenuTitle, 0);
    lv_label_set_long_mode(title, LV_LABEL_LONG_CLIP);

    lv_label_set_text_static(title, confirm ? "DELETE CHAT?" : "CHAT ACTION");
    const char *labels[] = { confirm ? "Delete Chat" : "Add Friend", confirm ? "Cancel" : "Delete Chat", "Cancel" };
    for (size_t i = 0; i < (confirm ? 2u : 3u); ++i) {
        auto *row = lv_btn_create(_actionOverlay);
        _actionRows[i] = row;
        lv_obj_add_style(row, Style::menuRow, 0);
        lv_obj_add_style(row, Style::listItemColor, 0);
        lv_obj_add_style(row, Style::listItem, 0);
        {
            for (auto *tone : Style::bgTones)
                lv_obj_remove_style(row, tone, 0);
            lv_obj_add_style(row, Style::bgTones[i == _menuIdx ? Style::BG_HOVER : Style::BG], 0);
        }
        lv_obj_add_style(row, Style::noPadding, 0);
        lv_obj_set_user_data(row, reinterpret_cast<void *>(i));

        auto *text = lv_label_create(row);
        lv_obj_add_style(text, Style::fontBody, 0);
        lv_obj_add_style(text, Style::textPrimary, 0);
        lv_obj_add_style(text, Style::buttonCaption, 0);
        lv_label_set_long_mode(text, LV_LABEL_LONG_CLIP);

        lv_label_set_text_static(text, labels[i]);
        lv_obj_add_event_cb(row, [](lv_event_t *event) {
            auto* self=static_cast<LvMessagesScreen*>(lv_event_get_user_data(event));
            self->_menuIdx=reinterpret_cast<uintptr_t>(lv_obj_get_user_data(lv_event_get_target(event)));
            KeyEvent enter{};enter.enter=true;self->handleKey(enter); }, LV_EVENT_CLICKED, this);
    }
}
void LvMessagesScreen::performAction(handheld::Operation operation) {
    if (!_service || _actionRequest || _actionIdentity != _service->status().generation || !_actionIdentity) {
        hideActionMenu();
        return;
    }
    char peer[33];
    peerText(_actionPeer, peer);
    const uint32_t identity = _actionIdentity;
    hideActionMenu();
    _actionRequest = _service->action(operation, peer, "", 0, [this, identity](const handheld::Result &result) {
        _actionRequest = 0;
        if (!_service || identity != _service->status().generation)
            return;
        if (result.outcome == handheld::Outcome::Ok) {
            _service->conversationWindow().refresh();
            if (_active && _screen)
                notice("Saved");
        } else if (_active && _screen)
            notice("Action failed; retry");
    });
    if (!_actionRequest)
        notice("Busy; try again");
}
bool LvMessagesScreen::handleLongPress() {
    if (!bound())
        return false;
#if HAS_TOUCH
    if (!_focusActive)
        return false;
#endif
    const int index = focusedIndex();
    if (index < 0)
        return false;
    showActionMenu(index);
    return true;
}
bool LvMessagesScreen::handleKey(const KeyEvent &event) {
    if (!_service || !_active)
        return false;
    if (!_rrc.navigation.direct())
        return handleRrcKey(event);
    if (_lpState != LP_NONE) {
        if (_actionIdentity != _service->status().generation) {
            hideActionMenu();
            return true;
        }
        const bool     confirm = _lpState == LP_CONFIRM_DELETE;
        const unsigned count = confirm ? 2 : 3;
        if (event.up || event.down) {
            _menuIdx = (_menuIdx + count + (event.down ? 1 : -1)) % count;
            rebuildActionOverlay();
            return true;
        }
        if (event.enter || event.character == '\n' || event.character == '\r') {
            if (confirm) {
                if (_menuIdx == 0)
                    performAction(handheld::Operation::DeleteConversation);
                else
                    hideActionMenu();
            } else if (_menuIdx == 0)
                performAction(handheld::Operation::SaveContact);
            else if (_menuIdx == 1) {
                _lpState = LP_CONFIRM_DELETE;
                _menuIdx = 0;
                rebuildActionOverlay();
            } else
                hideActionMenu();
            return true;
        }
        if (!event.repeat && (event.del || event.character == 8 || event.character == 0x1b))
            hideActionMenu();
        return true;
    }
#if HAS_TOUCH
    if (!_focusActive && (event.up || event.down || event.enter)) {
        _focusActive = true;
        auto *focused = lv_group_get_focused(LvInput::group());
        if (focused)
            lv_obj_add_state(focused, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
        return true;
    }
#endif
    return false;
}

#include "LvRrcBrowser.inc"
