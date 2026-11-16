#include "Style.h"

// Fixed color theme, shared by every display profile. No geometry or fonts.
namespace Style {

// Same constant-table representation as LV_STYLE_CONST_INIT, with the fields
// in C++ declaration order and mutable handles for LVGL's lv_style_t* API.
#if LV_USE_ASSERT_STYLE
#define STYLE_HANDLE(name)                                                                                                                                              \
    static lv_style_t name##Style = { LV_STYLE_SENTINEL_VALUE, { .const_props = name##Props }, LV_STYLE_PROP_ANY, 0xFF, sizeof(name##Props) / sizeof(name##Props[0]) }; \
    lv_style_t       *name = &name##Style
#else
#define STYLE_HANDLE(name)                                                                                                                     \
    static lv_style_t name##Style = { { .const_props = name##Props }, LV_STYLE_PROP_ANY, 0xFF, sizeof(name##Props) / sizeof(name##Props[0]) }; \
    lv_style_t       *name = &name##Style
#endif

static const lv_style_const_prop_t themeProps[] = {
    { LV_STYLE_BG_COLOR,   { .color = LV_COLOR_MAKE(0x05, 0x08, 0x0A) } },
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0xE4, 0xF3, 0xF0) } },
};
STYLE_HANDLE(theme);

static const lv_style_const_prop_t textPrimaryProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0xE4, 0xF3, 0xF0) } },
};
STYLE_HANDLE(textPrimary);

static const lv_style_const_prop_t textSecondaryProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x91, 0xA8, 0xA5) } },
};
STYLE_HANDLE(textSecondary);

static const lv_style_const_prop_t textAccentProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x4F, 0xD7, 0xFF) } },
};
STYLE_HANDLE(textAccent);

static const lv_style_const_prop_t buttonColorProps[] = {
    { LV_STYLE_BG_COLOR,     { .color = LV_COLOR_MAKE(0x0C, 0x14, 0x18) } },
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x21, 0x38, 0x3B) } },
    { LV_STYLE_TEXT_COLOR,   { .color = LV_COLOR_MAKE(0xE4, 0xF3, 0xF0) } },
};
STYLE_HANDLE(buttonColor);

static const lv_style_const_prop_t buttonPressedColorProps[] = {
    { LV_STYLE_BG_COLOR,     { .color = LV_COLOR_MAKE(0x06, 0x25, 0x1A) } },
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x00, 0xE0, 0x6D) } },
    { LV_STYLE_TEXT_COLOR,   { .color = LV_COLOR_MAKE(0x4F, 0xD7, 0xFF) } },
};
STYLE_HANDLE(buttonPressedColor);

static const lv_style_const_prop_t buttonFocusedColorProps[] = {
    { LV_STYLE_BORDER_COLOR,  { .color = LV_COLOR_MAKE(0x00, 0xE0, 0x6D) } },
    { LV_STYLE_OUTLINE_COLOR, { .color = LV_COLOR_MAKE(0x4F, 0xD7, 0xFF) } },
};
STYLE_HANDLE(buttonFocusedColor);

static const lv_style_const_prop_t barColorProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x21, 0x38, 0x3B) } },
};
STYLE_HANDLE(barColor);

static const lv_style_const_prop_t barIndicatorColorProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x00, 0xE0, 0x6D) } },
};
STYLE_HANDLE(barIndicatorColor);

static const lv_style_const_prop_t textareaFocusedColorProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x00, 0xE0, 0x6D) } },
};
STYLE_HANDLE(textareaFocusedColor);

static const lv_style_const_prop_t listColorProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x05, 0x08, 0x0A) } },
};
STYLE_HANDLE(listColor);

static const lv_style_const_prop_t listItemColorProps[] = {
    { LV_STYLE_BG_COLOR,     { .color = LV_COLOR_MAKE(0x05, 0x08, 0x0A) } },
    { LV_STYLE_TEXT_COLOR,   { .color = LV_COLOR_MAKE(0xE4, 0xF3, 0xF0) } },
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x10, 0x23, 0x29) } },
};
STYLE_HANDLE(listItemColor);

static const lv_style_const_prop_t listItemFocusedColorProps[] = {
    { LV_STYLE_BG_COLOR,     { .color = LV_COLOR_MAKE(0x18, 0x30, 0x33) } },
    { LV_STYLE_TEXT_COLOR,   { .color = LV_COLOR_MAKE(0xE4, 0xF3, 0xF0) } },
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x00, 0xE0, 0x6D) } },
};
STYLE_HANDLE(listItemFocusedColor);

static const lv_style_const_prop_t modalColorProps[] = {
    { LV_STYLE_BG_COLOR,     { .color = LV_COLOR_MAKE(0x12, 0x1D, 0x23) } },
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x21, 0x38, 0x3B) } },
    { LV_STYLE_SHADOW_COLOR, { .color = LV_COLOR_MAKE(0x00, 0x00, 0x00) } },
};
STYLE_HANDLE(modalColor);

static const lv_style_const_prop_t scrollbarColorProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x52, 0x68, 0x66) } },
};
STYLE_HANDLE(scrollbarColor);

static const lv_style_const_prop_t rollerColorProps[] = {
    { LV_STYLE_BG_COLOR,   { .color = LV_COLOR_MAKE(0x05, 0x08, 0x0A) } },
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x91, 0xA8, 0xA5) } },
};
STYLE_HANDLE(rollerColor);

static const lv_style_const_prop_t statusBarColorProps[] = {
    { LV_STYLE_BG_COLOR,     { .color = LV_COLOR_MAKE(0x07, 0x10, 0x14) } },
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x10, 0x23, 0x29) } },
};
STYLE_HANDLE(statusBarColor);

static const lv_style_const_prop_t statusToastColorProps[] = {
    { LV_STYLE_BG_COLOR,   { .color = LV_COLOR_MAKE(0x00, 0xE0, 0x6D) } },
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x05, 0x08, 0x0A) } },
};
STYLE_HANDLE(statusToastColor);

static const lv_style_const_prop_t bgElevatedProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x0C, 0x14, 0x18) } },
};
STYLE_HANDLE(bgElevated);

static const lv_style_const_prop_t bgPressedProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x00, 0xA8, 0x53) } },
};
STYLE_HANDLE(bgPressed);

static const lv_style_const_prop_t borderBorderProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x21, 0x38, 0x3B) } },
};
STYLE_HANDLE(borderBorder);

static const lv_style_const_prop_t borderAccentProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x4F, 0xD7, 0xFF) } },
};
STYLE_HANDLE(borderAccent);

static const lv_style_const_prop_t outlinePrimaryProps[] = {
    { LV_STYLE_OUTLINE_COLOR, { .color = LV_COLOR_MAKE(0x00, 0xE0, 0x6D) } },
};
STYLE_HANDLE(outlinePrimary);

static const lv_style_const_prop_t bgBatteryTrackProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x44, 0x44, 0x44) } },
};
STYLE_HANDLE(bgBatteryTrack);

static const lv_style_const_prop_t bgWhiteProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF) } },
};
STYLE_HANDLE(bgWhite);

static const lv_style_const_prop_t textBackgroundProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x05, 0x08, 0x0A) } },
};
STYLE_HANDLE(textBackground);

static const lv_style_const_prop_t lineWarningProps[] = {
    { LV_STYLE_LINE_COLOR, { .color = LV_COLOR_MAKE(0xF0, 0xC0, 0x4F) } },
};
STYLE_HANDLE(lineWarning);

static const lv_style_const_prop_t bgTabProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x08, 0x11, 0x15) } },
};
STYLE_HANDLE(bgTab);

static const lv_style_const_prop_t textActiveTabTextProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x00, 0xE0, 0x6D) } },
};
STYLE_HANDLE(textActiveTabText);

static const lv_style_const_prop_t bgActiveTabProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x0B, 0x24, 0x1B) } },
};
STYLE_HANDLE(bgActiveTab);

static const lv_style_const_prop_t textMutedProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x52, 0x68, 0x66) } },
};
STYLE_HANDLE(textMuted);

static const lv_style_const_prop_t bgSelectionProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x06, 0x25, 0x1A) } },
};
STYLE_HANDLE(bgSelection);

static const lv_style_const_prop_t bgDividerProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x10, 0x23, 0x29) } },
};
STYLE_HANDLE(bgDivider);

static const lv_style_const_prop_t textWarningProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0xF0, 0xC0, 0x4F) } },
};
STYLE_HANDLE(textWarning);

static const lv_style_const_prop_t textSuccessProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x31, 0xE9, 0x81) } },
};
STYLE_HANDLE(textSuccess);

static const lv_style_const_prop_t textErrorProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0xFF, 0x5C, 0x6C) } },
};
STYLE_HANDLE(textError);

static const lv_style_const_prop_t linePrimaryProps[] = {
    { LV_STYLE_LINE_COLOR, { .color = LV_COLOR_MAKE(0x00, 0xE0, 0x6D) } },
};
STYLE_HANDLE(linePrimary);

static const lv_style_const_prop_t bgHoverProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x18, 0x30, 0x33) } },
};
STYLE_HANDLE(bgHover);

static const lv_style_const_prop_t bgOutgoingProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x08, 0x27, 0x19) } },
};
STYLE_HANDLE(bgOutgoing);

static const lv_style_const_prop_t bgIncomingProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x0B, 0x15, 0x1C) } },
};
STYLE_HANDLE(bgIncoming);

static const lv_style_const_prop_t borderWarningProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0xF0, 0xC0, 0x4F) } },
};
STYLE_HANDLE(borderWarning);

static const lv_style_const_prop_t bgSurfaceProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x12, 0x1D, 0x23) } },
};
STYLE_HANDLE(bgSurface);

static const lv_style_const_prop_t textBgElevatedProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x0C, 0x14, 0x18) } },
};
STYLE_HANDLE(textBgElevated);

static const lv_style_const_prop_t textBgSurfaceProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x12, 0x1D, 0x23) } },
};
STYLE_HANDLE(textBgSurface);

static const lv_style_const_prop_t textBgHoverProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x18, 0x30, 0x33) } },
};
STYLE_HANDLE(textBgHover);

static const lv_style_const_prop_t textPrimaryMutedProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x00, 0xA8, 0x53) } },
};
STYLE_HANDLE(textPrimaryMuted);

static const lv_style_const_prop_t textPrimarySubtleProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x06, 0x25, 0x1A) } },
};
STYLE_HANDLE(textPrimarySubtle);

static const lv_style_const_prop_t textErrorSubtleProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x2A, 0x0E, 0x0E) } },
};
STYLE_HANDLE(textErrorSubtle);

static const lv_style_const_prop_t textBorderProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x21, 0x38, 0x3B) } },
};
STYLE_HANDLE(textBorder);

static const lv_style_const_prop_t textDividerProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x10, 0x23, 0x29) } },
};
STYLE_HANDLE(textDivider);

static const lv_style_const_prop_t textMsgOutBgProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x08, 0x27, 0x19) } },
};
STYLE_HANDLE(textMsgOutBg);

static const lv_style_const_prop_t textMsgInBgProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x0B, 0x15, 0x1C) } },
};
STYLE_HANDLE(textMsgInBg);

static const lv_style_const_prop_t textStatusBgProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x07, 0x10, 0x14) } },
};
STYLE_HANDLE(textStatusBg);

static const lv_style_const_prop_t textStatusFlashProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x12, 0x35, 0x1F) } },
};
STYLE_HANDLE(textStatusFlash);

static const lv_style_const_prop_t textTabBgProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x08, 0x11, 0x15) } },
};
STYLE_HANDLE(textTabBg);

static const lv_style_const_prop_t textTabActiveBgProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0x0B, 0x24, 0x1B) } },
};
STYLE_HANDLE(textTabActiveBg);

static const lv_style_const_prop_t textWhiteProps[] = {
    { LV_STYLE_TEXT_COLOR, { .color = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF) } },
};
STYLE_HANDLE(textWhite);

static const lv_style_const_prop_t bgTextPrimaryProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0xE4, 0xF3, 0xF0) } },
};
STYLE_HANDLE(bgTextPrimary);

static const lv_style_const_prop_t bgTextSecondaryProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x91, 0xA8, 0xA5) } },
};
STYLE_HANDLE(bgTextSecondary);

static const lv_style_const_prop_t bgAccentProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x4F, 0xD7, 0xFF) } },
};
STYLE_HANDLE(bgAccent);

static const lv_style_const_prop_t bgSuccessProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x31, 0xE9, 0x81) } },
};
STYLE_HANDLE(bgSuccess);

static const lv_style_const_prop_t bgWarningClrProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0xF0, 0xC0, 0x4F) } },
};
STYLE_HANDLE(bgWarningClr);

static const lv_style_const_prop_t bgErrorClrProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0xFF, 0x5C, 0x6C) } },
};
STYLE_HANDLE(bgErrorClr);

static const lv_style_const_prop_t bgErrorSubtleProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x2A, 0x0E, 0x0E) } },
};
STYLE_HANDLE(bgErrorSubtle);

static const lv_style_const_prop_t bgStatusBgProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x07, 0x10, 0x14) } },
};
STYLE_HANDLE(bgStatusBg);

static const lv_style_const_prop_t bgStatusFlashProps[] = {
    { LV_STYLE_BG_COLOR, { .color = LV_COLOR_MAKE(0x12, 0x35, 0x1F) } },
};
STYLE_HANDLE(bgStatusFlash);

static const lv_style_const_prop_t borderBgProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x05, 0x08, 0x0A) } },
};
STYLE_HANDLE(borderBg);

static const lv_style_const_prop_t borderBgElevatedProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x0C, 0x14, 0x18) } },
};
STYLE_HANDLE(borderBgElevated);

static const lv_style_const_prop_t borderBgSurfaceProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x12, 0x1D, 0x23) } },
};
STYLE_HANDLE(borderBgSurface);

static const lv_style_const_prop_t borderBgHoverProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x18, 0x30, 0x33) } },
};
STYLE_HANDLE(borderBgHover);

static const lv_style_const_prop_t borderTextPrimaryProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0xE4, 0xF3, 0xF0) } },
};
STYLE_HANDLE(borderTextPrimary);

static const lv_style_const_prop_t borderTextSecondaryProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x91, 0xA8, 0xA5) } },
};
STYLE_HANDLE(borderTextSecondary);

static const lv_style_const_prop_t borderTextMutedProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x52, 0x68, 0x66) } },
};
STYLE_HANDLE(borderTextMuted);

static const lv_style_const_prop_t borderPrimaryMutedProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x00, 0xA8, 0x53) } },
};
STYLE_HANDLE(borderPrimaryMuted);

static const lv_style_const_prop_t borderPrimarySubtleProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x06, 0x25, 0x1A) } },
};
STYLE_HANDLE(borderPrimarySubtle);

static const lv_style_const_prop_t borderSuccessProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x31, 0xE9, 0x81) } },
};
STYLE_HANDLE(borderSuccess);

static const lv_style_const_prop_t borderErrorClrProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0xFF, 0x5C, 0x6C) } },
};
STYLE_HANDLE(borderErrorClr);

static const lv_style_const_prop_t borderErrorSubtleProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x2A, 0x0E, 0x0E) } },
};
STYLE_HANDLE(borderErrorSubtle);

static const lv_style_const_prop_t borderDividerProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x10, 0x23, 0x29) } },
};
STYLE_HANDLE(borderDivider);

static const lv_style_const_prop_t borderMsgOutBgProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x08, 0x27, 0x19) } },
};
STYLE_HANDLE(borderMsgOutBg);

static const lv_style_const_prop_t borderMsgInBgProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x0B, 0x15, 0x1C) } },
};
STYLE_HANDLE(borderMsgInBg);

static const lv_style_const_prop_t borderStatusBgProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x07, 0x10, 0x14) } },
};
STYLE_HANDLE(borderStatusBg);

static const lv_style_const_prop_t borderStatusFlashProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x12, 0x35, 0x1F) } },
};
STYLE_HANDLE(borderStatusFlash);

static const lv_style_const_prop_t borderTabBgProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x08, 0x11, 0x15) } },
};
STYLE_HANDLE(borderTabBg);

static const lv_style_const_prop_t borderTabActiveBgProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0x0B, 0x24, 0x1B) } },
};
STYLE_HANDLE(borderTabActiveBg);

static const lv_style_const_prop_t borderWhiteProps[] = {
    { LV_STYLE_BORDER_COLOR, { .color = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF) } },
};
STYLE_HANDLE(borderWhite);

#undef STYLE_HANDLE

lv_style_t *textTones[ToneCount] = {
    &textBackgroundStyle,
    &textBgElevatedStyle,
    &textBgSurfaceStyle,
    &textBgHoverStyle,
    &textPrimaryStyle,
    &textSecondaryStyle,
    &textMutedStyle,
    &textActiveTabTextStyle,
    &textPrimaryMutedStyle,
    &textPrimarySubtleStyle,
    &textAccentStyle,
    &textSuccessStyle,
    &textWarningStyle,
    &textErrorStyle,
    &textErrorSubtleStyle,
    &textBorderStyle,
    &textActiveTabTextStyle,
    &textDividerStyle,
    &textMsgOutBgStyle,
    &textMsgInBgStyle,
    &textStatusBgStyle,
    &textStatusFlashStyle,
    &textTabBgStyle,
    &textTabActiveBgStyle,
    &textActiveTabTextStyle,
    &textSecondaryStyle,
    &textErrorStyle,
    &textActiveTabTextStyle,
    &textWhiteStyle,
};

lv_style_t *bgTones[ToneCount] = {
    &listColorStyle,
    &bgElevatedStyle,
    &bgSurfaceStyle,
    &bgHoverStyle,
    &bgTextPrimaryStyle,
    &bgTextSecondaryStyle,
    &scrollbarColorStyle,
    &barIndicatorColorStyle,
    &bgPressedStyle,
    &bgSelectionStyle,
    &bgAccentStyle,
    &bgSuccessStyle,
    &bgWarningClrStyle,
    &bgErrorClrStyle,
    &bgErrorSubtleStyle,
    &barColorStyle,
    &barIndicatorColorStyle,
    &bgDividerStyle,
    &bgOutgoingStyle,
    &bgIncomingStyle,
    &bgStatusBgStyle,
    &bgStatusFlashStyle,
    &bgTabStyle,
    &bgActiveTabStyle,
    &barIndicatorColorStyle,
    &bgTextSecondaryStyle,
    &bgErrorClrStyle,
    &barIndicatorColorStyle,
    &bgWhiteStyle,
};

lv_style_t *borderTones[ToneCount] = {
    &borderBgStyle,
    &borderBgElevatedStyle,
    &borderBgSurfaceStyle,
    &borderBgHoverStyle,
    &borderTextPrimaryStyle,
    &borderTextSecondaryStyle,
    &borderTextMutedStyle,
    &textareaFocusedColorStyle,
    &borderPrimaryMutedStyle,
    &borderPrimarySubtleStyle,
    &borderAccentStyle,
    &borderSuccessStyle,
    &borderWarningStyle,
    &borderErrorClrStyle,
    &borderErrorSubtleStyle,
    &borderBorderStyle,
    &textareaFocusedColorStyle,
    &borderDividerStyle,
    &borderMsgOutBgStyle,
    &borderMsgInBgStyle,
    &borderStatusBgStyle,
    &borderStatusFlashStyle,
    &borderTabBgStyle,
    &borderTabActiveBgStyle,
    &textareaFocusedColorStyle,
    &borderTextSecondaryStyle,
    &borderErrorClrStyle,
    &textareaFocusedColorStyle,
    &borderWhiteStyle,
};
} // namespace Style
