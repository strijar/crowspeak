#include "LvTabBar.h"
#include "Theme.h"
#include "fonts/fonts.h"
#include <cstdio>

constexpr const char* LvTabBar::TAB_NAMES[TAB_COUNT];

void LvTabBar::create(lv_obj_t* parent) {
    _view = lv_tabview_create(parent, LV_DIR_BOTTOM, Theme::TAB_BAR_H);
    lv_obj_set_pos(_view, 0, Theme::STATUS_BAR_H);
    lv_obj_set_size(_view, Theme::SCREEN_W, Theme::SCREEN_H - Theme::STATUS_BAR_H);
    auto* content = lv_tabview_get_content(_view);
    // Navigation is by buttons only. Lists retain their own vertical scrolling.
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_set_style_pad_gap(content, 0, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    for (int i = 0; i < TAB_COUNT; ++i) {
        _pages[i] = lv_tabview_add_tab(_view, TAB_NAMES[i]);
        lv_obj_clear_flag(_pages[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(_pages[i], 0, 0);
        lv_obj_set_style_border_width(_pages[i], 0, 0);
        lv_obj_set_style_radius(_pages[i], 0, 0);
    }
    lv_obj_add_event_cb(obj(), [](lv_event_t* event) {
        auto* self = static_cast<LvTabBar*>(lv_event_get_user_data(event));
        self->setActiveTab(lv_btnmatrix_get_selected_btn(lv_event_get_target(event)));
    }, LV_EVENT_VALUE_CHANGED, this);
    refreshTabs();
}

void LvTabBar::setActiveTab(int tab, bool notify) {
    if (tab < 0 || tab >= TAB_COUNT) return;
    _activeTab = tab;
    lv_tabview_set_act(_view, tab, LV_ANIM_OFF);
    if (notify && _tabCb) _tabCb(tab);
}

void LvTabBar::cycleTab(int direction) {
    setActiveTab((_activeTab + direction + TAB_COUNT) % TAB_COUNT);
}

void LvTabBar::setUnreadCount(int tab, int count) {
    if (tab < 0 || tab >= TAB_COUNT) return;
    if (count < 0) count = 0;
    if (_unread[tab] == count) return;
    _unread[tab] = count;
    refreshTab(tab);
}

void LvTabBar::refreshTab(int idx) {
    char caption[32];
    if (_unread[idx] > 99) snprintf(caption, sizeof(caption), "%s (99+)", TAB_NAMES[idx]);
    else if (_unread[idx]) snprintf(caption, sizeof(caption), "%s (%d)", TAB_NAMES[idx], _unread[idx]);
    else snprintf(caption, sizeof(caption), "%s", TAB_NAMES[idx]);
    lv_tabview_rename_tab(_view, idx, caption);
}

void LvTabBar::refreshTabs() {
    lv_obj_set_style_bg_color(_view, lv_color_hex(Theme::BG), 0);
    lv_obj_set_style_bg_opa(_view, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(_view, 0, 0);
    lv_obj_set_style_border_width(_view, 0, 0);
    auto* buttons = obj();
    lv_obj_set_style_bg_color(buttons, lv_color_hex(Theme::TAB_BG), 0);
    lv_obj_set_style_bg_opa(buttons, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(buttons, 0, 0);
    lv_obj_set_style_pad_gap(buttons, 0, 0);
    lv_obj_set_style_border_width(buttons, 0, 0);
    lv_obj_set_style_radius(buttons, 0, 0);
    lv_obj_set_style_text_font(buttons, &lv_font_crowpanel_14, LV_PART_ITEMS);
    lv_obj_set_style_text_color(buttons, lv_color_hex(Theme::TAB_INACTIVE), LV_PART_ITEMS);
    lv_obj_set_style_bg_color(buttons, lv_color_hex(Theme::TAB_BG), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(buttons, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_border_width(buttons, 0, LV_PART_ITEMS);
    lv_obj_set_style_radius(buttons, 0, LV_PART_ITEMS);
    lv_obj_set_style_text_color(buttons, lv_color_hex(Theme::TAB_ACTIVE), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(buttons, lv_color_hex(Theme::TAB_ACTIVE_BG), LV_PART_ITEMS | LV_STATE_CHECKED);
    for (int i = 0; i < TAB_COUNT; ++i) {
        lv_obj_set_style_bg_color(_pages[i], lv_color_hex(Theme::BG), 0);
        refreshTab(i);
    }
}
