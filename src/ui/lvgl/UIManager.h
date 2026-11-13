#pragma once

#include <lvgl.h>
#include "LvStatusBar.h"
#include "LvTabBar.h"
#include "LvVoicePanel.h"
#include "hal/Keyboard.h"

// LVGL screen base class
class LvScreen {
public:
    virtual ~LvScreen() = default;
    virtual void createUI(lv_obj_t* parent) = 0;
    virtual void destroyUI();
    virtual void refreshUI() {}
    virtual void onEnter() {}
    virtual void onExit() {}
    // Hiding a retained tab must not tear down its widget tree.
    virtual void onHide() {}
    virtual void onShow() { refreshUI(); }
    virtual bool handleKey(const KeyEvent& event) { return false; }
    virtual bool handleLongPress() { return false; }
    virtual const char* title() const = 0;

    lv_obj_t* screen() const { return _screen; }

protected:
    lv_obj_t* _screen = nullptr;
};

class UIManager {
public:
    void begin();
    void setVoicePanel(LvVoicePanel* panel) {_voicePanel=panel;}
    void openVoice(const char* peer) {if(_voicePanel)_voicePanel->start(peer);}
    void closeVoiceConversation() {if(_voicePanel)_voicePanel->closeConversation();}
    handheld::memo::Ui* memoUi() {return _voicePanel?&_voicePanel->model():nullptr;}
    void openVoiceMessage(const char* peer,uint32_t counter,bool incoming) {if(_voicePanel)_voicePanel->startMessage(peer,counter,incoming);}

    // Screen management
    void setScreen(LvScreen* screen);
    void registerTab(int tab, LvScreen* screen);
    void registerTabChild(int tab, LvScreen* screen);
    void selectTab(int tab);
    LvScreen* getScreen() { return _currentLvScreen; }

    // Component access
    LvStatusBar& lvStatusBar() { return _lvStatusBar; }
    LvTabBar& lvTabBar() { return _lvTabBar; }

    // Update data (called periodically)
    void update();

    // Force full redraw
    void forceRedraw();

    // Re-apply palette to shared styles and persistent shell after a theme switch
    void applyTheme();

    // Handle key event — routes to current screen
    bool handleKey(const KeyEvent& event);
    bool handleLongPress();

    // Boot mode — hides status bar and tab bar
    void setBootMode(bool boot);
    bool isBootMode() const { return _bootMode; }

    // LVGL content area parent (between status bar and tab bar)
    lv_obj_t* contentParent() { return _lvContent; }

private:
    // LVGL components
    LvVoicePanel* _voicePanel=nullptr;
    LvStatusBar _lvStatusBar;
    LvTabBar _lvTabBar;
    LvScreen* _currentLvScreen = nullptr;
    lv_obj_t* _lvContent = nullptr;

    struct RetainedScreen {
        LvScreen* screen = nullptr;
        lv_obj_t* root = nullptr;
        lv_group_t* group = nullptr;
        int tab = -1;
        bool stale = false;
    };
    RetainedScreen _retained[LvTabBar::TAB_COUNT + 1]{};
    LvScreen* _tabSelected[LvTabBar::TAB_COUNT]{};
    lv_group_t* _transientGroup = nullptr;
    RetainedScreen* retained(LvScreen* screen);
    void release(RetainedScreen& slot);
    void activate(RetainedScreen& slot);
    void logMemory(const char* action, const char* title, unsigned long elapsed);
    bool _bootMode = false;
};
