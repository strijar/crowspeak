#include "UIManager.h"
#include "TouchKeyboard.h"
#include "voice/VoiceInput.h"
#include "Theme.h"
#include "LvTheme.h"
#include "LvInput.h"
#include "LvMemory.h"
#include <esp_heap_caps.h>

// Widget roots belong to the manager. Screens only retire their own references.
void LvScreen::destroyUI() { _screen = nullptr; }

namespace {
void prepareRoot(lv_obj_t* root) {
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(root, 0, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root, lv_color_hex(Theme::BG), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_radius(root, 0, 0);
}
}

void UIManager::begin() {
    auto* scr = lv_scr_act();
    LvTheme::init(lv_disp_get_default());
    lv_obj_add_style(scr, LvTheme::styleScreen(), 0);
    _lvStatusBar.create(scr);
    _lvTabBar.create(scr);
    // Boot/setup screens have an independent, disposable full-screen host.
    _lvContent = lv_obj_create(scr);
    prepareRoot(_lvContent);
    setBootMode(false);
    Serial.println("[UI] Native Tabview created; screens retained on first visit");
}

UIManager::RetainedScreen* UIManager::retained(LvScreen* screen) {
    if (!screen) return nullptr;
    for (auto& slot : _retained) if (slot.screen == screen) return &slot;
    return nullptr;
}

void UIManager::registerTab(int tab, LvScreen* screen) {
    if (tab < 0 || tab >= LvTabBar::TAB_COUNT) return;
    _retained[tab].screen = screen; _retained[tab].tab = tab;
    _tabSelected[tab] = screen;
}

void UIManager::registerTabChild(int tab, LvScreen* screen) {
    if (tab < 0 || tab >= LvTabBar::TAB_COUNT) return;
    auto& slot = _retained[LvTabBar::TAB_COUNT];
    slot.screen = screen; slot.tab = tab;
}

void UIManager::selectTab(int tab) {
    if (tab < 0 || tab >= LvTabBar::TAB_COUNT || !_tabSelected[tab]) return;
    setScreen(_tabSelected[tab]);
}

void UIManager::release(RetainedScreen& slot) {
    if (!slot.root) return;
    LvInput::setGroup(slot.group);
    slot.screen->onExit();
    slot.screen->destroyUI();
    lv_obj_del(slot.root); slot.root = nullptr;
    // Keep the small empty group: default/input drivers must never dangle.
}

void UIManager::activate(RetainedScreen& slot) {
    if (!slot.group) slot.group = LvInput::createGroup();
    LvInput::setGroup(slot.group);
    if (slot.stale) { release(slot); slot.stale = false; }
    const bool created = !slot.root;
    if (created) {
        slot.root = lv_obj_create(_lvTabBar.page(slot.tab));
        prepareRoot(slot.root);
        slot.screen->createUI(slot.root);
        slot.screen->onEnter();
    } else {
        slot.screen->onShow();
    }
    lv_obj_clear_flag(slot.root, LV_OBJ_FLAG_HIDDEN);
    _tabSelected[slot.tab] = slot.screen;
    _lvTabBar.setActiveTab(slot.tab, false);
}

void UIManager::logMemory(const char* action, const char* title, unsigned long elapsed) {
    const auto usage = handheld_lvgl_memory_stats();
    unsigned screens = 0;
    for (const auto& slot : _retained) if (slot.root) ++screens;
    Serial.printf("[UI] %s %s: %lums, screens=%u LVGL=%u peak=%u refused=%u PSRAM-free=%u internal-free=%u\n",
        action, title, elapsed, screens, unsigned(usage.retained), unsigned(usage.peak),
        unsigned(usage.refused), unsigned(heap_caps_get_free_size(MALLOC_CAP_SPIRAM)),
        unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)));
}

void UIManager::setScreen(LvScreen* screen) {
    if (_currentLvScreen == screen) return;
    const auto started = millis();
    TouchKeyboard::close();
    handheld::voice::VoiceInput::instance().cancel();
    if (_voicePanel && _voicePanel->visible()) _voicePanel->hide();
    if (!_transientGroup) _transientGroup = LvInput::group();
    auto* next = retained(screen);
    if (auto* previous = retained(_currentLvScreen)) {
        previous->screen->onHide();
        lv_obj_add_flag(previous->root, LV_OBJ_FLAG_HIDDEN);
        // Back from a conversation closes it; switching tabs only suspends it.
        if (next && next->tab == previous->tab && previous == &_retained[LvTabBar::TAB_COUNT])
            release(*previous);
    } else if (_currentLvScreen) {
        _currentLvScreen->onExit();
        _currentLvScreen->destroyUI();
        lv_obj_clean(_lvContent);
    }
    _currentLvScreen = screen;
    if (next) {
        lv_obj_add_flag(_lvContent, LV_OBJ_FLAG_HIDDEN);
        activate(*next);
    } else if (screen) {
        LvInput::setGroup(_transientGroup);
        lv_obj_clear_flag(_lvContent, LV_OBJ_FLAG_HIDDEN);
        screen->createUI(_lvContent);
        screen->onEnter();
    }
    logMemory("show", screen ? screen->title() : "none", millis() - started);
}

void UIManager::setBootMode(bool boot) {
    _bootMode = boot;
    if (boot) {
        lv_obj_add_flag(_lvStatusBar.obj(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_lvTabBar.view(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_lvContent, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(_lvContent, 0, 0);
        lv_obj_set_size(_lvContent, Theme::SCREEN_W, Theme::SCREEN_H);
    } else {
        lv_obj_clear_flag(_lvStatusBar.obj(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(_lvTabBar.view(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(_lvContent, 0, Theme::STATUS_BAR_H);
        lv_obj_set_size(_lvContent, Theme::CONTENT_W, Theme::CONTENT_H);
        if (!_currentLvScreen || retained(_currentLvScreen)) lv_obj_add_flag(_lvContent, LV_OBJ_FLAG_HIDDEN);
    }
}

void UIManager::update() {
    _lvStatusBar.update();
    // Theme changes invalidate inline colors. Rebuild lazily, outside callbacks.
    if (auto* current = retained(_currentLvScreen)) {
        if (current->stale) {
            TouchKeyboard::close();
            handheld::voice::VoiceInput::instance().cancel();
            current->screen->onHide();
            activate(*current);
        }
    }
    if (_currentLvScreen) _currentLvScreen->refreshUI();
    if (_currentLvScreen) TouchKeyboard::attach(_currentLvScreen->screen());
    TouchKeyboard::attach(lv_layer_top());
}

void UIManager::forceRedraw() {
    lv_obj_invalidate(lv_scr_act());
}

void UIManager::applyTheme() {
    LvTheme::refresh();
    for (auto& slot : _retained) if (slot.root) slot.stale = true;
    if (_lvContent) lv_obj_set_style_bg_color(_lvContent, lv_color_hex(Theme::BG), 0);
    _lvStatusBar.applyTheme();
    _lvTabBar.refreshTabs();
    LvInput::applyTheme();
    forceRedraw();
}

bool UIManager::handleKey(const KeyEvent& event) {
    if(_voicePanel && _voicePanel->handleKey(event)) return true;
    if (_currentLvScreen) {
        return _currentLvScreen->handleKey(event);
    }
    return false;
}

bool UIManager::handleLongPress() {
    if (_currentLvScreen) {
        return _currentLvScreen->handleLongPress();
    }
    return false;
}
