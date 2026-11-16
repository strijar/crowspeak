// Compile the real Peers class declaration and lifecycle methods. Only application
// services and action callbacks are replaced: this test needs no radio or Arduino.
#include "Style.h"
#include "config/BoardConfig.h"
#include "LvTabBar.h"
#include "ui/PeerList.h"
#include <array>
#include <cassert>
#include <functional>
#include <string>
#include <vector>

using String = std::string;
struct KeyEvent {
    bool enter = false;
};
namespace handheld {
class ServiceClient;
class NodeView;
}
class LvScreen {
  public:
    virtual ~LvScreen() = default;
    virtual void createUI(lv_obj_t *) = 0;
    virtual void destroyUI() {
        _screen = nullptr;
    }
    virtual void refreshUI() {
    }
    virtual void onEnter() {
    }
    virtual void onHide() {
    }
    virtual bool handleKey(const KeyEvent &) {
        return false;
    }
    virtual bool handleLongPress() {
        return false;
    }
    virtual const char *title() const = 0;

  protected:
    lv_obj_t *_screen = nullptr;
};

// Generated from the production files by check_ui_styles.py, without their
// embedded-platform includes. No copy of the pool allocation logic lives here.
#include "peer_screen_declaration.h"
#include "peer_screen_lifecycle.inc"

void LvNodesScreen::rebuildList() {
    assert(!_rows.empty());
    for (const auto &row : _rows) {
        assert(row.box && lv_obj_is_valid(row.box));
        assert(row.name && lv_obj_is_valid(row.name));
        assert(row.meta && lv_obj_is_valid(row.meta));
        assert(row.id && lv_obj_is_valid(row.id));
        assert(lv_obj_get_parent(row.box) == _list);
    }
}
void LvNodesScreen::bindRows() {
    rebuildList();
}
void LvNodesScreen::focusSelection() {
}
void LvNodesScreen::showActionMenu(const std::string &) {
}
void LvNodesScreen::showNicknameInput() {
}
void LvNodesScreen::hideOverlay() {
    lv_obj_add_flag(_overlay, LV_OBJ_FLAG_HIDDEN);
}
void LvNodesScreen::refreshUI() {
    rebuildList();
}
bool LvNodesScreen::handleKey(const KeyEvent &) {
    return true;
}
bool LvNodesScreen::handleLongPress() {
    return false;
}

void testPeerLifecycle() {
    LvTabBar tabs;
    tabs.create(lv_scr_act());
    LvNodesScreen screen;
    for (unsigned lifetime = 0; lifetime < 3; ++lifetime) {
        auto *root = lv_obj_create(tabs.page(LvTabBar::TAB_PEERS));
        lv_obj_add_style(root, Style::screenRoot, 0);
        screen.createUI(root);
        auto *list = lv_obj_get_child(root, 1);
        lv_obj_update_layout(list);
        lv_style_value_t rowHeight;
        assert(lv_style_get_prop(Style::peerRow, LV_STYLE_HEIGHT, &rowHeight) == LV_STYLE_RES_FOUND);
        const unsigned expectedRows = (lv_obj_get_height(list) + rowHeight.num - 1) / rowHeight.num + 4;
        // One end marker plus the fully constructed row pool, even with no peers.
        assert(lv_obj_get_child_cnt(list) == expectedRows + 1);
        screen.onEnter();
        tabs.setActiveTab(LvTabBar::TAB_PEERS);
        for (unsigned visit = 0; visit < 20; ++visit) {
            screen.refreshUI();
            screen.onHide();
            tabs.setActiveTab(LvTabBar::TAB_HOME);
            tabs.setActiveTab(LvTabBar::TAB_PEERS);
            screen.onEnter();
            assert(lv_obj_get_child_cnt(list) == expectedRows + 1);
        }
        screen.destroyUI();
        lv_obj_del(root);
    }
    lv_obj_del(tabs.view());
}
