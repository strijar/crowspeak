#include "LvHomeScreen.h"
#include "Style.h"
#include "LvInput.h"
#include "LxmFaceAvatar.h"
#include "config/UserConfig.h"
#include "util/DisplayName.h"
#include <Arduino.h>

namespace {

void makeFocusable(lv_obj_t *obj) {
    if (!obj)
        return;
    {
        lv_obj_remove_style(obj, Style::buttonFocusedColor, LV_STATE_FOCUSED);
        lv_obj_add_style(obj, Style::buttonFocusedColor, LV_STATE_FOCUSED);
    }
    {
        lv_obj_remove_style(obj, Style::buttonFocused, LV_STATE_FOCUSED);
        lv_obj_add_style(obj, Style::buttonFocused, LV_STATE_FOCUSED);
    }
#if HAS_TOUCH
    // LVGL also assigns keypad focus during automatic group registration.
    // On touch boards, paint a ring only for actual key/trackball navigation.
    lv_obj_add_event_cb(obj, [](lv_event_t *event) {
        const auto code = lv_event_get_code(event);
        if (code != LV_EVENT_FOCUSED && code != LV_EVENT_PRESSED) return;
        auto* input = lv_indev_get_act();
        const bool touch = input && lv_indev_get_type(input) == LV_INDEV_TYPE_POINTER;
        if ((code == LV_EVENT_FOCUSED && !input) || touch) {
            auto* target = lv_event_get_target(event);
            auto* focused = lv_group_get_focused(static_cast<lv_group_t*>(lv_obj_get_group(target)));
            if (focused) lv_obj_clear_state(focused, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
            lv_obj_clear_state(target, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
        } }, LV_EVENT_ALL, nullptr);
#endif
    lv_group_add_obj(LvInput::group(), obj);
}

void makeClickable(lv_obj_t *obj, void (*cb)(lv_event_t *), void *userData) {
    if (!obj)
        return;
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    makeFocusable(obj);
    lv_obj_add_event_cb(obj, cb, LV_EVENT_CLICKED, userData);
}

bool tcpConfigured(const UserSettings &settings) {
    for (const auto &ep : settings.tcpConnections) {
        if (!ep.host.isEmpty() && ep.autoConnect)
            return true;
    }
    return false;
}

void formatAge(unsigned long elapsedMs, char *out, size_t len) {
    unsigned long sec = elapsedMs / 1000;
    if (sec < 60) {
        snprintf(out, len, "%lus ago", sec);
    } else if (sec < 3600) {
        snprintf(out, len, "%lum ago", sec / 60);
    } else {
        snprintf(out, len, "%luh ago", sec / 3600);
    }
}

} // namespace

void LvHomeScreen::createUI(lv_obj_t *parent) {
    _screen = parent;
    {
        lv_obj_remove_style(parent, Style::absoluteContent, 0);
        lv_obj_add_style(parent, Style::absoluteContent, 0);
    }
    {
        lv_obj_remove_style(parent, Style::listColor, 0);
        lv_obj_add_style(parent, Style::listColor, 0);
    }

    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);

    _identityPanel = lv_obj_create(parent);
    lv_obj_add_style(_identityPanel, Style::identityCard, 0);

    lv_obj_add_style(_identityPanel, Style::bgElevated, 0);

    lv_obj_add_style(_identityPanel, Style::borderBorder, 0);

    lv_obj_clear_flag(_identityPanel, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *rail = lv_obj_create(_identityPanel);
    lv_obj_add_style(rail, Style::identityAccent, 0);

    lv_obj_add_style(rail, Style::barIndicatorColor, 0);

    lv_obj_clear_flag(rail, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_style_value_t avatarWidth;
    lv_style_get_prop(Style::identityAvatar, LV_STYLE_WIDTH, &avatarWidth);
    const lv_coord_t kAvatarSize = avatarWidth.num;
    _avatarBuffer.assign(LxmFaceAvatar::bufferSize(kAvatarSize), 0);
    LxmFaceAvatar::View avatar;
    avatar.box = lv_obj_create(_identityPanel);
    lv_obj_add_style(avatar.box, Style::identityAvatar, 0);
    lv_obj_add_style(avatar.box, Style::avatarFrame, 0);
    lv_obj_add_style(avatar.box, Style::bgTones[Style::PRIMARY_SUBTLE], 0);
    lv_obj_add_style(avatar.box, Style::borderTones[Style::BORDER_ACTIVE], 0);
    lv_obj_clear_flag(avatar.box, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    avatar.canvas = lv_canvas_create(avatar.box);
    lv_canvas_set_buffer(avatar.canvas, _avatarBuffer.data(), kAvatarSize, kAvatarSize, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_obj_add_style(avatar.canvas, Style::avatarCanvas, 0);
    lv_img_set_antialias(avatar.canvas, false);
    lv_obj_clear_flag(avatar.canvas, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    _avatarBox = avatar.box;
    _avatarCanvas = avatar.canvas;
    _avatarSeed = "";

    _lblConsoleTitle = lv_label_create(_identityPanel);
    lv_obj_add_style(_lblConsoleTitle, Style::identityHeading, 0);

    lv_label_set_long_mode(_lblConsoleTitle, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblConsoleTitle, Style::fontCaption, 0);
    lv_obj_add_style(_lblConsoleTitle, Style::textAccent, 0);

    lv_label_set_text(_lblConsoleTitle, "IDENTITY:");

    _lblStatus = lv_label_create(_identityPanel);
    lv_obj_add_style(_lblStatus, Style::connectivityCaption, 0);

    lv_label_set_long_mode(_lblStatus, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblStatus, Style::fontCaption, 0);
    lv_obj_add_style(_lblStatus, Style::textSecondary, 0);

    lv_label_set_text(_lblStatus, "OFFLINE");

    _lblName = lv_label_create(_identityPanel);
    lv_obj_add_style(_lblName, Style::identityName, 0);

    lv_label_set_long_mode(_lblName, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblName, Style::fontTitle, 0);
    lv_obj_add_style(_lblName, Style::textPrimary, 0);

    lv_label_set_text(_lblName, "...");

    lv_label_set_long_mode(_lblName, LV_LABEL_LONG_DOT);

    _lblId = lv_label_create(_identityPanel);
    lv_obj_add_style(_lblId, Style::identityDestination, 0);

    lv_label_set_long_mode(_lblId, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblId, Style::fontCaption, 0);
    lv_obj_add_style(_lblId, Style::textSecondary, 0);

    lv_label_set_text(_lblId, "LXMF ----");

    _lblIdentity = lv_label_create(_identityPanel);
    lv_obj_add_style(_lblIdentity, Style::identityHash, 0);

    lv_label_set_long_mode(_lblIdentity, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblIdentity, Style::fontCaption, 0);
    lv_obj_add_style(_lblIdentity, Style::textMuted, 0);

    lv_label_set_text(_lblIdentity, "ID ----");

    _chipLora = lv_obj_create(parent);
    lv_obj_add_style(_chipLora, Style::summaryTile, 0);
    lv_obj_add_style(_chipLora, Style::transportTileLeft, 0);

    lv_obj_add_style(_chipLora, Style::bgElevated, 0);

    lv_obj_add_style(_chipLora, Style::borderBorder, 0);

    lv_obj_clear_flag(_chipLora, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *chipLoraCaption = lv_label_create(_chipLora);
    lv_obj_add_style(chipLoraCaption, Style::tileCaption, 0);

    lv_label_set_long_mode(chipLoraCaption, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(chipLoraCaption, Style::fontCaption, 0);
    lv_obj_add_style(chipLoraCaption, Style::textMuted, 0);

    lv_label_set_text(chipLoraCaption, "LORA");

    _lblLoraState = lv_label_create(_chipLora);
    lv_obj_add_style(_lblLoraState, Style::tileState, 0);

    lv_label_set_long_mode(_lblLoraState, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblLoraState, Style::fontBody, 0);
    lv_obj_add_style(_lblLoraState, Style::textSecondary, 0);

    lv_label_set_text(_lblLoraState, "...");

    makeClickable(_chipLora, [](lv_event_t *e) {
        auto* self = (LvHomeScreen*)lv_event_get_user_data(e);
        self->toggleLora(); }, this);

    _chipTcp = lv_obj_create(parent);
    lv_obj_add_style(_chipTcp, Style::summaryTile, 0);
    lv_obj_add_style(_chipTcp, Style::transportTileMiddle, 0);

    lv_obj_add_style(_chipTcp, Style::bgElevated, 0);

    lv_obj_add_style(_chipTcp, Style::borderBorder, 0);

    lv_obj_clear_flag(_chipTcp, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *chipTcpCaption = lv_label_create(_chipTcp);
    lv_obj_add_style(chipTcpCaption, Style::tileCaption, 0);

    lv_label_set_long_mode(chipTcpCaption, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(chipTcpCaption, Style::fontCaption, 0);
    lv_obj_add_style(chipTcpCaption, Style::textMuted, 0);

    lv_label_set_text(chipTcpCaption, "TCP");

    _lblTcpState = lv_label_create(_chipTcp);
    lv_obj_add_style(_lblTcpState, Style::tileState, 0);

    lv_label_set_long_mode(_lblTcpState, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblTcpState, Style::fontBody, 0);
    lv_obj_add_style(_lblTcpState, Style::textSecondary, 0);

    lv_label_set_text(_lblTcpState, "...");

    makeClickable(_chipTcp, [](lv_event_t *e) {
        auto* self = (LvHomeScreen*)lv_event_get_user_data(e);
        self->toggleTcp(); }, this);

    _chipWifi = lv_obj_create(parent);
    lv_obj_add_style(_chipWifi, Style::summaryTile, 0);
    lv_obj_add_style(_chipWifi, Style::transportTileRight, 0);

    lv_obj_add_style(_chipWifi, Style::bgElevated, 0);

    lv_obj_add_style(_chipWifi, Style::borderBorder, 0);

    lv_obj_clear_flag(_chipWifi, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *chipWifiCaption = lv_label_create(_chipWifi);
    lv_obj_add_style(chipWifiCaption, Style::tileCaption, 0);

    lv_label_set_long_mode(chipWifiCaption, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(chipWifiCaption, Style::fontCaption, 0);
    lv_obj_add_style(chipWifiCaption, Style::textMuted, 0);

    lv_label_set_text(chipWifiCaption, "WIFI");

    _lblWifiState = lv_label_create(_chipWifi);
    lv_obj_add_style(_lblWifiState, Style::tileState, 0);

    lv_label_set_long_mode(_lblWifiState, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblWifiState, Style::fontBody, 0);
    lv_obj_add_style(_lblWifiState, Style::textSecondary, 0);

    lv_label_set_text(_lblWifiState, "...");

    makeClickable(_chipWifi, [](lv_event_t *e) {
        auto* self = (LvHomeScreen*)lv_event_get_user_data(e);
        self->toggleWiFi(); }, this);

    _statNodes = lv_obj_create(parent);
    lv_obj_add_style(_statNodes, Style::summaryTile, 0);
    lv_obj_add_style(_statNodes, Style::statusTileLeft, 0);

    lv_obj_add_style(_statNodes, Style::bgElevated, 0);

    lv_obj_add_style(_statNodes, Style::borderBorder, 0);

    lv_obj_clear_flag(_statNodes, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *statNodesCaption = lv_label_create(_statNodes);
    lv_obj_add_style(statNodesCaption, Style::tileCaption, 0);

    lv_label_set_long_mode(statNodesCaption, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(statNodesCaption, Style::fontCaption, 0);
    lv_obj_add_style(statNodesCaption, Style::textMuted, 0);

    lv_label_set_text(statNodesCaption, "PEERS");

    _lblNodes = lv_label_create(_statNodes);
    lv_obj_add_style(_lblNodes, Style::tileValue, 0);

    lv_label_set_long_mode(_lblNodes, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblNodes, Style::fontTitle, 0);
    lv_obj_add_style(_lblNodes, Style::textPrimary, 0);

    lv_label_set_text(_lblNodes, "0");

    makeClickable(_statNodes, [](lv_event_t *e) {
        auto* self = (LvHomeScreen*)lv_event_get_user_data(e);
        self->openPeers(); }, this);

    _statPaths = lv_obj_create(parent);
    lv_obj_add_style(_statPaths, Style::summaryTile, 0);
    lv_obj_add_style(_statPaths, Style::statusTileMiddle, 0);

    lv_obj_add_style(_statPaths, Style::bgElevated, 0);

    lv_obj_add_style(_statPaths, Style::borderBorder, 0);

    lv_obj_clear_flag(_statPaths, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *statPathsCaption = lv_label_create(_statPaths);
    lv_obj_add_style(statPathsCaption, Style::tileCaption, 0);

    lv_label_set_long_mode(statPathsCaption, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(statPathsCaption, Style::fontCaption, 0);
    lv_obj_add_style(statPathsCaption, Style::textMuted, 0);

    lv_label_set_text(statPathsCaption, "AUDIO");

    _lblPaths = lv_label_create(_statPaths);
    lv_obj_add_style(_lblPaths, Style::tileValue, 0);

    lv_label_set_long_mode(_lblPaths, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblPaths, Style::fontTitle, 0);
    lv_obj_add_style(_lblPaths, Style::textPrimary, 0);

    lv_label_set_text(_lblPaths, "0");

    makeClickable(_statPaths, [](lv_event_t *e) {
        auto* self = (LvHomeScreen*)lv_event_get_user_data(e);
        self->toggleAudio(); }, this);

    _statLinks = lv_obj_create(parent);
    lv_obj_add_style(_statLinks, Style::summaryTile, 0);
    lv_obj_add_style(_statLinks, Style::statusTileRight, 0);

    lv_obj_add_style(_statLinks, Style::bgElevated, 0);

    lv_obj_add_style(_statLinks, Style::borderBorder, 0);

    lv_obj_clear_flag(_statLinks, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *statLinksCaption = lv_label_create(_statLinks);
    lv_obj_add_style(statLinksCaption, Style::summaryCaption, 0);

    lv_label_set_long_mode(statLinksCaption, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(statLinksCaption, Style::fontCaption, 0);
    lv_obj_add_style(statLinksCaption, Style::textMuted, 0);

    lv_label_set_text(statLinksCaption, "GPS");

    lv_obj_add_style(statLinksCaption, Style::announceAge, 0);

    _lblLinks = lv_label_create(_statLinks);
    lv_obj_add_style(_lblLinks, Style::tileValue, 0);

    lv_label_set_long_mode(_lblLinks, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblLinks, Style::fontTitle, 0);
    lv_obj_add_style(_lblLinks, Style::textPrimary, 0);

    lv_label_set_text(_lblLinks, "0");

    makeClickable(_statLinks, [](lv_event_t *e) {
        auto* self = (LvHomeScreen*)lv_event_get_user_data(e);
        self->toggleGPS(); }, this);

    // AUDIO/GPS ON/OFF values match the chip-row typography; the 14pt stat font
    // stays for numeric values (PEERS). Donors rendered these in the stat font.
    lv_obj_add_style(_lblPaths, Style::fontBody, 0);
    lv_obj_add_style(_lblPaths, Style::tileStatePosition, 0);
    lv_obj_add_style(_lblLinks, Style::fontBody, 0);
    lv_obj_add_style(_lblLinks, Style::tileStatePosition, 0);

    lv_obj_t *obj2 = lv_obj_create(parent);
    lv_obj_add_style(obj2, Style::summaryFooter, 0);

    lv_obj_add_style(obj2, Style::bgElevated, 0);

    lv_obj_add_style(obj2, Style::borderBorder, 0);

    lv_obj_clear_flag(obj2, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *footer = obj2;

    lv_obj_t *footerCaption2 = lv_label_create(footer);
    lv_obj_add_style(footerCaption2, Style::footerSummary, 0);

    lv_label_set_long_mode(footerCaption2, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(footerCaption2, Style::fontBody, 0);
    lv_obj_add_style(footerCaption2, Style::textSecondary, 0);

    lv_label_set_text(footerCaption2, "Waiting for transport");

    _lblSummary = footerCaption2;
    lv_label_set_long_mode(_lblSummary, LV_LABEL_LONG_DOT);

    lv_obj_t *footerCaption = lv_label_create(footer);
    lv_obj_add_style(footerCaption, Style::footerAge, 0);

    lv_label_set_long_mode(footerCaption, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(footerCaption, Style::fontCaption, 0);
    lv_obj_add_style(footerCaption, Style::textMuted, 0);

    lv_label_set_text(footerCaption, "Announced: never");

    _lblLastAnnounce = footerCaption;
    lv_label_set_long_mode(_lblLastAnnounce, LV_LABEL_LONG_CLIP);

    _btnAnnounce = lv_btn_create(parent);
    lv_obj_add_style(_btnAnnounce, Style::announceAction, 0);

    lv_obj_add_style(_btnAnnounce, Style::buttonColor, 0);
    lv_obj_add_style(_btnAnnounce, Style::button, 0);
    lv_obj_add_style(_btnAnnounce, Style::buttonPressedColor, LV_STATE_PRESSED);
    lv_obj_add_style(_btnAnnounce, Style::buttonPressed, LV_STATE_PRESSED);
    lv_obj_add_style(_btnAnnounce, Style::bgSelection, 0);
    lv_obj_add_style(_btnAnnounce, Style::textareaFocusedColor, 0);
    lv_obj_add_style(_btnAnnounce, Style::flatAction, 0);

    _lblAnnounceAction = lv_label_create(_btnAnnounce);
    lv_obj_add_style(_lblAnnounceAction, Style::fontBody, 0);
    lv_obj_add_style(_lblAnnounceAction, Style::textAccent, 0);
    lv_label_set_text(_lblAnnounceAction, "ANNOUNCE");
    lv_obj_add_style(_lblAnnounceAction, Style::centered, 0);

    makeFocusable(_btnAnnounce);
    lv_obj_add_event_cb(_btnAnnounce, [](lv_event_t *e) {
        auto* self = (LvHomeScreen*)lv_event_get_user_data(e);
        if (self->_announceCb) self->_announceCb(); }, LV_EVENT_CLICKED, this);

    _lastRefreshMs = 0;
    _lastUptime = ULONG_MAX;
    _lastHeap = UINT32_MAX;
    refreshUI();
}

void LvHomeScreen::onEnter() {
    _lastRefreshMs = 0;
    _lastUptime = ULONG_MAX;
    _lastHeap = UINT32_MAX;
    _avatarSeed = "";
#if !HAS_TOUCH
    // Logical focus on the action button; the ring only paints once the user
    // enters the screen (navbar-mode suppression lifts in main.cpp)
    if (_btnAnnounce)
        LvInput::focusObj(_btnAnnounce);
#endif
    refreshUI();
}

void LvHomeScreen::refreshUI() {
    if (!_lblName)
        return;

    unsigned long now = millis();
    bool          force = (_lastUptime == ULONG_MAX || _lastHeap == UINT32_MAX);
    if (!force && (now - _lastRefreshMs) < 1000)
        return;
    _lastRefreshMs = now;

    unsigned long upMins = now / 60000;
    uint32_t      heap = ESP.getFreeHeap() / 1024;
    _lastUptime = upMins;
    _lastHeap = heap;

    const String destination = _backend ? _backend->destinationHashHex() : String();
    char         fallback[handheld::DEVICE_NAME_FALLBACK_SIZE];
    const String displayName = handheld::deviceDisplayName(
        _cfg ? _cfg->settings().displayName.c_str() : nullptr,
        destination.c_str(),
        DEVICE_NAME,
        fallback
    );
    lv_label_set_text(_lblName, displayName.c_str());

    String avatarSeed = displayName;

    if (_backend) {
        String dh = _backend->destinationHashHex();
        if (!dh.isEmpty() && dh != "unknown")
            avatarSeed = dh;
        // Narrow panel: truncate; wide panel shows the full 32-hex LXMF addr

        lv_label_set_text_fmt(_lblId, "LXMF %s", dh.c_str());

        String ih = _backend->identityHash();
        if (ih.length() > 14)
            ih = ih.substring(0, 14);
        lv_label_set_text_fmt(_lblIdentity, "ID %s", ih.c_str());
    } else {
        lv_label_set_text(_lblId, "LXMF ----");
        lv_label_set_text(_lblIdentity, "ID ----");
    }
    renderAvatar(avatarSeed);

    bool loraRuntimeUp = _service && _service->status().lora;
    bool loraEnabled = _cfg ? _cfg->settings().loraEnabled : loraRuntimeUp;
    bool tcpUp = false;
    int  tcpClientCount = 0;
    if (_service)
        tcpUp = _service->status().tcpUp;

    bool wifiUp = _service && _service->status().wifi;
    bool wifiEnabled = _cfg ? _cfg->settings().wifiMode != RAT_WIFI_OFF : wifiUp;
    bool wifiRuntimeActive = wifiUp || (_cfg && _cfg->settings().wifiMode == RAT_WIFI_AP);
    bool tcpEnabled = _cfg ? tcpConfigured(_cfg->settings()) : tcpClientCount > 0;
    bool transportActive = _backend && _backend->isTransportActive();
    bool reachable = loraRuntimeUp || tcpUp;

    {
        uint32_t color = Style::TEXT_MUTED;
        uint32_t bg = Style::BG_ELEVATED;
        uint32_t border = Style::BORDER;

        if ((loraEnabled && loraRuntimeUp)) {
            color = Style::SUCCESS;
            bg = Style::PRIMARY_SUBTLE;
            border = Style::PRIMARY;
        } else if ((loraEnabled != loraRuntimeUp)) {
            color = Style::WARNING_CLR;
            border = Style::WARNING_CLR;
        }

        {
            if (_chipLora) {
                {
                    for (auto *tone : Style::bgTones)
                        lv_obj_remove_style(_chipLora, tone, 0);
                    lv_obj_add_style(_chipLora, Style::bgTones[bg], 0);
                }
                {
                    for (auto *tone : Style::borderTones)
                        lv_obj_remove_style(_chipLora, tone, 0);
                    lv_obj_add_style(_chipLora, Style::borderTones[border], 0);
                }
            }
        }
        if (_lblLoraState) {
            lv_label_set_text(_lblLoraState, (loraEnabled ? "ON" : "OFF"));
            {
                for (auto *tone : Style::textTones)
                    lv_obj_remove_style(_lblLoraState, tone, 0);
                lv_obj_add_style(_lblLoraState, Style::textTones[color], 0);
            }
        }
    }
    {
        uint32_t color = Style::TEXT_MUTED;
        uint32_t bg = Style::BG_ELEVATED;
        uint32_t border = Style::BORDER;

        if (tcpUp) {
            color = Style::SUCCESS;
            bg = Style::PRIMARY_SUBTLE;
            border = Style::PRIMARY;
        } else if ((tcpEnabled && !tcpUp)) {
            color = Style::WARNING_CLR;
            border = Style::WARNING_CLR;
        }

        {
            if (_chipTcp) {
                {
                    for (auto *tone : Style::bgTones)
                        lv_obj_remove_style(_chipTcp, tone, 0);
                    lv_obj_add_style(_chipTcp, Style::bgTones[bg], 0);
                }
                {
                    for (auto *tone : Style::borderTones)
                        lv_obj_remove_style(_chipTcp, tone, 0);
                    lv_obj_add_style(_chipTcp, Style::borderTones[border], 0);
                }
            }
        }
        if (_lblTcpState) {
            lv_label_set_text(_lblTcpState, (tcpEnabled ? "ON" : "OFF"));
            {
                for (auto *tone : Style::textTones)
                    lv_obj_remove_style(_lblTcpState, tone, 0);
                lv_obj_add_style(_lblTcpState, Style::textTones[color], 0);
            }
        }
    }
    // LORA-pattern state rule (donor used runtime-only active): config OFF never
    // renders green; any config/runtime mismatch shows the warning tone.
    {
        uint32_t color = Style::TEXT_MUTED;
        uint32_t bg = Style::BG_ELEVATED;
        uint32_t border = Style::BORDER;

        if ((wifiEnabled && wifiRuntimeActive)) {
            color = Style::SUCCESS;
            bg = Style::PRIMARY_SUBTLE;
            border = Style::PRIMARY;
        } else if ((wifiEnabled != wifiRuntimeActive)) {
            color = Style::WARNING_CLR;
            border = Style::WARNING_CLR;
        }

        {
            if (_chipWifi) {
                {
                    for (auto *tone : Style::bgTones)
                        lv_obj_remove_style(_chipWifi, tone, 0);
                    lv_obj_add_style(_chipWifi, Style::bgTones[bg], 0);
                }
                {
                    for (auto *tone : Style::borderTones)
                        lv_obj_remove_style(_chipWifi, tone, 0);
                    lv_obj_add_style(_chipWifi, Style::borderTones[border], 0);
                }
            }
        }
        if (_lblWifiState) {
            lv_label_set_text(_lblWifiState, (wifiEnabled ? "ON" : "OFF"));
            {
                for (auto *tone : Style::textTones)
                    lv_obj_remove_style(_lblWifiState, tone, 0);
                lv_obj_add_style(_lblWifiState, Style::textTones[color], 0);
            }
        }
    }

    if (reachable) {
        lv_label_set_text(_lblStatus, "ONLINE");
        {
            lv_obj_remove_style(_lblStatus, Style::textSuccess, 0);
            lv_obj_add_style(_lblStatus, Style::textSuccess, 0);
        }
        {
            if (_identityPanel) {
                {
                    for (auto *tone : Style::bgTones)
                        lv_obj_remove_style(_identityPanel, tone, 0);
                    lv_obj_add_style(_identityPanel, Style::bgTones[(Style::BG_ELEVATED)], 0);
                }
                {
                    for (auto *tone : Style::borderTones)
                        lv_obj_remove_style(_identityPanel, tone, 0);
                    lv_obj_add_style(_identityPanel, Style::borderTones[(Style::PRIMARY)], 0);
                }
            }
        }
    } else if (wifiUp || transportActive) {
        lv_label_set_text(_lblStatus, wifiUp ? "WIFI ONLY" : "ROUTING");
        {
            lv_obj_remove_style(_lblStatus, Style::textWarning, 0);
            lv_obj_add_style(_lblStatus, Style::textWarning, 0);
        }
        {
            if (_identityPanel) {
                {
                    for (auto *tone : Style::bgTones)
                        lv_obj_remove_style(_identityPanel, tone, 0);
                    lv_obj_add_style(_identityPanel, Style::bgTones[(Style::BG_ELEVATED)], 0);
                }
                {
                    for (auto *tone : Style::borderTones)
                        lv_obj_remove_style(_identityPanel, tone, 0);
                    lv_obj_add_style(_identityPanel, Style::borderTones[(Style::WARNING_CLR)], 0);
                }
            }
        }
    } else {
        lv_label_set_text(_lblStatus, "OFFLINE");
        {
            lv_obj_remove_style(_lblStatus, Style::textError, 0);
            lv_obj_add_style(_lblStatus, Style::textError, 0);
        }
        {
            if (_identityPanel) {
                {
                    for (auto *tone : Style::bgTones)
                        lv_obj_remove_style(_identityPanel, tone, 0);
                    lv_obj_add_style(_identityPanel, Style::bgTones[(Style::BG_ELEVATED)], 0);
                }
                {
                    for (auto *tone : Style::borderTones)
                        lv_obj_remove_style(_identityPanel, tone, 0);
                    lv_obj_add_style(_identityPanel, Style::borderTones[(Style::BORDER)], 0);
                }
            }
        }
    }

    int online = 0;
    if (_am) {
        online = _am->nodeCount();
    }
    lv_label_set_text_fmt(_lblNodes, "%d", online);
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(_lblNodes, tone, 0);
        lv_obj_add_style(_lblNodes, Style::textTones[online > 0 ? Style::ACCENT : Style::TEXT_SECONDARY], 0);
    }
    {
        if (_statNodes) {
            {
                for (auto *tone : Style::bgTones)
                    lv_obj_remove_style(_statNodes, tone, 0);
                lv_obj_add_style(_statNodes, Style::bgTones[(online > 0 ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED)], 0);
            }
            {
                for (auto *tone : Style::borderTones)
                    lv_obj_remove_style(_statNodes, tone, 0);
                lv_obj_add_style(_statNodes, Style::borderTones[(online > 0 ? Style::PRIMARY : Style::BORDER)], 0);
            }
        }
    }

    bool audioOn = !_cfg || _cfg->settings().audioEnabled;
    lv_label_set_text(_lblPaths, audioOn ? "ON" : "OFF");
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(_lblPaths, tone, 0);
        lv_obj_add_style(_lblPaths, Style::textTones[audioOn ? Style::SUCCESS : Style::TEXT_MUTED], 0);
    }
    {
        if (_statPaths) {
            {
                for (auto *tone : Style::bgTones)
                    lv_obj_remove_style(_statPaths, tone, 0);
                lv_obj_add_style(_statPaths, Style::bgTones[(audioOn ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED)], 0);
            }
            {
                for (auto *tone : Style::borderTones)
                    lv_obj_remove_style(_statPaths, tone, 0);
                lv_obj_add_style(_statPaths, Style::borderTones[(audioOn ? Style::PRIMARY : Style::BORDER)], 0);
            }
        }
    }

#if HAS_GPS
    bool gpsOn = _cfg && _cfg->settings().gpsTimeEnabled;
#else
    bool gpsOn = false;
#endif
    lv_label_set_text(_lblLinks, gpsOn ? "ON" : "OFF");
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(_lblLinks, tone, 0);
        lv_obj_add_style(_lblLinks, Style::textTones[gpsOn ? Style::SUCCESS : Style::TEXT_MUTED], 0);
    }
    {
        if (_statLinks) {
            {
                for (auto *tone : Style::bgTones)
                    lv_obj_remove_style(_statLinks, tone, 0);
                lv_obj_add_style(_statLinks, Style::bgTones[(gpsOn ? Style::PRIMARY_SUBTLE : Style::BG_ELEVATED)], 0);
            }
            {
                for (auto *tone : Style::borderTones)
                    lv_obj_remove_style(_statLinks, tone, 0);
                lv_obj_add_style(_statLinks, Style::borderTones[(gpsOn ? Style::PRIMARY : Style::BORDER)], 0);
            }
        }
    }

    if (!_backend) {
        lv_label_set_text(_lblSummary, "Identity unavailable");
        {
            lv_obj_remove_style(_lblSummary, Style::textWarning, 0);
            lv_obj_add_style(_lblSummary, Style::textWarning, 0);
        }
    } else if (!reachable && transportActive) {
        lv_label_set_text(_lblSummary, "Transport active, no link");
        {
            lv_obj_remove_style(_lblSummary, Style::textWarning, 0);
            lv_obj_add_style(_lblSummary, Style::textWarning, 0);
        }
    } else if (!reachable) {
        lv_label_set_text(_lblSummary, "No active transport");
        {
            lv_obj_remove_style(_lblSummary, Style::textError, 0);
            lv_obj_add_style(_lblSummary, Style::textError, 0);
        }
    } else if (online == 0) {
        lv_label_set_text(_lblSummary, "Listening for peers");
        {
            lv_obj_remove_style(_lblSummary, Style::textSecondary, 0);
            lv_obj_add_style(_lblSummary, Style::textSecondary, 0);
        }
    } else if (online == 1) {
        lv_label_set_text(_lblSummary, "1 peer heard in 30m");
        {
            lv_obj_remove_style(_lblSummary, Style::textAccent, 0);
            lv_obj_add_style(_lblSummary, Style::textAccent, 0);
        }
    } else {
        lv_label_set_text_fmt(_lblSummary, "%d peers heard in 30m", online);
        {
            lv_obj_remove_style(_lblSummary, Style::textAccent, 0);
            lv_obj_add_style(_lblSummary, Style::textAccent, 0);
        }
    }

    if (_backend && _backend->lastAnnounceTime() > 0) {
        char age[16];
        formatAge(now - _backend->lastAnnounceTime(), age, sizeof(age));
        lv_label_set_text_fmt(_lblLastAnnounce, "Announced: %s", age);
    } else {
        lv_label_set_text(_lblLastAnnounce, "Announced: never");
    }
}

bool LvHomeScreen::handleKey(const KeyEvent &event) {
    if (event.enter || event.character == '\n' || event.character == '\r') {
        lv_obj_t *focused = lv_group_get_focused(LvInput::group());
        if (focused == _statPaths) {
            toggleAudio();
            return true;
        }
        if (focused == _chipLora) {
            toggleLora();
            return true;
        }
        if (focused == _chipTcp) {
            toggleTcp();
            return true;
        }
        if (focused == _chipWifi) {
            toggleWiFi();
            return true;
        }
        if (focused == _statLinks) {
            toggleGPS();
            return true;
        }
        if (focused == _statNodes) {
            openPeers();
            return true;
        }
        if (_announceCb)
            _announceCb();
        return true;
    }
    return false;
}

void LvHomeScreen::toggleAudio() {
    if (_audioToggleCb)
        _audioToggleCb();
    forceRefresh();
}

void LvHomeScreen::toggleLora() {
    if (_loraToggleCb)
        _loraToggleCb();
    forceRefresh();
}

void LvHomeScreen::toggleTcp() {
    if (_tcpToggleCb)
        _tcpToggleCb();
    forceRefresh();
}

void LvHomeScreen::toggleWiFi() {
    if (_wifiToggleCb)
        _wifiToggleCb();
    forceRefresh();
}

void LvHomeScreen::toggleGPS() {
    if (_gpsToggleCb)
        _gpsToggleCb();
    forceRefresh();
}

void LvHomeScreen::openPeers() {
    if (_peersCb)
        _peersCb();
}

void LvHomeScreen::forceRefresh() {
    _lastRefreshMs = 0;
    _lastUptime = ULONG_MAX;
    _lastHeap = UINT32_MAX;
    refreshUI();
}

void LvHomeScreen::renderAvatar(const String &seed) {
    if (seed.isEmpty() || seed == _avatarSeed || !_avatarCanvas)
        return;
    if (LxmFaceAvatar::render(_avatarCanvas, seed)) {
        _avatarSeed = seed;
    }
}
