#include "LvStatusBar.h"
#include "Style.h"
#include "ClockText.h"
#include <Arduino.h>
#include <cstdio>
#include <time.h>

namespace {

int normalizedBattery(int pct) {
    if (pct < 0)
        return -1;
    if (pct > 100)
        return 100;
    return pct;
}

} // namespace

void LvStatusBar::create(lv_obj_t *parent) {
    _bar = lv_obj_create(parent);
    lv_obj_add_style(_bar, Style::statusBarPosition, 0);

    lv_obj_add_style(_bar, Style::statusBarColor, 0);
    lv_obj_add_style(_bar, Style::statusBar, 0);
    lv_obj_clear_flag(_bar, LV_OBJ_FLAG_SCROLLABLE);

    // Left: time. Fixed width keeps the center indicators stable.
    _lblTime = lv_label_create(_bar);
    lv_obj_add_style(_lblTime, Style::clockPosition, 0);
    lv_label_set_long_mode(_lblTime, LV_LABEL_LONG_CLIP);
    lv_obj_add_style(_lblTime, Style::fontBody, 0);

    lv_label_set_text(_lblTime, "--:--");

    // Center: brand/status text. Leave a fixed slot beside the battery for Wi-Fi.
    _lblLinks = lv_label_create(_bar);
    lv_obj_add_style(_lblLinks, Style::transportPosition, 0);
    lv_label_set_long_mode(_lblLinks, LV_LABEL_LONG_CLIP);
    lv_label_set_recolor(_lblLinks, true);
    lv_obj_add_style(_lblLinks, Style::fontCaption, 0);

    // Right: battery percentage.
    _lblBatt = lv_label_create(_bar);
    lv_obj_add_style(_lblBatt, Style::batteryTextPosition, 0);
    lv_obj_add_style(_lblBatt, Style::fontBody, 0);

    lv_label_set_long_mode(_lblBatt, LV_LABEL_LONG_CLIP);

    // Right: battery bar
    _barBatt = lv_bar_create(_bar);
    lv_obj_add_style(_barBatt, Style::batteryGaugePosition, 0);
    lv_bar_set_range(_barBatt, 0, 100);
    lv_bar_set_value(_barBatt, 0, LV_ANIM_OFF);

    lv_obj_add_style(_barBatt, Style::bgBatteryTrack, LV_PART_MAIN);
    lv_obj_add_style(_barBatt, Style::opaque, LV_PART_MAIN);
    lv_obj_add_style(_barBatt, Style::bgWhite, LV_PART_INDICATOR);
    lv_obj_add_style(_barBatt, Style::opaque, LV_PART_INDICATOR);

    _wifiIcon = lv_label_create(_bar);
    lv_label_set_text(_wifiIcon, LV_SYMBOL_WIFI);
    lv_obj_add_style(_wifiIcon, Style::fontSymbol, 0);
    lv_obj_add_style(_wifiIcon, Style::wifiPosition, 0);
    _wifiSlash = lv_line_create(_wifiIcon);
    static const lv_point_t slash[] = {
        { 0,  13 },
        { 15, 0  }
    };
    lv_line_set_points(_wifiSlash, slash, 2);
    lv_obj_add_style(_wifiSlash, Style::disabledSlash, 0);

    // Toast overlay (hidden by default). It is a child of the bar so boot mode
    // and other shell visibility changes affect it with the status bar.
    _toast = lv_obj_create(_bar);
    lv_obj_add_style(_toast, Style::statusBarPosition, 0);

    lv_obj_add_style(_toast, Style::statusToastColor, 0);
    lv_obj_add_style(_toast, Style::statusToast, 0);
    lv_obj_clear_flag(_toast, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(_toast, LV_OBJ_FLAG_HIDDEN);

    _lblToast = lv_label_create(_toast);
    lv_obj_add_style(_lblToast, Style::toastCaption, 0);
    lv_label_set_long_mode(_lblToast, LV_LABEL_LONG_DOT);
    lv_obj_add_style(_lblToast, Style::fontBody, 0);
    lv_obj_add_style(_lblToast, Style::textBackground, 0);

    lv_label_set_text(_lblToast, "");

    refreshTimeColor();
    refreshIndicators();
    refreshBattery();
}

void LvStatusBar::update() {
    if (_announceFlashEnd > 0 && millis() >= _announceFlashEnd) {
        _announceFlashEnd = 0;
        refreshIndicators();
    }
    if (_toast && _toastEnd > 0 && millis() >= _toastEnd) {
        _toastEnd = 0;
        lv_obj_add_flag(_toast, LV_OBJ_FLAG_HIDDEN);
    }
}

void LvStatusBar::updateTime() {
    if (!_lblTime)
        return;
    time_t now = time(nullptr);
    if (now <= 1700000000) {
        if (_lastHour != -2) {
            lv_label_set_text(_lblTime, "--:--");
            _lastHour = -2;
            _lastMinute = -2;
            refreshTimeColor();
        }
        return;
    }
    struct tm  converted;
    struct tm *local = localtime_r(&now, &converted);
    if (!local) {
        if (_lastHour != -2) {
            lv_label_set_text(_lblTime, "--:--");
            _lastHour = -2;
            _lastMinute = -2;
            refreshTimeColor();
        }
        return;
    }
    if (local->tm_hour == _lastHour && local->tm_min == _lastMinute)
        return;
    _lastHour = local->tm_hour;
    _lastMinute = local->tm_min;
    char buf[8];
    formatClockText(buf, sizeof(buf), *local, _use24h);
    lv_label_set_text(_lblTime, buf);
    refreshTimeColor();
}

void LvStatusBar::setLoRaOnline(bool online) {
    if (_loraOnline == online)
        return;
    _loraOnline = online;
    refreshIndicators();
}

void LvStatusBar::setBLEActive(bool active) {
    if (_bleActive == active)
        return;
    _bleActive = active;
    refreshIndicators();
}

void LvStatusBar::setBLEEnabled(bool enabled) {
    if (_bleEnabled == enabled)
        return;
    _bleEnabled = enabled;
    refreshIndicators();
}

void LvStatusBar::setWiFiActive(bool active) {
    if (_wifiActive == active)
        return;
    _wifiActive = active;
    refreshIndicators();
}

void LvStatusBar::setWiFiEnabled(bool enabled) {
    if (_wifiEnabled == enabled)
        return;
    _wifiEnabled = enabled;
    refreshIndicators();
}

void LvStatusBar::setTCPConnected(bool connected) {
    if (_tcpConnected == connected)
        return;
    _tcpConnected = connected;
    refreshIndicators();
}

void LvStatusBar::setAutoIfacePeers(int n) {
    if (n < -1)
        n = -1;
    if (_autoIfacePeers == n)
        return;
    _autoIfacePeers = n;
    refreshIndicators();
}

void LvStatusBar::setGPSFix(bool hasFix) {
    if (_gpsFix == hasFix)
        return;
    _gpsFix = hasFix;
    refreshIndicators();
    refreshTimeColor();
}

void LvStatusBar::setBatteryPercent(int pct) {
    pct = normalizedBattery(pct);

    // Suppress single-percent ADC noise: only update if the value changed by 2% or
    // more. A -1 reading (fuel gauge not ready) always propagates as "--%".
    if (_battPct >= 0 && pct >= 0 && abs(pct - _battPct) < 2)
        return;

    _battPct = pct;
    refreshBattery();
}

void LvStatusBar::setBatteryDisplay(uint8_t mode) {
    if (_battDisplay == mode)
        return;
    _battDisplay = mode;
    refreshBattery();
}

void LvStatusBar::flashAnnounce() {
    _announceFlashEnd = millis() + 1000;
    refreshIndicators();
}

void LvStatusBar::showToast(const char *msg, uint32_t durationMs) {
    if (!_toast || !_lblToast)
        return;
    lv_label_set_text(_lblToast, msg);
    _toastEnd = millis() + durationMs;
    lv_obj_clear_flag(_toast, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(_toast);
}

void LvStatusBar::setUse24Hour(bool use24h) {
    if (_use24h == use24h)
        return;
    _use24h = use24h;
    _lastHour = -1;
    _lastMinute = -1;
    updateTime();
}

void LvStatusBar::refreshIndicators() {
    if (!_lblLinks)
        return;

    if (_wifiIcon) {
        if (_wifiEnabled)
            lv_obj_clear_flag(_wifiIcon, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(_wifiIcon, LV_OBJ_FLAG_HIDDEN);
        {
            for (auto *tone : Style::textTones)
                lv_obj_remove_style(_wifiIcon, tone, 0);
            lv_obj_add_style(_wifiIcon, Style::textTones[_wifiActive ? Style::SUCCESS : Style::WARNING_CLR], 0);
        }
        {
            lv_obj_remove_style(_wifiSlash, Style::lineWarning, 0);
            lv_obj_add_style(_wifiSlash, Style::lineWarning, 0);
        }
        if (_wifiActive)
            lv_obj_add_flag(_wifiSlash, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(_wifiSlash, LV_OBJ_FLAG_HIDDEN);
    }

    const bool txFlash = _announceFlashEnd > 0 && millis() < _announceFlashEnd;
    lv_label_set_recolor(_lblLinks, false);
    lv_label_set_text_static(_lblLinks, "Ratspeak.org");
    for (auto *tone : Style::textTones)
        lv_obj_remove_style(_lblLinks, tone, 0);
    lv_obj_add_style(_lblLinks, Style::textTones[txFlash ? Style::WARNING_CLR : Style::ACCENT], 0);

    if (_bar) {
        {
            for (auto *tone : Style::bgTones)
                lv_obj_remove_style(_bar, tone, 0);
            lv_obj_add_style(_bar, Style::bgTones[txFlash ? Style::STATUS_FLASH : Style::STATUS_BG], 0);
        }
        {
            for (auto *tone : Style::borderTones)
                lv_obj_remove_style(_bar, tone, 0);
            lv_obj_add_style(_bar, Style::borderTones[txFlash ? Style::PRIMARY : Style::DIVIDER], 0);
        }
    }
}

void LvStatusBar::refreshBattery() {
    if (!_lblBatt || !_barBatt)
        return;

    if (_battDisplay == 1) {
        // Bar mode
        lv_obj_clear_flag(_barBatt, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_lblBatt, LV_OBJ_FLAG_HIDDEN);

        lv_bar_set_value(_barBatt, _battPct < 0 ? 0 : _battPct, LV_ANIM_OFF);

        uint32_t indicatorCol;
        if (_isCharging) {
            indicatorCol = Style::SUCCESS;

        } else if (_battPct >= 0 && _battPct <= 10) {
            indicatorCol = Style::ERROR_CLR;
        } else if (_battPct >= 0 && _battPct <= 20) {
            indicatorCol = Style::WARNING_CLR;

        } else {
            indicatorCol = Style::WHITE;
        }
        {
            for (auto *tone : Style::bgTones)
                lv_obj_remove_style(_barBatt, tone, LV_PART_INDICATOR);
            lv_obj_add_style(_barBatt, Style::bgTones[indicatorCol], LV_PART_INDICATOR);
        }

    } else {
        // Percent mode
        lv_obj_clear_flag(_lblBatt, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_barBatt, LV_OBJ_FLAG_HIDDEN);

        char     buf[8];
        uint32_t col = Style::TEXT_MUTED;
        if (_battPct < 0) {
            snprintf(buf, sizeof(buf), "--%%");
        } else if (_isCharging) {
            snprintf(buf, sizeof(buf), "+%d%%", _battPct);
            col = Style::SUCCESS;
        } else {
            snprintf(buf, sizeof(buf), "%d%%", _battPct);
            col = Style::TEXT_PRIMARY;
            if (_battPct <= 10)
                col = Style::ERROR_CLR;
            else if (_battPct <= 20)
                col = Style::WARNING_CLR;
        }
        lv_label_set_text(_lblBatt, buf);
        {
            for (auto *tone : Style::textTones)
                lv_obj_remove_style(_lblBatt, tone, 0);
            lv_obj_add_style(_lblBatt, Style::textTones[col], 0);
        }
    }
}

void LvStatusBar::setCharging(bool charging) {
    if (_isCharging == charging)
        return;
    _isCharging = charging;
    refreshBattery();
}

void LvStatusBar::refreshTimeColor() {
    if (!_lblTime)
        return;
    const bool     validTime = _lastHour >= 0;
    const uint32_t col = validTime
                             ? (_gpsFix ? Style::ACCENT : Style::TEXT_PRIMARY)
                             : Style::TEXT_MUTED;
    {
        for (auto *tone : Style::textTones)
            lv_obj_remove_style(_lblTime, tone, 0);
        lv_obj_add_style(_lblTime, Style::textTones[col], 0);
    }
}
