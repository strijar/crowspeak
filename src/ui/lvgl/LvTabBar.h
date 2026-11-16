#pragma once

#include <lvgl.h>

class LvTabBar {
  public:
    enum Tab {
        TAB_HOME = 0,
        TAB_MSGS = 1,
        TAB_CONTACTS = 2,
        TAB_NODES = 3,
        TAB_SETTINGS = 4,
        TAB_CHATS = TAB_MSGS,
        TAB_PEERS = TAB_NODES,
        TAB_SETUP = TAB_SETTINGS,
        TAB_COUNT = 5
    };

    void create(lv_obj_t *parent);

    void setActiveTab(int tab, bool notify = true);
    int  getActiveTab() const {
        return _activeTab;
    }
    void cycleTab(int direction);

    void setUnreadCount(int tab, int count);

    using TabCallback = void (*)(int tab);
    void setTabCallback(TabCallback cb) {
        _tabCb = cb;
    }

    lv_obj_t *obj() {
        return lv_tabview_get_tab_btns(_view);
    }
    lv_obj_t *view() {
        return _view;
    }
    lv_obj_t *page(int tab) {
        return _pages[tab];
    }

    // Refresh shared tab styles and unread captions without accumulating styles.
    void refreshTabs();

  private:
    void refreshTab(int idx);

    lv_obj_t   *_view = nullptr;
    lv_obj_t   *_pages[TAB_COUNT] = {};
    int         _activeTab = TAB_HOME;
    int         _unread[TAB_COUNT] = {};
    TabCallback _tabCb = nullptr;

    static constexpr const char *TAB_NAMES[TAB_COUNT] = { "Home", "Chats", "Contacts", "Peers", "Settings" };
};
