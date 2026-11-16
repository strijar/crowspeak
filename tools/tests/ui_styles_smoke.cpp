#include "Style.h"
#include "TouchKeyboard.h"
#include "LvTabBar.h"
#include <cassert>
#include <cstdio>
#include <cstring>

static void flush(lv_disp_drv_t *display, const lv_area_t *, lv_color_t *) {
    lv_disp_flush_ready(display);
}

static void within(lv_obj_t *child, lv_obj_t *parent) {
    lv_area_t a, b;
    lv_obj_get_coords(child, &a);
    lv_obj_get_coords(parent, &b);
    assert(a.x1 >= b.x1 && a.y1 >= b.y1);
    if (a.x2 > b.x2 || a.y2 > b.y2)
        std::fprintf(stderr, "overflow child=[%d,%d,%d,%d] parent=[%d,%d,%d,%d]\n", a.x1, a.y1, a.x2, a.y2, b.x1, b.y1, b.x2, b.y2);
    assert(a.x2 <= b.x2 && a.y2 <= b.y2);
    assert(lv_obj_get_width(child) > 0 && lv_obj_get_height(child) > 0);
}

void testPeerLifecycle();

int main() {
    lv_init();
    // Static flex tables must match the runtime registration order.
    assert(LV_LAYOUT_FLEX == 1);
    assert(LV_STYLE_FLEX_FLOW == _LV_STYLE_LAST_BUILT_IN_PROP + 1);
    assert(LV_STYLE_FLEX_MAIN_PLACE == _LV_STYLE_LAST_BUILT_IN_PROP + 2);
    assert(LV_STYLE_FLEX_CROSS_PLACE == _LV_STYLE_LAST_BUILT_IN_PROP + 3);
    assert(LV_STYLE_FLEX_TRACK_PLACE == _LV_STYLE_LAST_BUILT_IN_PROP + 4);
    assert(LV_STYLE_FLEX_GROW == _LV_STYLE_LAST_BUILT_IN_PROP + 5);
    static lv_color_t         pixels[800 * 20];
    static lv_disp_draw_buf_t buffer;
    lv_disp_draw_buf_init(&buffer, pixels, nullptr, 800 * 20);
    lv_disp_drv_t driver;
    lv_disp_drv_init(&driver);
    driver.hor_res = 800;
    driver.ver_res = 480;
    driver.draw_buf = &buffer;
    driver.flush_cb = flush;
    lv_disp_drv_register(&driver);
    lv_obj_add_style(lv_scr_act(), Style::theme, 0);

    auto *button = lv_btn_create(lv_scr_act());
    lv_obj_add_style(button, Style::buttonColor, 0);
    lv_obj_add_style(button, Style::footerAccept, 0);
    lv_obj_add_style(button, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_update_layout(lv_scr_act());
    within(button, lv_scr_act());
    const auto normal = lv_obj_get_style_bg_color(button, 0).full;
    lv_obj_add_state(button, LV_STATE_PRESSED);
    assert(normal != lv_obj_get_style_bg_color(button, 0).full);
    lv_obj_clear_state(button, LV_STATE_PRESSED);
    assert(normal == lv_obj_get_style_bg_color(button, 0).full);
    lv_obj_del(button);

    for (unsigned language = 0; language < 2; ++language) {
        TouchKeyboard::configure(language, [](uint8_t) { return true; });
        TouchKeyboard::show("Edit", "Hello", 200, [](const std::string &) { return true; });
        auto *overlay = lv_obj_get_child(lv_layer_top(), 0);
        lv_obj_update_layout(overlay);
        assert(lv_obj_get_width(overlay) == 800 && lv_obj_get_height(overlay) == 480);
        assert(lv_obj_get_width(lv_obj_get_child(overlay, 1)) == lv_obj_get_content_width(overlay));
        // title, editor, hint, keys, Cancel, Done
        assert(lv_obj_get_child_cnt(overlay) == 6);
        for (unsigned i = 0; i < 6; ++i)
            within(lv_obj_get_child(overlay, i), overlay);
        auto *keys = lv_obj_get_child(overlay, 3);
        assert(lv_obj_get_child_cnt(keys) == 4);
        for (unsigned r = 0; r < 4; ++r) {
            auto *row = lv_obj_get_child(keys, r);
            within(row, keys);
            assert(lv_obj_get_child_cnt(row) > 0);
            lv_coord_t previous = -1;
            for (unsigned k = 0; k < lv_obj_get_child_cnt(row); ++k) {
                auto *key = lv_obj_get_child(row, k);
                within(key, row);
                assert(lv_obj_get_x(key) > previous);
                previous = lv_obj_get_x(key);
            }
        }
        auto *commands = lv_obj_get_child(keys, 3);
        // Shift and symbol layouts rebuild the actual keyboard through its callbacks.
        lv_event_send(lv_obj_get_child(commands, 0), LV_EVENT_CLICKED, nullptr);
        lv_obj_update_layout(overlay);
        commands = lv_obj_get_child(keys, 3);
        lv_event_send(lv_obj_get_child(commands, 1), LV_EVENT_CLICKED, nullptr);
        lv_obj_update_layout(overlay);
        assert(lv_obj_get_child_cnt(keys) == 4);
        TouchKeyboard::close();
    }
    LvTabBar tabs;
    tabs.create(lv_scr_act());
    lv_obj_update_layout(tabs.view());
    within(tabs.view(), lv_scr_act());
    const auto tabStyles = tabs.obj()->style_cnt;
    const auto viewStyles = tabs.view()->style_cnt;
    for (unsigned i = 0; i < 100; ++i) {
        tabs.refreshTabs();
        tabs.setUnreadCount(1, i);
        assert(tabs.obj()->style_cnt == tabStyles);
        assert(tabs.view()->style_cnt == viewStyles);
    }
    auto *row = lv_obj_create(tabs.page(1));
    lv_obj_add_style(row, Style::messageRow, 0);
    auto *bubble = lv_obj_create(row);
    lv_obj_add_style(bubble, Style::messageBubble, 0);
    lv_obj_add_style(bubble, Style::bubbleContent, 0);
    lv_obj_add_style(bubble, Style::outgoingPosition, 0);
    auto *text = lv_label_create(bubble);
    lv_obj_add_style(text, Style::fullWidth, 0);
    lv_label_set_text(text, "Hello");
    auto *metadata = lv_obj_create(bubble);
    lv_obj_add_style(metadata, Style::messageMetadata, 0);
    auto *status = lv_label_create(metadata);
    lv_obj_add_style(status, Style::fontCaption, 0);
    lv_label_set_text(status, "Sent");
    lv_obj_update_layout(row);
    const auto originalHeight = lv_obj_get_height(row);
    within(bubble, row);
    lv_label_set_text(status, "Failed\nRetry delivery");
    lv_obj_update_layout(row);
    assert(lv_obj_get_height(row) > originalHeight);
    within(bubble, row);
    lv_obj_del(tabs.view());

    TouchKeyboard::choose("Choose", { "One", "Two", "Three" }, 1, [](size_t) { return true; });
    auto *overlay = lv_obj_get_child(lv_layer_top(), 0);
    lv_obj_update_layout(overlay);
    auto *choices = lv_obj_get_child(overlay, 1);
    assert(lv_obj_get_child_cnt(choices) == 3);
    for (unsigned i = 0; i < 3; ++i)
        within(lv_obj_get_child(choices, i), choices);
    TouchKeyboard::close();
    testPeerLifecycle();
    puts("UI styles: static flex IDs, cascading states, EN/RU keyboard, choices, retained tabs and message reflow and Peers lifecycle OK");
}
