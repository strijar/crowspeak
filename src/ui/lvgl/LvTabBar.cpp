#include "Style.h"
#include "LvTabBar.h"
#include <cstdio>

constexpr const char *LvTabBar::TAB_NAMES[TAB_COUNT];

void LvTabBar::create(lv_obj_t *parent) {
    lv_style_value_t tabHeight;
    lv_style_get_prop(Style::tabBarSize, LV_STYLE_HEIGHT, &tabHeight);
    _view = lv_tabview_create(parent, LV_DIR_BOTTOM, tabHeight.num);
    // The native constructor installs local 100% dimensions, which outrank
    // shared styles. Let the profile own the viewport below the status bar.
    lv_obj_remove_local_style_prop(_view, LV_STYLE_WIDTH, 0);
    lv_obj_remove_local_style_prop(_view, LV_STYLE_HEIGHT, 0);
    lv_obj_add_style(_view, Style::tabViewport, 0);

    auto *content = lv_tabview_get_content(_view);
    // Navigation is by buttons only. Lists retain their own vertical scrolling.
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    {
        lv_obj_remove_style(content, Style::seamlessContent, 0);
        lv_obj_add_style(content, Style::seamlessContent, 0);
    }

    for (int i = 0; i < TAB_COUNT; ++i) {
        _pages[i] = lv_tabview_add_tab(_view, TAB_NAMES[i]);
        lv_obj_clear_flag(_pages[i], LV_OBJ_FLAG_SCROLLABLE);
        {
            lv_obj_remove_style(_pages[i], Style::flatContent, 0);
            lv_obj_add_style(_pages[i], Style::flatContent, 0);
        }
    }
    lv_obj_add_event_cb(obj(), [](lv_event_t *event) {
        auto* self = static_cast<LvTabBar*>(lv_event_get_user_data(event));
        self->setActiveTab(lv_btnmatrix_get_selected_btn(lv_event_get_target(event))); }, LV_EVENT_VALUE_CHANGED, this);
    refreshTabs();
}

void LvTabBar::setActiveTab(int tab, bool notify) {
    if (tab < 0 || tab >= TAB_COUNT)
        return;
    _activeTab = tab;
    lv_tabview_set_act(_view, tab, LV_ANIM_OFF);
    if (notify && _tabCb)
        _tabCb(tab);
}

void LvTabBar::cycleTab(int direction) {
    setActiveTab((_activeTab + direction + TAB_COUNT) % TAB_COUNT);
}

void LvTabBar::setUnreadCount(int tab, int count) {
    if (tab < 0 || tab >= TAB_COUNT)
        return;
    if (count < 0)
        count = 0;
    if (_unread[tab] == count)
        return;
    _unread[tab] = count;
    refreshTab(tab);
}

void LvTabBar::refreshTab(int idx) {
    char caption[32];
    if (_unread[idx] > 99)
        snprintf(caption, sizeof(caption), "%s (99+)", TAB_NAMES[idx]);
    else if (_unread[idx])
        snprintf(caption, sizeof(caption), "%s (%d)", TAB_NAMES[idx], _unread[idx]);
    else
        snprintf(caption, sizeof(caption), "%s", TAB_NAMES[idx]);
    lv_tabview_rename_tab(_view, idx, caption);
}

void LvTabBar::refreshTabs() {
    {
        lv_obj_remove_style(_view, Style::listColor, 0);
        lv_obj_add_style(_view, Style::listColor, 0);
    }
    {
        lv_obj_remove_style(_view, Style::opaqueContent, 0);
        lv_obj_add_style(_view, Style::opaqueContent, 0);
    }

    auto *buttons = obj();
    {
        lv_obj_remove_style(buttons, Style::bgTab, 0);
        lv_obj_add_style(buttons, Style::bgTab, 0);
    }
    {
        lv_obj_remove_style(buttons, Style::tabStrip, 0);
        lv_obj_add_style(buttons, Style::tabStrip, 0);
    }

    {
        lv_obj_remove_style(buttons, Style::fontTab, LV_PART_ITEMS);
        lv_obj_add_style(buttons, Style::fontTab, LV_PART_ITEMS);
    }
    {
        lv_obj_remove_style(buttons, Style::textSecondary, LV_PART_ITEMS);
        lv_obj_add_style(buttons, Style::textSecondary, LV_PART_ITEMS);
    }
    {
        lv_obj_remove_style(buttons, Style::bgTab, LV_PART_ITEMS);
        lv_obj_add_style(buttons, Style::bgTab, LV_PART_ITEMS);
    }
    {
        lv_obj_remove_style(buttons, Style::tabItem, LV_PART_ITEMS);
        lv_obj_add_style(buttons, Style::tabItem, LV_PART_ITEMS);
    }

    {
        lv_obj_remove_style(buttons, Style::textActiveTabText, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_add_style(buttons, Style::textActiveTabText, LV_PART_ITEMS | LV_STATE_CHECKED);
    }
    {
        lv_obj_remove_style(buttons, Style::bgActiveTab, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_add_style(buttons, Style::bgActiveTab, LV_PART_ITEMS | LV_STATE_CHECKED);
    }
    for (int i = 0; i < TAB_COUNT; ++i) {
        {
            lv_obj_remove_style(_pages[i], Style::listColor, 0);
            lv_obj_add_style(_pages[i], Style::listColor, 0);
        }
        refreshTab(i);
    }
}
