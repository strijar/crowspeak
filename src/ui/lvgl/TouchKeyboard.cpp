#include "Style.h"
#include "TouchKeyboard.h"
#include "KeyboardLayouts.h"
#include "input/Utf8.h"
#include "LvPrivacy.h"
#include <cstring>
#include <utility>
namespace TouchKeyboard {
namespace {
lv_obj_t                    *overlay = nullptr, *editor = nullptr, *keys = nullptr, *hint = nullptr;
Accept                       accepted;
std::function<bool(size_t)>  choiceAccepted;
size_t                       limit = 0, language = 0;
uint32_t                     generation = 0;
bool                         shift = false, symbols = false;
std::function<bool(uint8_t)> persistLanguage;
constexpr size_t             languageCount = sizeof(layouts) / sizeof(layouts[0]);
void                         layout();
// The firmware disables LVGL's default theme. Top-layer windows must supply
// their own text color, and backgrounds remain transparent without an opacity.

void key(lv_event_t *e) {
    const char *text = lv_label_get_text(lv_obj_get_child(lv_event_get_target(e), 0));
    if (!strcmp(text, "Shift")) {
        shift = !shift;
        layout();
        return;
    }
    if (!strcmp(text, "Lang")) {
        const size_t next = (language + 1) % languageCount;
        if (persistLanguage && persistLanguage(uint8_t(next))) {
            language = next;
            symbols = false;
            layout();
        } else
            lv_label_set_text(hint, "Settings busy. Try again.");
        return;
    }
    if (!strcmp(text, "123") || !strcmp(text, "ABC")) {
        symbols = !symbols;
        layout();
        return;
    }
    if (!strcmp(text, "Backspace")) {
        lv_textarea_del_char(editor);
        return;
    }
    if (!strcmp(text, "Left")) {
        lv_textarea_cursor_left(editor);
        return;
    }
    if (!strcmp(text, "Right")) {
        lv_textarea_cursor_right(editor);
        return;
    }
    if (!strcmp(text, "Space"))
        text = " ";
    if (strlen(lv_textarea_get_text(editor)) + strlen(text) > limit) {
        lv_label_set_text(hint, "Text limit reached");
        return;
    }
    lv_textarea_add_text(editor, text);
}
void layout() {
    lv_obj_clean(keys);
    const char *numberRows[] = { "1234567890", "@#%&*()-_+=", R"keys(.,:;!?/\"')keys" };
    const auto &l = layouts[language];
    for (int row = 0; row < 3; ++row) {
        const char *chars = symbols ? numberRows[row] : (shift ? l.upper[row] : l.lower[row]);
        size_t      count = 0;
        for (const unsigned char *s = reinterpret_cast<const unsigned char *>(chars); *s; ++s)
            if ((*s & 0xc0) != 0x80)
                ++count;
        if (!count)
            continue;
        auto *keyRow = lv_obj_create(keys);
        lv_obj_add_style(keyRow, Style::keyboardRow, 0);
        lv_obj_clear_flag(keyRow, LV_OBJ_FLAG_SCROLLABLE);
        while (*chars) {
            char   symbol[5] {};
            size_t n = 1;
            while (chars[n] && (static_cast<unsigned char>(chars[n]) & 0xc0) == 0x80 && n < 4)
                ++n;
            memcpy(symbol, chars, n);
            chars += n;
            auto *keyButton = lv_btn_create(keyRow);
            lv_obj_add_style(keyButton, Style::buttonColor, 0);
            lv_obj_add_style(keyButton, Style::keyboardKey, 0);
            lv_obj_add_style(keyButton, Style::buttonPressedColor, LV_STATE_PRESSED);
            lv_obj_add_event_cb(keyButton, key, LV_EVENT_CLICKED, nullptr);
            auto *caption = lv_label_create(keyButton);
            lv_obj_add_style(caption, Style::centered, 0);
            lv_label_set_text(caption, symbol);
        }
    }
    auto *commandRow = lv_obj_create(keys);
    lv_obj_add_style(commandRow, Style::keyboardRow, 0);
    lv_obj_clear_flag(commandRow, LV_OBJ_FLAG_SCROLLABLE);
    const char *commands[] = { "Shift", symbols ? "ABC" : "123", "Lang", "Space", "Left", "Right", "Backspace" };
    for (const char *command : commands) {
        auto *keyButton = lv_btn_create(commandRow);
        lv_obj_add_style(keyButton, Style::buttonColor, 0);
        lv_obj_add_style(keyButton, Style::keyboardKey, 0);
        if (!strcmp(command, "Space"))
            lv_obj_add_style(keyButton, Style::spaceKey, 0);
        else if (!strcmp(command, "Backspace"))
            lv_obj_add_style(keyButton, Style::wideKey, 0);
        lv_obj_add_style(keyButton, Style::buttonPressedColor, LV_STATE_PRESSED);
        lv_obj_add_event_cb(keyButton, key, LV_EVENT_CLICKED, nullptr);
        auto *caption = lv_label_create(keyButton);
        lv_obj_add_style(caption, Style::centered, 0);
        lv_label_set_text(caption, command);
    }
    lv_label_set_text_fmt(hint, "%s | %s", l.name, shift ? "Uppercase" : "Lowercase");
}
void textareaClick(lv_event_t *e) {
    auto *target = lv_event_get_target(e);
    if (overlay || lv_obj_has_state(target, LV_STATE_DISABLED))
        return;
    const bool     password = lv_textarea_get_password_mode(target);
    const uint32_t maxLength = lv_textarea_get_max_length(target);
    show("Edit text", lv_textarea_get_text(target), maxLength ? maxLength : 400, [target](const std::string &text) {
   if(!lv_obj_is_valid(target)) return false;
   lv_textarea_set_text(target,text.c_str());
   lv_event_send(target,LV_EVENT_READY,nullptr);
   return true; }, password);
}
}
void configure(uint8_t initial, std::function<bool(uint8_t)> save) {
    language = initial < languageCount ? initial : 0;
    persistLanguage = std::move(save);
}
bool visible() {
    return overlay;
}
void close() {
    ++generation;
    accepted = {};
    choiceAccepted = {};
    if (overlay) {
        auto *old = overlay;
        overlay = editor = keys = hint = nullptr;
        lv_obj_del(old);
    }
}
void show(const char *title, const char *initial, size_t maxBytes, Accept accept, bool password) {
    close();
    accepted = std::move(accept);
    limit = maxBytes;
    shift = symbols = false;

    overlay = lv_obj_create(lv_layer_top());
    lv_obj_add_style(overlay, Style::keyboardOverlay, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_style(overlay, Style::listColor, 0);

    lv_obj_add_style(overlay, Style::textPrimary, 0);
    lv_obj_add_style(overlay, Style::fontTitle, 0);

    auto *label = lv_label_create(overlay);
    lv_label_set_text(label, title);
    lv_obj_add_style(label, Style::titleInset, 0);
    editor = lv_textarea_create(overlay);
    lv_obj_add_style(editor, Style::keyboardEditor, 0);
    lv_obj_add_style(editor, Style::fontTitle, 0);
    lv_obj_add_style(editor, Style::bgElevated, 0);

    lv_obj_add_style(editor, Style::borderBorder, 0);

    lv_obj_add_style(editor, Style::borderAccent, LV_PART_CURSOR);
    lv_obj_add_style(editor, Style::textCursor, LV_PART_CURSOR);

    lv_textarea_set_max_length(editor, maxBytes);
    lv_textarea_set_password_mode(editor, password);
    if (password)
        LvPrivacy::markSensitive(editor);
    lv_textarea_set_text(editor, handheld::utf8::prefix(initial ? initial : "", limit).c_str());
    lv_textarea_set_cursor_pos(editor, LV_TEXTAREA_CURSOR_LAST);
    hint = lv_label_create(overlay);
    lv_obj_add_style(hint, Style::keyboardHint, 0);
    keys = lv_obj_create(overlay);
    lv_obj_add_style(keys, Style::keyboardRows, 0);

    lv_obj_clear_flag(keys, LV_OBJ_FLAG_SCROLLABLE);
    auto *cancel = lv_btn_create(overlay);
    lv_obj_add_style(cancel, Style::buttonColor, 0);
    lv_obj_add_style(cancel, Style::footerCancel, 0);
    lv_obj_add_style(cancel, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_event_cb(cancel, [](lv_event_t *) { close(); }, LV_EVENT_CLICKED, nullptr);
    auto *cancelText = lv_label_create(cancel);
    lv_obj_add_style(cancelText, Style::centered, 0);
    lv_label_set_text(cancelText, "Cancel");
    auto *done = lv_btn_create(overlay);
    lv_obj_add_style(done, Style::buttonColor, 0);
    lv_obj_add_style(done, Style::footerAccept, 0);
    lv_obj_add_style(done, Style::buttonPressedColor, LV_STATE_PRESSED);
    auto *doneText = lv_label_create(done);
    lv_obj_add_style(doneText, Style::centered, 0);
    lv_label_set_text(doneText, "Done");
    lv_obj_add_event_cb(done, [](lv_event_t *) {
  std::string text=lv_textarea_get_text(editor);
  auto callback=accepted;const auto current=generation;
  const bool ok=callback && callback(text);
  if(generation==current && ok) close();
  else if(generation==current) lv_label_set_text(hint,"Unable to apply value. Check input and retry."); }, LV_EVENT_CLICKED, nullptr);
    layout();
}
namespace {
void choiceClick(lv_event_t *event) {
    const auto   current = generation;
    auto         callback = choiceAccepted;
    const size_t index = reinterpret_cast<size_t>(lv_event_get_user_data(event));
    if (callback && callback(index) && generation == current)
        close();
}
}
void choose(const char *title, const std::vector<std::string> &values, size_t selected, std::function<bool(size_t)> accept) {
    close();
    choiceAccepted = std::move(accept);

    overlay = lv_obj_create(lv_layer_top());
    lv_obj_add_style(overlay, Style::keyboardOverlay, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_style(overlay, Style::listColor, 0);

    lv_obj_add_style(overlay, Style::textPrimary, 0);
    lv_obj_add_style(overlay, Style::fontTitle, 0);

    auto *label = lv_label_create(overlay);
    lv_label_set_text(label, title);
    auto *list = lv_obj_create(overlay);
    lv_obj_add_style(list, Style::choiceList, 0);

    for (size_t i = 0; i < values.size(); ++i) {
        auto *b = lv_btn_create(list);
        lv_obj_add_style(b, Style::choiceListItem, 0);
        {
            for (auto *tone : Style::bgTones)
                lv_obj_remove_style(b, tone, 0);
            lv_obj_add_style(b, Style::bgTones[i == selected ? Style::PRIMARY_MUTED : Style::BG_ELEVATED], 0);
        }

        lv_obj_add_style(b, Style::bgPressed, LV_STATE_PRESSED);
        lv_obj_add_event_cb(b, choiceClick, LV_EVENT_CLICKED, reinterpret_cast<void *>(i));
        auto *text = lv_label_create(b);
        lv_label_set_text(text, values[i].c_str());
        lv_obj_add_style(text, Style::centered, 0);
    }
    auto *cancel = lv_btn_create(overlay);
    lv_obj_add_style(cancel, Style::buttonColor, 0);
    lv_obj_add_style(cancel, Style::footerCancel, 0);
    lv_obj_add_style(cancel, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_event_cb(cancel, [](lv_event_t *) { close(); }, LV_EVENT_CLICKED, nullptr);
    auto *cancelText = lv_label_create(cancel);
    lv_obj_add_style(cancelText, Style::centered, 0);
    lv_label_set_text(cancelText, "Cancel");
}
void attach(lv_obj_t *root) {
    if (!root || root == overlay)
        return;
    if (lv_obj_check_type(root, &lv_textarea_class)) {
        if (!lv_obj_get_event_user_data(root, textareaClick))
            lv_obj_add_event_cb(root, textareaClick, LV_EVENT_CLICKED, root);
    }
    const uint32_t n = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < n; ++i)
        attach(lv_obj_get_child(root, i));
}
}
