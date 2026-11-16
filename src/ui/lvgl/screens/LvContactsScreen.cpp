#include "LvContactsScreen.h"
#include "Style.h"
#include "LvInput.h"
#include "LxmFaceAvatar.h"
#include "UIManager.h"
#include <Arduino.h>
#include <algorithm>
#include <climits>

namespace {

unsigned long nodeAgeMs(const DiscoveredNode &node, unsigned long now) {
    if (node.lastSeen == 0 || now < node.lastSeen)
        return ULONG_MAX;
    return now - node.lastSeen;
}

std::string displayNameFor(const DiscoveredNode &node) {
    if (!node.name.empty())
        return node.name;
    std::string hex = node.hash.toHex();
    return hex.substr(0, 12);
}

std::string identityLineFor(const DiscoveredNode &node) {
    std::string hex = node.hash.toHex();
    return "ID: " + hex;
}

std::string compactAge(unsigned long ageMs) {
    if (ageMs == ULONG_MAX)
        return "old";
    if (ageMs < 5000)
        return "now";

    unsigned long sec = ageMs / 1000;
    char          buf[12];
    if (sec < 60) {
        snprintf(buf, sizeof(buf), "%lus", sec);
    } else if (sec < 3600) {
        snprintf(buf, sizeof(buf), "%lum", sec / 60);
    } else if (sec < 86400) {
        snprintf(buf, sizeof(buf), "%luh", sec / 3600);
    } else {
        snprintf(buf, sizeof(buf), "%lud", sec / 86400);
    }
    return buf;
}

std::string contactMetaFor(unsigned long ageMs) {
    return compactAge(ageMs);
}

} // namespace

void LvContactsScreen::createUI(lv_obj_t *parent) {
    _screen = parent;
    {
        lv_obj_remove_style(parent, Style::listColor, 0);
        lv_obj_add_style(parent, Style::listColor, 0);
    }
    {
        lv_obj_remove_style(parent, Style::noPadding, 0);
        lv_obj_add_style(parent, Style::noPadding, 0);
    }

    _emptyState = lv_obj_create(parent);
    lv_obj_add_style(_emptyState, Style::emptyContact, 0);

    lv_obj_clear_flag(_emptyState, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *head = lv_obj_create(_emptyState);
    lv_obj_add_style(head, Style::contactHead, 0);

    lv_obj_add_style(head, Style::textareaFocusedColor, 0);

    lv_obj_clear_flag(head, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *shoulders = lv_obj_create(_emptyState);
    lv_obj_add_style(shoulders, Style::contactShoulders, 0);

    lv_obj_add_style(shoulders, Style::textareaFocusedColor, 0);

    lv_obj_clear_flag(shoulders, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *emptyStateCaption = lv_label_create(_emptyState);
    lv_obj_add_style(emptyStateCaption, Style::fontTitle, 0);
    lv_obj_add_style(emptyStateCaption, Style::textSecondary, 0);
    lv_label_set_text(emptyStateCaption, "No contacts");
    lv_obj_add_style(emptyStateCaption, Style::emptyCaption, 0);

    _list = lv_obj_create(parent);
    lv_obj_add_style(_list, Style::fillParent, 0);
    lv_obj_add_style(_list, Style::listColor, 0);
    lv_obj_add_style(_list, Style::list, 0);
    lv_obj_add_style(_list, Style::column, 0);

    lv_obj_add_flag(_list, LV_OBJ_FLAG_HIDDEN);

    _lastContactCount = -1;
    rebuildList();
}

void LvContactsScreen::onEnter() {
    _lastContactCount = -1;
#if HAS_TOUCH
    _focusActive = false;
#endif
    rebuildList();
}

void LvContactsScreen::refreshUI() {
    if (!_am)
        return;
    unsigned long now = millis();
    int           contacts = 0;
    for (const auto &n : _am->nodes()) {
        if (n.saved)
            contacts++;
    }
    if (contacts != _lastContactCount || now - _lastRebuild >= REBUILD_INTERVAL_MS) {
        rebuildList();
    }
}

void LvContactsScreen::rebuildList() {
    if (!_am || !_list)
        return;
    _lastRebuild = millis();
    const auto  scroll = lv_obj_get_scroll_y(_list);
    std::string focusedHex;
    auto       *focused = lv_group_get_focused(LvInput::group());
    if (focused && lv_obj_get_parent(focused) == _list) {
        int index = (int) (intptr_t) lv_obj_get_user_data(focused);
        if (index >= 0 && index < (int) _contactHexes.size())
            focusedHex = _contactHexes[index];
    }
    lv_obj_t *restoredFocus = nullptr;
    _contactHexes.clear();

    lv_obj_clean(_list);
    _avatarBuffers.clear();

    const auto      &nodes = _am->nodes();
    std::vector<int> contactIndices;
    for (int i = 0; i < (int) nodes.size(); i++) {
        if (nodes[i].saved)
            contactIndices.push_back(i);
    }
    std::sort(contactIndices.begin(), contactIndices.end(), [&nodes](int a, int b) {
        std::string an = displayNameFor(nodes[a]);
        std::string bn = displayNameFor(nodes[b]);
        if (an == bn)
            return nodes[a].hash.toHex() < nodes[b].hash.toHex();
        return an < bn;
    });
    int count = (int) contactIndices.size();
    _lastContactCount = count;
    _avatarBuffers.reserve(count);

    // Keep list visible even with no contacts so the QR header row is reachable.
    lv_obj_clear_flag(_list, LV_OBJ_FLAG_HIDDEN);
    if (_emptyState) {
        if (count == 0) {
            lv_obj_clear_flag(_emptyState, LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(_emptyState);
        } else {
            lv_obj_add_flag(_emptyState, LV_OBJ_FLAG_HIDDEN);
        }
    }

    {
        lv_obj_t *qrRow = lv_obj_create(_list);
        lv_obj_add_style(qrRow, Style::actionListRow, 0);
        lv_obj_add_style(qrRow, Style::listItemColor, 0);
        lv_obj_add_style(qrRow, Style::listItem, 0);
        lv_obj_add_style(qrRow, Style::listItemFocusedColor, LV_STATE_FOCUSED);
        lv_obj_add_style(qrRow, Style::listItemFocused, LV_STATE_FOCUSED);
        lv_obj_add_style(qrRow, Style::bgSelection, 0);
        lv_obj_add_style(qrRow, Style::actionRowBorder, 0);

        lv_obj_add_style(qrRow, Style::borderBorder, 0);

        lv_obj_clear_flag(qrRow, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(qrRow, LV_OBJ_FLAG_CLICKABLE);
        // Sentinel marks this as not-a-contact for handleLongPress().
        lv_obj_set_user_data(qrRow, (void *) (intptr_t) -1);

        lv_obj_add_event_cb(qrRow, [](lv_event_t *e) {
            auto* self = (LvContactsScreen*)lv_event_get_user_data(e);
            if (self->_showQrCb) self->_showQrCb(); }, LV_EVENT_CLICKED, this);

        lv_group_add_obj(LvInput::group(), qrRow);
        lv_obj_add_event_cb(qrRow, [](lv_event_t *e) { lv_obj_scroll_to_view(lv_event_get_target(e), LV_ANIM_ON); }, LV_EVENT_FOCUSED, nullptr);

        lv_obj_t *qrLbl = lv_label_create(qrRow);
        lv_obj_add_style(qrLbl, Style::fontTitle, 0);
        lv_obj_add_style(qrLbl, Style::textAccent, 0);
        lv_label_set_text(qrLbl, "Share My QR");
        lv_obj_add_style(qrLbl, Style::leadingCaption, 0);

        lv_obj_t *hintLbl = lv_label_create(qrRow);
        lv_obj_add_style(hintLbl, Style::fontCaption, 0);
        lv_obj_add_style(hintLbl, Style::textMuted, 0);
        lv_label_set_text(hintLbl, "Enter");
        lv_obj_add_style(hintLbl, Style::trailingHint, 0);
    }

    unsigned long now = millis();

    for (int i = 0; i < count; i++) {
        int         nodeIdx = contactIndices[i];
        const auto &node = nodes[nodeIdx];
        _contactHexes.push_back(node.hash.toHex());
        unsigned long age = nodeAgeMs(node, now);

        lv_obj_t *row = lv_obj_create(_list);
        lv_obj_add_style(row, Style::contactRow, 0);
        lv_obj_add_style(row, Style::listItemColor, 0);
        lv_obj_add_style(row, Style::listItem, 0);
        lv_obj_add_style(row, Style::listItemFocusedColor, LV_STATE_FOCUSED);
        lv_obj_add_style(row, Style::listItemFocused, LV_STATE_FOCUSED);
        lv_obj_add_style(row, Style::listRowBorder, 0);

        lv_obj_add_style(row, Style::borderBorder, 0);

        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_user_data(row, (void *) (intptr_t) i);

        lv_obj_add_event_cb(row, [](lv_event_t *e) {
            auto* self = (LvContactsScreen*)lv_event_get_user_data(e);
            int idx = (int)(intptr_t)lv_obj_get_user_data(lv_event_get_target(e));
            if (idx >= 0 && idx < (int)self->_contactHexes.size() && self->_onSelect) {
                self->_onSelect(self->_contactHexes[idx]);
            } }, LV_EVENT_CLICKED, this);

        lv_group_add_obj(LvInput::group(), row);
        lv_obj_add_event_cb(row, [](lv_event_t *e) { lv_obj_scroll_to_view(lv_event_get_target(e), LV_ANIM_ON); }, LV_EVENT_FOCUSED, nullptr);

        std::string hashHex = node.hash.toHex();
        if (hashHex == focusedHex)
            restoredFocus = row;
        lv_style_value_t avatarWidth;
        lv_style_get_prop(Style::contactAvatar, LV_STYLE_WIDTH, &avatarWidth);
        const lv_coord_t kContactAvatar = avatarWidth.num;
        _avatarBuffers.emplace_back(LxmFaceAvatar::bufferSize(kContactAvatar));
        LxmFaceAvatar::View avatar;
        avatar.box = lv_obj_create(row);
        lv_obj_add_style(avatar.box, Style::contactAvatar, 0);
        lv_obj_add_style(avatar.box, Style::avatarFrame, 0);
        lv_obj_add_style(avatar.box, Style::bgTones[Style::PRIMARY_SUBTLE], 0);
        lv_obj_add_style(avatar.box, Style::borderTones[Style::BORDER], 0);
        lv_obj_clear_flag(avatar.box, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        avatar.canvas = lv_canvas_create(avatar.box);
        lv_canvas_set_buffer(avatar.canvas, _avatarBuffers.back().data(), kContactAvatar, kContactAvatar, LV_IMG_CF_TRUE_COLOR_ALPHA);
        lv_obj_add_style(avatar.canvas, Style::avatarCanvas, 0);
        lv_img_set_antialias(avatar.canvas, false);
        lv_obj_clear_flag(avatar.canvas, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

        LxmFaceAvatar::render(avatar.canvas, String(hashHex.c_str()));

        lv_obj_t *nameLbl = lv_label_create(row);
        lv_obj_add_style(nameLbl, Style::fontTitle, 0);
        lv_obj_add_style(nameLbl, Style::textAccent, 0);
        lv_label_set_long_mode(nameLbl, LV_LABEL_LONG_CLIP);
        lv_obj_add_style(nameLbl, Style::contactName, 0);
        std::string name = displayNameFor(node);
        lv_label_set_text(nameLbl, name.c_str());

        lv_obj_t *metaLbl = lv_label_create(row);
        lv_obj_add_style(metaLbl, Style::fontCaption, 0);
        lv_obj_add_style(metaLbl, Style::textSecondary, 0);
        lv_obj_add_style(metaLbl, Style::contactAge, 0);
        lv_label_set_long_mode(metaLbl, LV_LABEL_LONG_CLIP);

        std::string meta = contactMetaFor(age);
        lv_label_set_text(metaLbl, meta.c_str());

        lv_obj_t *idLbl = lv_label_create(row);
        lv_obj_add_style(idLbl, Style::fontCaption, 0);
        lv_obj_add_style(idLbl, Style::textMuted, 0);
        lv_label_set_long_mode(idLbl, LV_LABEL_LONG_CLIP);
        lv_obj_add_style(idLbl, Style::contactAddress, 0);
        std::string identity = identityLineFor(node);
        lv_label_set_text(idLbl, identity.c_str());
    }

    if (restoredFocus)
        LvInput::focusObj(restoredFocus);
    lv_obj_update_layout(_list);
    lv_obj_scroll_to_y(_list, scroll, LV_ANIM_OFF);

#if HAS_TOUCH
    // Trackball/touch boards defer visible focus until the user navigates.
    if (!_focusActive) {
        lv_obj_t *focused = lv_group_get_focused(LvInput::group());
        if (focused)
            lv_obj_clear_state(focused, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
    }
#endif
}

bool LvContactsScreen::handleLongPress() {
    if (!_am || _contactHexes.empty())
        return false;
#if HAS_TOUCH
    // On trackball/touch boards, an unfocused hold belongs to the power shell.
    if (!_focusActive)
        return false;
#endif
    lv_obj_t *focused = lv_group_get_focused(LvInput::group());
    if (!focused || lv_obj_get_parent(focused) != _list)
        return false;
    int idx = (int) (intptr_t) lv_obj_get_user_data(focused);
    if (idx < 0 || idx >= (int) _contactHexes.size())
        return false;
    _deleteHex = _contactHexes[idx];
    _confirmDelete = true;
    if (_ui)
        _ui->lvStatusBar().showToast("Remove? Enter=Remove Backspace=Keep", 5000);
    return true;
}

bool LvContactsScreen::handleKey(const KeyEvent &event) {
    if (!_am || _contactHexes.empty())
        return false;

#if HAS_TOUCH
    if (!_focusActive && (event.up || event.down || event.enter)) {
        _focusActive = true;
        lv_obj_t *focused = lv_group_get_focused(LvInput::group());
        if (focused)
            lv_obj_add_state(focused, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
        return true;
    }
#endif

    if (_confirmDelete) {
        if (event.enter || event.character == '\n' || event.character == '\r') {
            if (!_deleteHex.empty()) {
                bool removed = _am->deleteContactByHex(_deleteHex);
                if (_ui)
                    _ui->lvStatusBar().showToast(
                        removed ? "Removing contact..." : "Contact action unavailable",
                        1200
                    );
                _deleteHex.clear();
                rebuildList();
            }
            _confirmDelete = false;
            return true;
        }
        _confirmDelete = false;
        if (_ui)
            _ui->lvStatusBar().showToast("Kept contact", 800);
        return true;
    }

    return false;
}
