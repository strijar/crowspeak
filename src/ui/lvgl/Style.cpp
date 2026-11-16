#include "Style.h"
#include "fonts/fonts.h"

// CrowPanel 800 x 480 geometry and typography. Colors live in Theme.cpp.
// Static property tables: no initialization routine, heap allocation, or accessors.
namespace Style {

// LVGL 8.3.11 registers flex first in lv_extra_init(). Its exported IDs
// are zero before lv_init(), so static tables must use the registration
// constants, not those runtime variables. Checked by the native UI smoke test.
static_assert(LVGL_VERSION_MAJOR == 8 && LVGL_VERSION_MINOR == 3 && LVGL_VERSION_PATCH == 11, "Review flex registration IDs when updating LVGL");
constexpr int             FlexLayout = 1;
constexpr lv_style_prop_t FlexFlow = lv_style_prop_t(_LV_STYLE_LAST_BUILT_IN_PROP + 1);
constexpr lv_style_prop_t FlexMain = lv_style_prop_t(_LV_STYLE_LAST_BUILT_IN_PROP + 2);
constexpr lv_style_prop_t FlexCross = lv_style_prop_t(_LV_STYLE_LAST_BUILT_IN_PROP + 3);
constexpr lv_style_prop_t FlexTrack = lv_style_prop_t(_LV_STYLE_LAST_BUILT_IN_PROP + 4);
constexpr lv_style_prop_t FlexGrow = lv_style_prop_t(_LV_STYLE_LAST_BUILT_IN_PROP + 5);

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

static const lv_style_const_prop_t screenProps[] = {
    { LV_STYLE_BG_OPA,    { .num = LV_OPA_COVER }       },
    { LV_STYLE_TEXT_FONT, { .ptr = &lv_font_rsdeck_14 } },
};
STYLE_HANDLE(screen);

static const lv_style_const_prop_t fontTitleProps[] = {
    { LV_STYLE_TEXT_FONT, { .ptr = &lv_font_rsdeck_14 } },
};
STYLE_HANDLE(fontTitle);

static const lv_style_const_prop_t fontBodyProps[] = {
    { LV_STYLE_TEXT_FONT, { .ptr = &lv_font_rsdeck_12 } },
};
STYLE_HANDLE(fontBody);

static const lv_style_const_prop_t buttonProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }       },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                  },
    { LV_STYLE_TEXT_FONT,    { .ptr = &lv_font_rsdeck_12 } },
    { LV_STYLE_RADIUS,       { .num = 3 }                  },
    { LV_STYLE_PAD_TOP,      { .num = 5 }                  },
    { LV_STYLE_PAD_BOTTOM,   { .num = 5 }                  },
    { LV_STYLE_PAD_LEFT,     { .num = 8 }                  },
    { LV_STYLE_PAD_RIGHT,    { .num = 8 }                  },
};
STYLE_HANDLE(button);

static const lv_style_const_prop_t buttonPressedProps[] = {
    { LV_STYLE_BORDER_WIDTH, { .num = 1 } },
};
STYLE_HANDLE(buttonPressed);

static const lv_style_const_prop_t buttonFocusedProps[] = {
    { LV_STYLE_BORDER_WIDTH,  { .num = 2 } },
    { LV_STYLE_OUTLINE_WIDTH, { .num = 1 } },
    { LV_STYLE_OUTLINE_PAD,   { .num = 0 } },
};
STYLE_HANDLE(buttonFocused);

static const lv_style_const_prop_t barProps[] = {
    { LV_STYLE_BG_OPA, { .num = LV_OPA_COVER } },
    { LV_STYLE_RADIUS, { .num = 2 }            },
};
STYLE_HANDLE(bar);

static const lv_style_const_prop_t textareaProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }       },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                  },
    { LV_STYLE_TEXT_FONT,    { .ptr = &lv_font_rsdeck_14 } },
    { LV_STYLE_RADIUS,       { .num = 3 }                  },
    { LV_STYLE_PAD_LEFT,     { .num = 6 }                  },
    { LV_STYLE_PAD_RIGHT,    { .num = 6 }                  },
    { LV_STYLE_PAD_TOP,      { .num = 6 }                  },
    { LV_STYLE_PAD_BOTTOM,   { .num = 6 }                  },
};
STYLE_HANDLE(textarea);

static const lv_style_const_prop_t listProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_PAD_ROW,      { .num = 0 }            },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_RADIUS,       { .num = 0 }            },
};
STYLE_HANDLE(list);

static const lv_style_const_prop_t listItemProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }          },
    { LV_STYLE_TEXT_FONT,    { .ptr = &lv_font_rsdeck_12 }    },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                     },
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_BOTTOM } },
    { LV_STYLE_PAD_TOP,      { .num = 5 }                     },
    { LV_STYLE_PAD_BOTTOM,   { .num = 5 }                     },
    { LV_STYLE_PAD_LEFT,     { .num = 8 }                     },
    { LV_STYLE_PAD_RIGHT,    { .num = 8 }                     },
    { LV_STYLE_RADIUS,       { .num = 0 }                     },
};
STYLE_HANDLE(listItem);

static const lv_style_const_prop_t listItemFocusedProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }        },
    { LV_STYLE_BORDER_WIDTH, { .num = 3 }                   },
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_LEFT } },
};
STYLE_HANDLE(listItemFocused);

static const lv_style_const_prop_t modalProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }            },
    { LV_STYLE_RADIUS,       { .num = 4 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 10 }           },
    { LV_STYLE_PAD_RIGHT,    { .num = 10 }           },
    { LV_STYLE_PAD_TOP,      { .num = 10 }           },
    { LV_STYLE_PAD_BOTTOM,   { .num = 10 }           },
    { LV_STYLE_PAD_ROW,      { .num = 4 }            },
    { LV_STYLE_SHADOW_WIDTH, { .num = 4 }            },
    { LV_STYLE_SHADOW_OPA,   { .num = LV_OPA_50 }    },
};
STYLE_HANDLE(modal);

static const lv_style_const_prop_t scrollbarProps[] = {
    { LV_STYLE_BG_OPA,    { .num = LV_OPA_40 }        },
    { LV_STYLE_RADIUS,    { .num = LV_RADIUS_CIRCLE } },
    { LV_STYLE_WIDTH,     { .num = 3 }                },
    { LV_STYLE_PAD_RIGHT, { .num = 2 }                },
};
STYLE_HANDLE(scrollbar);

static const lv_style_const_prop_t rollerProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }       },
    { LV_STYLE_TEXT_FONT,    { .ptr = &lv_font_rsdeck_14 } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                  },
};
STYLE_HANDLE(roller);

static const lv_style_const_prop_t statusBarProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }          },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                     },
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_BOTTOM } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                     },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                     },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                     },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                     },
    { LV_STYLE_RADIUS,       { .num = 0 }                     },
};
STYLE_HANDLE(statusBar);

static const lv_style_const_prop_t statusToastProps[] = {
    { LV_STYLE_BG_OPA,     { .num = LV_OPA_COVER }       },
    { LV_STYLE_TEXT_FONT,  { .ptr = &lv_font_rsdeck_12 } },
    { LV_STYLE_PAD_LEFT,   { .num = 0 }                  },
    { LV_STYLE_PAD_RIGHT,  { .num = 0 }                  },
    { LV_STYLE_PAD_TOP,    { .num = 0 }                  },
    { LV_STYLE_PAD_BOTTOM, { .num = 0 }                  },
    { LV_STYLE_RADIUS,     { .num = 0 }                  },
};
STYLE_HANDLE(statusToast);

static const lv_style_const_prop_t centeredProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_CENTER } },
    { LV_STYLE_X,     { .num = 0 }               },
    { LV_STYLE_Y,     { .num = 0 }               },
};
STYLE_HANDLE(centered);

static const lv_style_const_prop_t keyboardOverlayProps[] = {
    { LV_STYLE_WIDTH,      { .num = LV_PCT(100) }  },
    { LV_STYLE_HEIGHT,     { .num = LV_PCT(100) }  },
    { LV_STYLE_X,          { .num = 0 }            },
    { LV_STYLE_Y,          { .num = 0 }            },
    { LV_STYLE_PAD_LEFT,   { .num = 8 }            },
    { LV_STYLE_PAD_RIGHT,  { .num = 8 }            },
    { LV_STYLE_PAD_TOP,    { .num = 8 }            },
    { LV_STYLE_PAD_BOTTOM, { .num = 8 }            },
    { LV_STYLE_RADIUS,     { .num = 0 }            },
    { LV_STYLE_BG_OPA,     { .num = LV_OPA_COVER } },
};
STYLE_HANDLE(keyboardOverlay);

static const lv_style_const_prop_t titleInsetProps[] = {
    { LV_STYLE_X, { .num = 4 } },
    { LV_STYLE_Y, { .num = 0 } },
};
STYLE_HANDLE(titleInset);

static const lv_style_const_prop_t keyboardEditorProps[] = {
    { LV_STYLE_X,            { .num = 0 }            },
    { LV_STYLE_Y,            { .num = 28 }           },
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }  },
    { LV_STYLE_HEIGHT,       { .num = 130 }          },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_PAD_LEFT,     { .num = 6 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 6 }            },
    { LV_STYLE_PAD_TOP,      { .num = 6 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 6 }            },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }            },
};
STYLE_HANDLE(keyboardEditor);

static const lv_style_const_prop_t textCursorProps[] = {
    { LV_STYLE_BORDER_WIDTH, { .num = 2 }                   },
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_LEFT } },
};
STYLE_HANDLE(textCursor);

static const lv_style_const_prop_t keyboardHintProps[] = {
    { LV_STYLE_X, { .num = 4 }   },
    { LV_STYLE_Y, { .num = 164 } },
};
STYLE_HANDLE(keyboardHint);

static const lv_style_const_prop_t keyboardRowsProps[] = {
    { LV_STYLE_X,            { .num = 0 }                   },
    { LV_STYLE_Y,            { .num = 190 }                 },
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }         },
    { LV_STYLE_HEIGHT,       { .num = 220 }                 },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                   },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                   },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                   },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                   },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                   },
    { LV_STYLE_LAYOUT,       { .num = FlexLayout }          },
    { FlexFlow,              { .num = LV_FLEX_FLOW_COLUMN } },
    { LV_STYLE_PAD_ROW,      { .num = 6 }                   },
};
STYLE_HANDLE(keyboardRows);

static const lv_style_const_prop_t choiceListProps[] = {
    { LV_STYLE_X,      { .num = 0 }                   },
    { LV_STYLE_Y,      { .num = 30 }                  },
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) }         },
    { LV_STYLE_HEIGHT, { .num = 370 }                 },
    { FlexFlow,        { .num = LV_FLEX_FLOW_COLUMN } },
};
STYLE_HANDLE(choiceList);

static const lv_style_const_prop_t choiceListItemProps[] = {
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) }  },
    { LV_STYLE_HEIGHT, { .num = 48 }           },
    { LV_STYLE_BG_OPA, { .num = LV_OPA_COVER } },
};
STYLE_HANDLE(choiceListItem);

static const lv_style_const_prop_t pointerCursorProps[] = {
    { LV_STYLE_WIDTH,         { .num = 14 }           },
    { LV_STYLE_HEIGHT,        { .num = 14 }           },
    { LV_STYLE_RADIUS,        { .num = 7 }            },
    { LV_STYLE_BG_OPA,        { .num = LV_OPA_60 }    },
    { LV_STYLE_OUTLINE_WIDTH, { .num = 1 }            },
    { LV_STYLE_OUTLINE_OPA,   { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH,  { .num = 0 }            },
    { LV_STYLE_TRANSLATE_X,   { .num = -7 }           },
    { LV_STYLE_TRANSLATE_Y,   { .num = -7 }           },
};
STYLE_HANDLE(pointerCursor);

static const lv_style_const_prop_t fontCaptionProps[] = {
    { LV_STYLE_TEXT_FONT, { .ptr = &lv_font_rsdeck_10 } },
};
STYLE_HANDLE(fontCaption);

static const lv_style_const_prop_t fontSymbolProps[] = {
    { LV_STYLE_TEXT_FONT, { .ptr = &lv_font_montserrat_12 } },
};
STYLE_HANDLE(fontSymbol);

static const lv_style_const_prop_t statusBarPositionProps[] = {
    { LV_STYLE_WIDTH,  { .num = 800 }               },
    { LV_STYLE_HEIGHT, { .num = 28 }                },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_TOP_LEFT } },
    { LV_STYLE_X,      { .num = 0 }                 },
    { LV_STYLE_Y,      { .num = 0 }                 },
};
STYLE_HANDLE(statusBarPosition);

static const lv_style_const_prop_t clockPositionProps[] = {
    { LV_STYLE_WIDTH,      { .num = 82 }                 },
    { LV_STYLE_HEIGHT,     { .num = 28 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_LEFT_MID }  },
    { LV_STYLE_X,          { .num = 4 }                  },
    { LV_STYLE_Y,          { .num = 0 }                  },
};
STYLE_HANDLE(clockPosition);

static const lv_style_const_prop_t transportPositionProps[] = {
    { LV_STYLE_WIDTH,      { .num = 614 }                  },
    { LV_STYLE_HEIGHT,     { .num = 28 }                   },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_LEFT_MID }    },
    { LV_STYLE_X,          { .num = 90 }                   },
    { LV_STYLE_Y,          { .num = 0 }                    },
};
STYLE_HANDLE(transportPosition);

static const lv_style_const_prop_t batteryTextPositionProps[] = {
    { LV_STYLE_WIDTH,      { .num = 64 }                  },
    { LV_STYLE_HEIGHT,     { .num = 28 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_RIGHT } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_RIGHT_MID }  },
    { LV_STYLE_X,          { .num = -4 }                  },
    { LV_STYLE_Y,          { .num = 0 }                   },
};
STYLE_HANDLE(batteryTextPosition);

static const lv_style_const_prop_t batteryGaugePositionProps[] = {
    { LV_STYLE_WIDTH,  { .num = 20 }                 },
    { LV_STYLE_HEIGHT, { .num = 8 }                  },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,      { .num = -4 }                 },
    { LV_STYLE_Y,      { .num = 0 }                  },
};
STYLE_HANDLE(batteryGaugePosition);

static const lv_style_const_prop_t opaqueProps[] = {
    { LV_STYLE_BG_OPA, { .num = LV_OPA_COVER } },
};
STYLE_HANDLE(opaque);

static const lv_style_const_prop_t wifiPositionProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,     { .num = -72 }                },
    { LV_STYLE_Y,     { .num = 0 }                  },
};
STYLE_HANDLE(wifiPosition);

static const lv_style_const_prop_t disabledSlashProps[] = {
    { LV_STYLE_LINE_WIDTH,   { .num = 2 }               },
    { LV_STYLE_LINE_ROUNDED, { .num = true }            },
    { LV_STYLE_ALIGN,        { .num = LV_ALIGN_CENTER } },
    { LV_STYLE_X,            { .num = 0 }               },
    { LV_STYLE_Y,            { .num = 0 }               },
};
STYLE_HANDLE(disabledSlash);

static const lv_style_const_prop_t toastCaptionProps[] = {
    { LV_STYLE_WIDTH,      { .num = 788 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_CENTER }      },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 0 }                    },
};
STYLE_HANDLE(toastCaption);

static const lv_style_const_prop_t avatarFrameProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }            },
    { LV_STYLE_CLIP_CORNER,  { .num = false }        },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_SHADOW_WIDTH, { .num = 0 }            },
};
STYLE_HANDLE(avatarFrame);

static const lv_style_const_prop_t fullscreenProps[] = {
    { LV_STYLE_WIDTH,  { .num = 800 }             },
    { LV_STYLE_HEIGHT, { .num = 480 }             },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_CENTER } },
    { LV_STYLE_X,      { .num = 0 }               },
    { LV_STYLE_Y,      { .num = 0 }               },
};
STYLE_HANDLE(fullscreen);

static const lv_style_const_prop_t modalPaddingProps[] = {
    { LV_STYLE_PAD_LEFT,   { .num = 10 } },
    { LV_STYLE_PAD_RIGHT,  { .num = 10 } },
    { LV_STYLE_PAD_TOP,    { .num = 10 } },
    { LV_STYLE_PAD_BOTTOM, { .num = 10 } },
};
STYLE_HANDLE(modalPadding);

static const lv_style_const_prop_t voiceTitleProps[] = {
    { LV_STYLE_WIDTH,  { .num = 700 } },
    { LV_STYLE_HEIGHT, { .num = 28 }  },
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 0 }   },
};
STYLE_HANDLE(voiceTitle);

static const lv_style_const_prop_t voicePeerProps[] = {
    { LV_STYLE_WIDTH,  { .num = 776 } },
    { LV_STYLE_HEIGHT, { .num = 28 }  },
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 24 }  },
};
STYLE_HANDLE(voicePeer);

static const lv_style_const_prop_t voiceStatusProps[] = {
    { LV_STYLE_WIDTH,  { .num = 776 } },
    { LV_STYLE_HEIGHT, { .num = 28 }  },
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 48 }  },
};
STYLE_HANDLE(voiceStatus);

static const lv_style_const_prop_t voiceDetailProps[] = {
    { LV_STYLE_WIDTH,  { .num = 776 } },
    { LV_STYLE_HEIGHT, { .num = 28 }  },
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 71 }  },
};
STYLE_HANDLE(voiceDetail);

static const lv_style_const_prop_t voiceHintProps[] = {
    { LV_STYLE_WIDTH,  { .num = 776 } },
    { LV_STYLE_HEIGHT, { .num = 28 }  },
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 444 } },
};
STYLE_HANDLE(voiceHint);

static const lv_style_const_prop_t voiceBackProps[] = {
    { LV_STYLE_X,      { .num = 716 } },
    { LV_STYLE_Y,      { .num = -3 }  },
    { LV_STYLE_WIDTH,  { .num = 60 }  },
    { LV_STYLE_HEIGHT, { .num = 26 }  },
};
STYLE_HANDLE(voiceBack);

static const lv_style_const_prop_t volumeDecreaseProps[] = {
    { LV_STYLE_X,      { .num = 708 } },
    { LV_STYLE_Y,      { .num = 64 }  },
    { LV_STYLE_WIDTH,  { .num = 30 }  },
    { LV_STYLE_HEIGHT, { .num = 24 }  },
};
STYLE_HANDLE(volumeDecrease);

static const lv_style_const_prop_t volumeIncreaseProps[] = {
    { LV_STYLE_X,      { .num = 746 } },
    { LV_STYLE_Y,      { .num = 64 }  },
    { LV_STYLE_WIDTH,  { .num = 30 }  },
    { LV_STYLE_HEIGHT, { .num = 24 }  },
};
STYLE_HANDLE(volumeIncrease);

static const lv_style_const_prop_t voiceDetailWidthProps[] = {
    { LV_STYLE_WIDTH, { .num = 700 } },
};
STYLE_HANDLE(voiceDetailWidth);

static const lv_style_const_prop_t fontTabProps[] = {
    { LV_STYLE_TEXT_FONT, { .ptr = &lv_font_crowpanel_14 } },
};
STYLE_HANDLE(fontTab);

static const lv_style_const_prop_t tabViewportProps[] = {
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 28 }  },
    { LV_STYLE_WIDTH,  { .num = 800 } },
    { LV_STYLE_HEIGHT, { .num = 452 } },
};
STYLE_HANDLE(tabViewport);

static const lv_style_const_prop_t seamlessContentProps[] = {
    { LV_STYLE_PAD_LEFT,     { .num = 0 } },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 } },
    { LV_STYLE_PAD_TOP,      { .num = 0 } },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 } },
    { LV_STYLE_PAD_ROW,      { .num = 0 } },
    { LV_STYLE_PAD_COLUMN,   { .num = 0 } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 } },
};
STYLE_HANDLE(seamlessContent);

static const lv_style_const_prop_t flatContentProps[] = {
    { LV_STYLE_PAD_LEFT,     { .num = 0 } },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 } },
    { LV_STYLE_PAD_TOP,      { .num = 0 } },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 } },
    { LV_STYLE_RADIUS,       { .num = 0 } },
};
STYLE_HANDLE(flatContent);

static const lv_style_const_prop_t opaqueContentProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
};
STYLE_HANDLE(opaqueContent);

static const lv_style_const_prop_t tabStripProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_PAD_ROW,      { .num = 0 }            },
    { LV_STYLE_PAD_COLUMN,   { .num = 0 }            },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_RADIUS,       { .num = 0 }            },
};
STYLE_HANDLE(tabStrip);

static const lv_style_const_prop_t tabItemProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_RADIUS,       { .num = 0 }            },
};
STYLE_HANDLE(tabItem);

static const lv_style_const_prop_t noticeDialogProps[] = {
    { LV_STYLE_WIDTH,  { .num = 768 }             },
    { LV_STYLE_HEIGHT, { .num = LV_SIZE_CONTENT } },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_CENTER } },
    { LV_STYLE_X,      { .num = 0 }               },
    { LV_STYLE_Y,      { .num = 0 }               },
};
STYLE_HANDLE(noticeDialog);

static const lv_style_const_prop_t noticeColumnProps[] = {
    { LV_STYLE_PAD_LEFT,   { .num = 10 }                  },
    { LV_STYLE_PAD_RIGHT,  { .num = 10 }                  },
    { LV_STYLE_PAD_TOP,    { .num = 10 }                  },
    { LV_STYLE_PAD_BOTTOM, { .num = 10 }                  },
    { LV_STYLE_PAD_ROW,    { .num = 6 }                   },
    { LV_STYLE_LAYOUT,     { .num = FlexLayout }          },
    { FlexFlow,            { .num = LV_FLEX_FLOW_COLUMN } },
};
STYLE_HANDLE(noticeColumn);

static const lv_style_const_prop_t fullWidthProps[] = {
    { LV_STYLE_WIDTH, { .num = LV_PCT(100) } },
};
STYLE_HANDLE(fullWidth);

static const lv_style_const_prop_t centeredTextProps[] = {
    { LV_STYLE_WIDTH,      { .num = LV_PCT(100) }          },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
};
STYLE_HANDLE(centeredText);

static const lv_style_const_prop_t fontHeadingProps[] = {
    { LV_STYLE_TEXT_FONT, { .ptr = &lv_font_montserrat_16 } },
};
STYLE_HANDLE(fontHeading);

static const lv_style_const_prop_t bootTitleProps[] = {
    { LV_STYLE_WIDTH,      { .num = 240 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_CENTER }      },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = -26 }                  },
};
STYLE_HANDLE(bootTitle);

static const lv_style_const_prop_t bootProgressProps[] = {
    { LV_STYLE_WIDTH,  { .num = 188 }             },
    { LV_STYLE_HEIGHT, { .num = 8 }               },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_CENTER } },
    { LV_STYLE_X,      { .num = 0 }               },
    { LV_STYLE_Y,      { .num = 14 }              },
};
STYLE_HANDLE(bootProgress);

static const lv_style_const_prop_t errorHeadingProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_TOP_MID } },
    { LV_STYLE_X,     { .num = 0 }                },
    { LV_STYLE_Y,     { .num = 12 }               },
};
STYLE_HANDLE(errorHeading);

static const lv_style_const_prop_t errorDescriptionProps[] = {
    { LV_STYLE_WIDTH,      { .num = 768 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 48 }                   },
};
STYLE_HANDLE(errorDescription);

static const lv_style_const_prop_t footerHintProps[] = {
    { LV_STYLE_WIDTH,      { .num = 768 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_BOTTOM_MID }  },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = -12 }                  },
};
STYLE_HANDLE(footerHint);

static const lv_style_const_prop_t noPaddingProps[] = {
    { LV_STYLE_PAD_LEFT,   { .num = 0 } },
    { LV_STYLE_PAD_RIGHT,  { .num = 0 } },
    { LV_STYLE_PAD_TOP,    { .num = 0 } },
    { LV_STYLE_PAD_BOTTOM, { .num = 0 } },
};
STYLE_HANDLE(noPadding);

static const lv_style_const_prop_t emptyContactProps[] = {
    { LV_STYLE_WIDTH,        { .num = 252 }             },
    { LV_STYLE_HEIGHT,       { .num = 72 }              },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }   },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }               },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }               },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }               },
    { LV_STYLE_PAD_TOP,      { .num = 0 }               },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }               },
    { LV_STYLE_ALIGN,        { .num = LV_ALIGN_CENTER } },
    { LV_STYLE_X,            { .num = 0 }               },
    { LV_STYLE_Y,            { .num = 0 }               },
};
STYLE_HANDLE(emptyContact);

static const lv_style_const_prop_t contactHeadProps[] = {
    { LV_STYLE_WIDTH,        { .num = 18 }            },
    { LV_STYLE_HEIGHT,       { .num = 18 }            },
    { LV_STYLE_X,            { .num = 117 }           },
    { LV_STYLE_Y,            { .num = 7 }             },
    { LV_STYLE_RADIUS,       { .num = 9 }             },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP } },
    { LV_STYLE_BORDER_WIDTH, { .num = 2 }             },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }             },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }             },
    { LV_STYLE_PAD_TOP,      { .num = 0 }             },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }             },
};
STYLE_HANDLE(contactHead);

static const lv_style_const_prop_t contactShouldersProps[] = {
    { LV_STYLE_WIDTH,        { .num = 36 }            },
    { LV_STYLE_HEIGHT,       { .num = 17 }            },
    { LV_STYLE_X,            { .num = 108 }           },
    { LV_STYLE_Y,            { .num = 27 }            },
    { LV_STYLE_RADIUS,       { .num = 8 }             },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP } },
    { LV_STYLE_BORDER_WIDTH, { .num = 2 }             },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }             },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }             },
    { LV_STYLE_PAD_TOP,      { .num = 0 }             },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }             },
};
STYLE_HANDLE(contactShoulders);

static const lv_style_const_prop_t emptyCaptionProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_TOP_MID } },
    { LV_STYLE_X,     { .num = 0 }                },
    { LV_STYLE_Y,     { .num = 49 }               },
};
STYLE_HANDLE(emptyCaption);

static const lv_style_const_prop_t fillParentProps[] = {
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = LV_PCT(100) } },
};
STYLE_HANDLE(fillParent);

static const lv_style_const_prop_t columnProps[] = {
    { LV_STYLE_LAYOUT, { .num = FlexLayout }          },
    { FlexFlow,        { .num = LV_FLEX_FLOW_COLUMN } },
};
STYLE_HANDLE(column);

static const lv_style_const_prop_t actionListRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = 800 } },
    { LV_STYLE_HEIGHT, { .num = 28 }  },
};
STYLE_HANDLE(actionListRow);

static const lv_style_const_prop_t actionRowBorderProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }          },
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_BOTTOM } },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                     },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                     },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                     },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                     },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                     },
};
STYLE_HANDLE(actionRowBorder);

static const lv_style_const_prop_t leadingCaptionProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_LEFT_MID } },
    { LV_STYLE_X,     { .num = 12 }                },
    { LV_STYLE_Y,     { .num = 0 }                 },
};
STYLE_HANDLE(leadingCaption);

static const lv_style_const_prop_t trailingHintProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,     { .num = -12 }                },
    { LV_STYLE_Y,     { .num = 0 }                  },
};
STYLE_HANDLE(trailingHint);

static const lv_style_const_prop_t contactRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = 800 } },
    { LV_STYLE_HEIGHT, { .num = 38 }  },
};
STYLE_HANDLE(contactRow);

static const lv_style_const_prop_t listRowBorderProps[] = {
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_BOTTOM } },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                     },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                     },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                     },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                     },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                     },
};
STYLE_HANDLE(listRowBorder);

static const lv_style_const_prop_t contactNameProps[] = {
    { LV_STYLE_WIDTH, { .num = 680 } },
    { LV_STYLE_X,     { .num = 48 }  },
    { LV_STYLE_Y,     { .num = 2 }   },
};
STYLE_HANDLE(contactName);

static const lv_style_const_prop_t contactAgeProps[] = {
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_RIGHT } },
    { LV_STYLE_WIDTH,      { .num = 64 }                  },
    { LV_STYLE_X,          { .num = 728 }                 },
    { LV_STYLE_Y,          { .num = 5 }                   },
};
STYLE_HANDLE(contactAge);

static const lv_style_const_prop_t contactAddressProps[] = {
    { LV_STYLE_WIDTH, { .num = 744 } },
    { LV_STYLE_X,     { .num = 48 }  },
    { LV_STYLE_Y,     { .num = 22 }  },
};
STYLE_HANDLE(contactAddress);

static const lv_style_const_prop_t helpDialogProps[] = {
    { LV_STYLE_WIDTH,  { .num = 304 }             },
    { LV_STYLE_HEIGHT, { .num = 190 }             },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_CENTER } },
    { LV_STYLE_X,      { .num = 0 }               },
    { LV_STYLE_Y,      { .num = 0 }               },
};
STYLE_HANDLE(helpDialog);

static const lv_style_const_prop_t helpColumnProps[] = {
    { LV_STYLE_PAD_LEFT,   { .num = 8 }                   },
    { LV_STYLE_PAD_RIGHT,  { .num = 8 }                   },
    { LV_STYLE_PAD_TOP,    { .num = 8 }                   },
    { LV_STYLE_PAD_BOTTOM, { .num = 8 }                   },
    { LV_STYLE_PAD_ROW,    { .num = 1 }                   },
    { LV_STYLE_LAYOUT,     { .num = FlexLayout }          },
    { FlexFlow,            { .num = LV_FLEX_FLOW_COLUMN } },
};
STYLE_HANDLE(helpColumn);

static const lv_style_const_prop_t helpHeaderProps[] = {
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }      },
    { LV_STYLE_HEIGHT,       { .num = 16 }               },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }    },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                },
    { LV_STYLE_PAD_COLUMN,   { .num = 6 }                },
    { LV_STYLE_LAYOUT,       { .num = FlexLayout }       },
    { FlexFlow,              { .num = LV_FLEX_FLOW_ROW } },
};
STYLE_HANDLE(helpHeader);

static const lv_style_const_prop_t helpTitleProps[] = {
    { LV_STYLE_WIDTH,      { .num = 70 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
};
STYLE_HANDLE(helpTitle);

static const lv_style_const_prop_t helpBrandProps[] = {
    { LV_STYLE_WIDTH,      { .num = 180 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_RIGHT } },
};
STYLE_HANDLE(helpBrand);

static const lv_style_const_prop_t helpRowProps[] = {
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }      },
    { LV_STYLE_HEIGHT,       { .num = 13 }               },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }    },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                },
    { LV_STYLE_PAD_COLUMN,   { .num = 6 }                },
    { LV_STYLE_LAYOUT,       { .num = FlexLayout }       },
    { FlexFlow,              { .num = LV_FLEX_FLOW_ROW } },
};
STYLE_HANDLE(helpRow);

static const lv_style_const_prop_t helpKeyProps[] = {
    { LV_STYLE_WIDTH,      { .num = 72 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
};
STYLE_HANDLE(helpKey);

static const lv_style_const_prop_t helpActionProps[] = {
    { LV_STYLE_WIDTH,      { .num = 178 }                },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
};
STYLE_HANDLE(helpAction);

static const lv_style_const_prop_t helpFooterProps[] = {
    { LV_STYLE_WIDTH,      { .num = 260 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_PAD_TOP,    { .num = 2 }                    },
};
STYLE_HANDLE(helpFooter);

static const lv_style_const_prop_t setupTitleProps[] = {
    { LV_STYLE_WIDTH,      { .num = 260 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 24 }                   },
};
STYLE_HANDLE(setupTitle);

static const lv_style_const_prop_t setupDividerProps[] = {
    { LV_STYLE_X,            { .num = 24 }           },
    { LV_STYLE_Y,            { .num = 52 }           },
    { LV_STYLE_WIDTH,        { .num = 752 }          },
    { LV_STYLE_HEIGHT,       { .num = 1 }            },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
};
STYLE_HANDLE(setupDivider);

static const lv_style_const_prop_t setupPromptProps[] = {
    { LV_STYLE_WIDTH,      { .num = 250 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 64 }                   },
};
STYLE_HANDLE(setupPrompt);

static const lv_style_const_prop_t singleLineInputProps[] = {
    { LV_STYLE_WIDTH,  { .num = 252 }              },
    { LV_STYLE_HEIGHT, { .num = 36 }               },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_TOP_MID } },
    { LV_STYLE_X,      { .num = 0 }                },
    { LV_STYLE_Y,      { .num = 102 }              },
};
STYLE_HANDLE(singleLineInput);

static const lv_style_const_prop_t setupAcceptProps[] = {
    { LV_STYLE_WIDTH,  { .num = 168 }              },
    { LV_STYLE_HEIGHT, { .num = 34 }               },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_TOP_MID } },
    { LV_STYLE_X,      { .num = 0 }                },
    { LV_STYLE_Y,      { .num = 158 }              },
};
STYLE_HANDLE(setupAccept);

static const lv_style_const_prop_t flatActionProps[] = {
    { LV_STYLE_SHADOW_WIDTH, { .num = 0 } },
    { LV_STYLE_RADIUS,       { .num = 4 } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 } },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 } },
    { LV_STYLE_PAD_TOP,      { .num = 0 } },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 } },
};
STYLE_HANDLE(flatAction);

static const lv_style_const_prop_t buttonCaptionProps[] = {
    { LV_STYLE_WIDTH,      { .num = LV_PCT(100) }          },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_CENTER }      },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 0 }                    },
};
STYLE_HANDLE(buttonCaption);

static const lv_style_const_prop_t versionFooterProps[] = {
    { LV_STYLE_WIDTH,      { .num = 120 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_BOTTOM_MID }  },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = -10 }                  },
};
STYLE_HANDLE(versionFooter);

static const lv_style_const_prop_t setupEyebrowProps[] = {
    { LV_STYLE_WIDTH,      { .num = 220 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 10 }                   },
};
STYLE_HANDLE(setupEyebrow);

static const lv_style_const_prop_t storageTitleProps[] = {
    { LV_STYLE_WIDTH,      { .num = 260 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 28 }                   },
};
STYLE_HANDLE(storageTitle);

static const lv_style_const_prop_t storageDescriptionProps[] = {
    { LV_STYLE_WIDTH,      { .num = 268 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 58 }                   },
};
STYLE_HANDLE(storageDescription);

static const lv_style_const_prop_t storagePromptProps[] = {
    { LV_STYLE_WIDTH,      { .num = 276 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 82 }                   },
};
STYLE_HANDLE(storagePrompt);

static const lv_style_const_prop_t choiceLeftProps[] = {
    { LV_STYLE_X, { .num = 28 }  },
    { LV_STYLE_Y, { .num = 122 } },
};
STYLE_HANDLE(choiceLeft);

static const lv_style_const_prop_t choiceRightProps[] = {
    { LV_STYLE_X, { .num = 166 } },
    { LV_STYLE_Y, { .num = 122 } },
};
STYLE_HANDLE(choiceRight);

static const lv_style_const_prop_t confirmationHintProps[] = {
    { LV_STYLE_WIDTH,      { .num = 280 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 164 }                  },
};
STYLE_HANDLE(confirmationHint);

static const lv_style_const_prop_t inputHintProps[] = {
    { LV_STYLE_WIDTH,      { .num = 286 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 188 }                  },
};
STYLE_HANDLE(inputHint);

static const lv_style_const_prop_t operationStatusProps[] = {
    { LV_STYLE_WIDTH,      { .num = 286 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 140 }                  },
};
STYLE_HANDLE(operationStatus);

static const lv_style_const_prop_t versionTextProps[] = {
    { LV_STYLE_WIDTH,      { .num = 120 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
};
STYLE_HANDLE(versionText);

static const lv_style_const_prop_t absoluteContentProps[] = {
    { LV_STYLE_LAYOUT,       { .num = 0 }            },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_RADIUS,       { .num = 0 }            },
};
STYLE_HANDLE(absoluteContent);

static const lv_style_const_prop_t identityCardProps[] = {
    { LV_STYLE_X,            { .num = 8 }            },
    { LV_STYLE_Y,            { .num = 8 }            },
    { LV_STYLE_WIDTH,        { .num = 784 }          },
    { LV_STYLE_HEIGHT,       { .num = 96 }           },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }            },
    { LV_STYLE_RADIUS,       { .num = 4 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_SHADOW_WIDTH, { .num = 0 }            },
};
STYLE_HANDLE(identityCard);

static const lv_style_const_prop_t identityAccentProps[] = {
    { LV_STYLE_X,            { .num = 0 }            },
    { LV_STYLE_Y,            { .num = 0 }            },
    { LV_STYLE_WIDTH,        { .num = 3 }            },
    { LV_STYLE_HEIGHT,       { .num = 96 }           },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_RADIUS,       { .num = 0 }            },
};
STYLE_HANDLE(identityAccent);

static const lv_style_const_prop_t identityHeadingProps[] = {
    { LV_STYLE_X,                 { .num = 92 }                 },
    { LV_STYLE_Y,                 { .num = 8 }                  },
    { LV_STYLE_WIDTH,             { .num = 116 }                },
    { LV_STYLE_HEIGHT,            { .num = 19 }                 },
    { LV_STYLE_TEXT_ALIGN,        { .num = LV_TEXT_ALIGN_LEFT } },
    { LV_STYLE_TEXT_LETTER_SPACE, { .num = 1 }                  },
};
STYLE_HANDLE(identityHeading);

static const lv_style_const_prop_t connectivityCaptionProps[] = {
    { LV_STYLE_X,                 { .num = 614 }                 },
    { LV_STYLE_Y,                 { .num = 8 }                   },
    { LV_STYLE_WIDTH,             { .num = 160 }                 },
    { LV_STYLE_HEIGHT,            { .num = 19 }                  },
    { LV_STYLE_TEXT_ALIGN,        { .num = LV_TEXT_ALIGN_RIGHT } },
    { LV_STYLE_TEXT_LETTER_SPACE, { .num = 1 }                   },
};
STYLE_HANDLE(connectivityCaption);

static const lv_style_const_prop_t identityNameProps[] = {
    { LV_STYLE_X,          { .num = 92 }                 },
    { LV_STYLE_Y,          { .num = 32 }                 },
    { LV_STYLE_WIDTH,      { .num = 682 }                },
    { LV_STYLE_HEIGHT,     { .num = 28 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
};
STYLE_HANDLE(identityName);

static const lv_style_const_prop_t identityDestinationProps[] = {
    { LV_STYLE_X,          { .num = 92 }                 },
    { LV_STYLE_Y,          { .num = 66 }                 },
    { LV_STYLE_WIDTH,      { .num = 490 }                },
    { LV_STYLE_HEIGHT,     { .num = 19 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
};
STYLE_HANDLE(identityDestination);

static const lv_style_const_prop_t identityHashProps[] = {
    { LV_STYLE_X,          { .num = 594 }                 },
    { LV_STYLE_Y,          { .num = 66 }                  },
    { LV_STYLE_WIDTH,      { .num = 180 }                 },
    { LV_STYLE_HEIGHT,     { .num = 19 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_RIGHT } },
};
STYLE_HANDLE(identityHash);

static const lv_style_const_prop_t transportTileLeftProps[] = {
    { LV_STYLE_X, { .num = 8 }   },
    { LV_STYLE_Y, { .num = 116 } },
};
STYLE_HANDLE(transportTileLeft);

static const lv_style_const_prop_t tileCaptionProps[] = {
    { LV_STYLE_X,                 { .num = 7 }                  },
    { LV_STYLE_Y,                 { .num = 4 }                  },
    { LV_STYLE_WIDTH,             { .num = 239 }                },
    { LV_STYLE_HEIGHT,            { .num = 19 }                 },
    { LV_STYLE_TEXT_ALIGN,        { .num = LV_TEXT_ALIGN_LEFT } },
    { LV_STYLE_TEXT_LETTER_SPACE, { .num = 1 }                  },
};
STYLE_HANDLE(tileCaption);

static const lv_style_const_prop_t tileStateProps[] = {
    { LV_STYLE_X,          { .num = 7 }                  },
    { LV_STYLE_Y,          { .num = 32 }                 },
    { LV_STYLE_WIDTH,      { .num = 239 }                },
    { LV_STYLE_HEIGHT,     { .num = 24 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
};
STYLE_HANDLE(tileState);

static const lv_style_const_prop_t transportTileMiddleProps[] = {
    { LV_STYLE_X, { .num = 273 } },
    { LV_STYLE_Y, { .num = 116 } },
};
STYLE_HANDLE(transportTileMiddle);

static const lv_style_const_prop_t transportTileRightProps[] = {
    { LV_STYLE_X, { .num = 538 } },
    { LV_STYLE_Y, { .num = 116 } },
};
STYLE_HANDLE(transportTileRight);

static const lv_style_const_prop_t statusTileLeftProps[] = {
    { LV_STYLE_X, { .num = 8 }   },
    { LV_STYLE_Y, { .num = 200 } },
};
STYLE_HANDLE(statusTileLeft);

static const lv_style_const_prop_t tileValueProps[] = {
    { LV_STYLE_X,          { .num = 7 }                  },
    { LV_STYLE_Y,          { .num = 32 }                 },
    { LV_STYLE_WIDTH,      { .num = 239 }                },
    { LV_STYLE_HEIGHT,     { .num = 28 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
};
STYLE_HANDLE(tileValue);

static const lv_style_const_prop_t statusTileMiddleProps[] = {
    { LV_STYLE_X, { .num = 273 } },
    { LV_STYLE_Y, { .num = 200 } },
};
STYLE_HANDLE(statusTileMiddle);

static const lv_style_const_prop_t statusTileRightProps[] = {
    { LV_STYLE_X, { .num = 538 } },
    { LV_STYLE_Y, { .num = 200 } },
};
STYLE_HANDLE(statusTileRight);

static const lv_style_const_prop_t summaryCaptionProps[] = {
    { LV_STYLE_X,          { .num = 7 }                  },
    { LV_STYLE_Y,          { .num = 4 }                  },
    { LV_STYLE_WIDTH,      { .num = 239 }                },
    { LV_STYLE_HEIGHT,     { .num = 19 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
};
STYLE_HANDLE(summaryCaption);

static const lv_style_const_prop_t announceAgeProps[] = {
    { LV_STYLE_TEXT_LETTER_SPACE, { .num = 1 } },
};
STYLE_HANDLE(announceAge);

static const lv_style_const_prop_t tileStatePositionProps[] = {
    { LV_STYLE_X, { .num = 7 }  },
    { LV_STYLE_Y, { .num = 32 } },
};
STYLE_HANDLE(tileStatePosition);

static const lv_style_const_prop_t summaryFooterProps[] = {
    { LV_STYLE_X,            { .num = 8 }            },
    { LV_STYLE_Y,            { .num = 284 }          },
    { LV_STYLE_WIDTH,        { .num = 592 }          },
    { LV_STYLE_HEIGHT,       { .num = 72 }           },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }            },
    { LV_STYLE_RADIUS,       { .num = 4 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_SHADOW_WIDTH, { .num = 0 }            },
};
STYLE_HANDLE(summaryFooter);

static const lv_style_const_prop_t footerSummaryProps[] = {
    { LV_STYLE_X,          { .num = 8 }                  },
    { LV_STYLE_Y,          { .num = 10 }                 },
    { LV_STYLE_WIDTH,      { .num = 576 }                },
    { LV_STYLE_HEIGHT,     { .num = 24 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
};
STYLE_HANDLE(footerSummary);

static const lv_style_const_prop_t footerAgeProps[] = {
    { LV_STYLE_X,          { .num = 8 }                  },
    { LV_STYLE_Y,          { .num = 42 }                 },
    { LV_STYLE_WIDTH,      { .num = 576 }                },
    { LV_STYLE_HEIGHT,     { .num = 19 }                 },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_LEFT } },
};
STYLE_HANDLE(footerAge);

static const lv_style_const_prop_t announceActionProps[] = {
    { LV_STYLE_X,      { .num = 612 } },
    { LV_STYLE_Y,      { .num = 284 } },
    { LV_STYLE_WIDTH,  { .num = 180 } },
    { LV_STYLE_HEIGHT, { .num = 72 }  },
};
STYLE_HANDLE(announceAction);

static const lv_style_const_prop_t absoluteLayoutProps[] = {
    { LV_STYLE_LAYOUT,     { .num = 0 } },
    { LV_STYLE_PAD_LEFT,   { .num = 0 } },
    { LV_STYLE_PAD_RIGHT,  { .num = 0 } },
    { LV_STYLE_PAD_TOP,    { .num = 0 } },
    { LV_STYLE_PAD_BOTTOM, { .num = 0 } },
};
STYLE_HANDLE(absoluteLayout);

static const lv_style_const_prop_t familyCaptionProps[] = {
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 0 }                    },
    { LV_STYLE_WIDTH,      { .num = 400 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_CENTER }      },
};
STYLE_HANDLE(familyCaption);

static const lv_style_const_prop_t emptyDescriptionProps[] = {
    { LV_STYLE_X,          { .num = 12 }                   },
    { LV_STYLE_Y,          { .num = 55 }                   },
    { LV_STYLE_WIDTH,      { .num = 776 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
};
STYLE_HANDLE(emptyDescription);

static const lv_style_const_prop_t emptyStateProps[] = {
    { LV_STYLE_WIDTH,        { .num = 252 }             },
    { LV_STYLE_HEIGHT,       { .num = 94 }              },
    { LV_STYLE_ALIGN,        { .num = LV_ALIGN_CENTER } },
    { LV_STYLE_X,            { .num = 0 }               },
    { LV_STYLE_Y,            { .num = 0 }               },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }   },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }               },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }               },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }               },
    { LV_STYLE_PAD_TOP,      { .num = 0 }               },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }               },
};
STYLE_HANDLE(emptyState);

static const lv_style_const_prop_t conversationIconProps[] = {
    { LV_STYLE_X,            { .num = 108 }           },
    { LV_STYLE_Y,            { .num = 7 }             },
    { LV_STYLE_WIDTH,        { .num = 36 }            },
    { LV_STYLE_HEIGHT,       { .num = 28 }            },
    { LV_STYLE_RADIUS,       { .num = 6 }             },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP } },
    { LV_STYLE_BORDER_WIDTH, { .num = 2 }             },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }             },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }             },
    { LV_STYLE_PAD_TOP,      { .num = 0 }             },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }             },
};
STYLE_HANDLE(conversationIcon);

static const lv_style_const_prop_t conversationIconTailProps[] = {
    { LV_STYLE_X,          { .num = 116 } },
    { LV_STYLE_Y,          { .num = 34 }  },
    { LV_STYLE_LINE_WIDTH, { .num = 2 }   },
};
STYLE_HANDLE(conversationIconTail);

static const lv_style_const_prop_t conversationIconDotProps[] = {
    { LV_STYLE_WIDTH,        { .num = 3 }                },
    { LV_STYLE_HEIGHT,       { .num = 3 }                },
    { LV_STYLE_RADIUS,       { .num = LV_RADIUS_CIRCLE } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }     },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                },
};
STYLE_HANDLE(conversationIconDot);

static const lv_style_const_prop_t emptyTitleProps[] = {
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 49 }                   },
    { LV_STYLE_WIDTH,      { .num = 252 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
};
STYLE_HANDLE(emptyTitle);

static const lv_style_const_prop_t emptyHintProps[] = {
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 70 }                   },
    { LV_STYLE_WIDTH,      { .num = 252 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
};
STYLE_HANDLE(emptyHint);

static const lv_style_const_prop_t recoveryActionProps[] = {
    { LV_STYLE_WIDTH,  { .num = 112 } },
    { LV_STYLE_HEIGHT, { .num = 30 }  },
};
STYLE_HANDLE(recoveryAction);

static const lv_style_const_prop_t conversationRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = 800 } },
    { LV_STYLE_HEIGHT, { .num = 88 }  },
};
STYLE_HANDLE(conversationRow);

static const lv_style_const_prop_t conversationAccentProps[] = {
    { LV_STYLE_X,            { .num = 0 }            },
    { LV_STYLE_Y,            { .num = 0 }            },
    { LV_STYLE_WIDTH,        { .num = 4 }            },
    { LV_STYLE_HEIGHT,       { .num = 88 }           },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_RADIUS,       { .num = 0 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
};
STYLE_HANDLE(conversationAccent);

static const lv_style_const_prop_t conversationNameProps[] = {
    { LV_STYLE_X,      { .num = 54 }  },
    { LV_STYLE_Y,      { .num = 2 }   },
    { LV_STYLE_WIDTH,  { .num = 655 } },
    { LV_STYLE_HEIGHT, { .num = 28 }  },
};
STYLE_HANDLE(conversationName);

static const lv_style_const_prop_t conversationTimeProps[] = {
    { LV_STYLE_X,     { .num = 720 } },
    { LV_STYLE_Y,     { .num = 6 }   },
    { LV_STYLE_WIDTH, { .num = 76 }  },
};
STYLE_HANDLE(conversationTime);

static const lv_style_const_prop_t conversationPreviewProps[] = {
    { LV_STYLE_X,      { .num = 54 }  },
    { LV_STYLE_Y,      { .num = 33 }  },
    { LV_STYLE_WIDTH,  { .num = 738 } },
    { LV_STYLE_HEIGHT, { .num = 24 }  },
};
STYLE_HANDLE(conversationPreview);

static const lv_style_const_prop_t truncatedPreviewProps[] = {
    { LV_STYLE_WIDTH, { .num = 723 } },
};
STYLE_HANDLE(truncatedPreview);

static const lv_style_const_prop_t previewContinuationProps[] = {
    { LV_STYLE_X,     { .num = 775 } },
    { LV_STYLE_Y,     { .num = 33 }  },
    { LV_STYLE_WIDTH, { .num = 21 }  },
};
STYLE_HANDLE(previewContinuation);

static const lv_style_const_prop_t conversationStatusProps[] = {
    { LV_STYLE_X,      { .num = 54 }  },
    { LV_STYLE_Y,      { .num = 64 }  },
    { LV_STYLE_WIDTH,  { .num = 738 } },
    { LV_STYLE_HEIGHT, { .num = 19 }  },
};
STYLE_HANDLE(conversationStatus);

static const lv_style_const_prop_t compactModalPaddingProps[] = {
    { LV_STYLE_PAD_LEFT,   { .num = 6 } },
    { LV_STYLE_PAD_RIGHT,  { .num = 6 } },
    { LV_STYLE_PAD_TOP,    { .num = 6 } },
    { LV_STYLE_PAD_BOTTOM, { .num = 6 } },
};
STYLE_HANDLE(compactModalPadding);

static const lv_style_const_prop_t actionMenuTitleProps[] = {
    { LV_STYLE_X,     { .num = 3 }   },
    { LV_STYLE_Y,     { .num = 0 }   },
    { LV_STYLE_WIDTH, { .num = 220 } },
};
STYLE_HANDLE(actionMenuTitle);

static const lv_style_const_prop_t menuRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = 220 } },
    { LV_STYLE_HEIGHT, { .num = 24 }  },
};
STYLE_HANDLE(menuRow);

static const lv_style_const_prop_t regionEyebrowProps[] = {
    { LV_STYLE_WIDTH,      { .num = 220 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 8 }                    },
};
STYLE_HANDLE(regionEyebrow);

static const lv_style_const_prop_t regionDescriptionProps[] = {
    { LV_STYLE_WIDTH,      { .num = 280 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_TOP_MID }     },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 48 }                   },
};
STYLE_HANDLE(regionDescription);

static const lv_style_const_prop_t selectionCardProps[] = {
    { LV_STYLE_WIDTH,        { .num = 288 }              },
    { LV_STYLE_HEIGHT,       { .num = 104 }              },
    { LV_STYLE_ALIGN,        { .num = LV_ALIGN_TOP_MID } },
    { LV_STYLE_X,            { .num = 0 }                },
    { LV_STYLE_Y,            { .num = 76 }               },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }     },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                },
    { LV_STYLE_RADIUS,       { .num = 4 }                },
    { LV_STYLE_PAD_LEFT,     { .num = 4 }                },
    { LV_STYLE_PAD_RIGHT,    { .num = 4 }                },
    { LV_STYLE_PAD_TOP,      { .num = 4 }                },
    { LV_STYLE_PAD_BOTTOM,   { .num = 4 }                },
    { LV_STYLE_CLIP_CORNER,  { .num = true }             },
};
STYLE_HANDLE(selectionCard);

static const lv_style_const_prop_t regionRollerProps[] = {
    { LV_STYLE_WIDTH,  { .num = 278 } },
    { LV_STYLE_HEIGHT, { .num = 94 }  },
    { LV_STYLE_X,      { .num = 4 }   },
    { LV_STYLE_Y,      { .num = 4 }   },
};
STYLE_HANDLE(regionRoller);

static const lv_style_const_prop_t textCenteredProps[] = {
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
};
STYLE_HANDLE(textCentered);

static const lv_style_const_prop_t selectedTextCenteredProps[] = {
    { LV_STYLE_BG_OPA,     { .num = LV_OPA_COVER }         },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
};
STYLE_HANDLE(selectedTextCentered);

static const lv_style_const_prop_t stepPreviousProps[] = {
    { LV_STYLE_X, { .num = 22 }  },
    { LV_STYLE_Y, { .num = 194 } },
};
STYLE_HANDLE(stepPrevious);

static const lv_style_const_prop_t stepAcceptProps[] = {
    { LV_STYLE_X,      { .num = 106 } },
    { LV_STYLE_Y,      { .num = 194 } },
    { LV_STYLE_WIDTH,  { .num = 108 } },
    { LV_STYLE_HEIGHT, { .num = 34 }  },
};
STYLE_HANDLE(stepAccept);

static const lv_style_const_prop_t stepNextProps[] = {
    { LV_STYLE_X, { .num = 224 } },
    { LV_STYLE_Y, { .num = 194 } },
};
STYLE_HANDLE(stepNext);

static const lv_style_const_prop_t conversationColumnProps[] = {
    { LV_STYLE_PAD_LEFT,   { .num = 0 }                   },
    { LV_STYLE_PAD_RIGHT,  { .num = 0 }                   },
    { LV_STYLE_PAD_TOP,    { .num = 0 }                   },
    { LV_STYLE_PAD_BOTTOM, { .num = 0 }                   },
    { LV_STYLE_LAYOUT,     { .num = FlexLayout }          },
    { FlexFlow,            { .num = LV_FLEX_FLOW_COLUMN } },
    { LV_STYLE_PAD_ROW,    { .num = 0 }                   },
};
STYLE_HANDLE(conversationColumn);

static const lv_style_const_prop_t conversationHeaderProps[] = {
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }           },
    { LV_STYLE_HEIGHT,       { .num = 56 }                    },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }          },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                     },
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_BOTTOM } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                     },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                     },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                     },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                     },
    { LV_STYLE_RADIUS,       { .num = 0 }                     },
};
STYLE_HANDLE(conversationHeader);

static const lv_style_const_prop_t backCaptionProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_LEFT_MID } },
    { LV_STYLE_X,     { .num = 6 }                 },
    { LV_STYLE_Y,     { .num = 0 }                 },
};
STYLE_HANDLE(backCaption);

static const lv_style_const_prop_t conversationHeadingProps[] = {
    { LV_STYLE_WIDTH,  { .num = 770 } },
    { LV_STYLE_HEIGHT, { .num = 28 }  },
    { LV_STYLE_X,      { .num = 22 }  },
    { LV_STYLE_Y,      { .num = 2 }   },
};
STYLE_HANDLE(conversationHeading);

static const lv_style_const_prop_t conversationSubtitleProps[] = {
    { LV_STYLE_WIDTH,  { .num = 770 } },
    { LV_STYLE_HEIGHT, { .num = 19 }  },
    { LV_STYLE_X,      { .num = 22 }  },
    { LV_STYLE_Y,      { .num = 30 }  },
};
STYLE_HANDLE(conversationSubtitle);

static const lv_style_const_prop_t messageHistoryProps[] = {
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }         },
    { FlexGrow,              { .num = 1 }                   },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }        },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                   },
    { LV_STYLE_PAD_LEFT,     { .num = 6 }                   },
    { LV_STYLE_PAD_RIGHT,    { .num = 6 }                   },
    { LV_STYLE_PAD_TOP,      { .num = 6 }                   },
    { LV_STYLE_PAD_BOTTOM,   { .num = 6 }                   },
    { LV_STYLE_PAD_ROW,      { .num = 7 }                   },
    { LV_STYLE_RADIUS,       { .num = 0 }                   },
    { LV_STYLE_LAYOUT,       { .num = FlexLayout }          },
    { FlexFlow,              { .num = LV_FLEX_FLOW_COLUMN } },
};
STYLE_HANDLE(messageHistory);

static const lv_style_const_prop_t historyNoticeProps[] = {
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = 28 }          },
};
STYLE_HANDLE(historyNotice);

static const lv_style_const_prop_t composerRowProps[] = {
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }        },
    { LV_STYLE_HEIGHT,       { .num = 56 }                 },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }       },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                  },
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_TOP } },
    { LV_STYLE_PAD_LEFT,     { .num = 3 }                  },
    { LV_STYLE_PAD_RIGHT,    { .num = 3 }                  },
    { LV_STYLE_PAD_TOP,      { .num = 3 }                  },
    { LV_STYLE_PAD_BOTTOM,   { .num = 3 }                  },
    { LV_STYLE_RADIUS,       { .num = 0 }                  },
};
STYLE_HANDLE(composerRow);

static const lv_style_const_prop_t messageInputProps[] = {
    { LV_STYLE_WIDTH,  { .num = 700 }               },
    { LV_STYLE_HEIGHT, { .num = 46 }                },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_LEFT_MID } },
    { LV_STYLE_X,      { .num = 0 }                 },
    { LV_STYLE_Y,      { .num = 0 }                 },
};
STYLE_HANDLE(messageInput);

static const lv_style_const_prop_t compactInputProps[] = {
    { LV_STYLE_BORDER_WIDTH, { .num = 1 } },
    { LV_STYLE_PAD_LEFT,     { .num = 2 } },
    { LV_STYLE_PAD_RIGHT,    { .num = 2 } },
    { LV_STYLE_PAD_TOP,      { .num = 2 } },
    { LV_STYLE_PAD_BOTTOM,   { .num = 2 } },
};
STYLE_HANDLE(compactInput);

static const lv_style_const_prop_t sendActionProps[] = {
    { LV_STYLE_WIDTH,  { .num = 88 }                 },
    { LV_STYLE_HEIGHT, { .num = 46 }                 },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,      { .num = 0 }                  },
    { LV_STYLE_Y,      { .num = 0 }                  },
};
STYLE_HANDLE(sendAction);

static const lv_style_const_prop_t recordActionProps[] = {
    { LV_STYLE_WIDTH,  { .num = 58 }                 },
    { LV_STYLE_HEIGHT, { .num = 23 }                 },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,      { .num = -92 }                },
    { LV_STYLE_Y,      { .num = 0 }                  },
};
STYLE_HANDLE(recordAction);

static const lv_style_const_prop_t messageInputWithAudioProps[] = {
    { LV_STYLE_WIDTH, { .num = 638 } },
};
STYLE_HANDLE(messageInputWithAudio);

static const lv_style_const_prop_t headingWithAudioProps[] = {
    { LV_STYLE_WIDTH, { .num = 708 } },
};
STYLE_HANDLE(headingWithAudio);

static const lv_style_const_prop_t audioHeaderActionProps[] = {
    { LV_STYLE_WIDTH,  { .num = 58 }                 },
    { LV_STYLE_HEIGHT, { .num = 28 }                 },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,      { .num = -3 }                 },
    { LV_STYLE_Y,      { .num = 0 }                  },
};
STYLE_HANDLE(audioHeaderAction);

static const lv_style_const_prop_t pendingAudioRowProps[] = {
    { LV_STYLE_WIDTH,        { .num = 788 }           },
    { LV_STYLE_HEIGHT,       { .num = 48 }            },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }             },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }             },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }             },
    { LV_STYLE_PAD_TOP,      { .num = 0 }             },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }             },
    { LV_STYLE_RADIUS,       { .num = 0 }             },
};
STYLE_HANDLE(pendingAudioRow);

static const lv_style_const_prop_t pendingAudioBubbleProps[] = {
    { LV_STYLE_WIDTH,        { .num = 168 }                },
    { LV_STYLE_HEIGHT,       { .num = 48 }                 },
    { LV_STYLE_ALIGN,        { .num = LV_ALIGN_TOP_RIGHT } },
    { LV_STYLE_X,            { .num = 0 }                  },
    { LV_STYLE_Y,            { .num = 0 }                  },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }       },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                  },
    { LV_STYLE_PAD_LEFT,     { .num = 7 }                  },
    { LV_STYLE_PAD_RIGHT,    { .num = 7 }                  },
    { LV_STYLE_PAD_TOP,      { .num = 7 }                  },
    { LV_STYLE_PAD_BOTTOM,   { .num = 7 }                  },
    { LV_STYLE_RADIUS,       { .num = 6 }                  },
};
STYLE_HANDLE(pendingAudioBubble);

static const lv_style_const_prop_t centeredCaptionProps[] = {
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_CENTER }      },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 0 }                    },
};
STYLE_HANDLE(centeredCaption);

static const lv_style_const_prop_t messageRowProps[] = {
    { LV_STYLE_WIDTH,        { .num = 788 }             },
    { LV_STYLE_HEIGHT,       { .num = LV_SIZE_CONTENT } },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }   },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }               },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }               },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }               },
    { LV_STYLE_PAD_TOP,      { .num = 0 }               },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }               },
    { LV_STYLE_RADIUS,       { .num = 0 }               },
};
STYLE_HANDLE(messageRow);

static const lv_style_const_prop_t bubbleContentProps[] = {
    { LV_STYLE_HEIGHT,       { .num = LV_SIZE_CONTENT }     },
    { LV_STYLE_LAYOUT,       { .num = FlexLayout }          },
    { FlexFlow,              { .num = LV_FLEX_FLOW_COLUMN } },
    { LV_STYLE_PAD_LEFT,     { .num = 7 }                   },
    { LV_STYLE_PAD_RIGHT,    { .num = 7 }                   },
    { LV_STYLE_PAD_TOP,      { .num = 5 }                   },
    { LV_STYLE_PAD_BOTTOM,   { .num = 5 }                   },
    { LV_STYLE_PAD_ROW,      { .num = 3 }                   },
    { LV_STYLE_RADIUS,       { .num = 6 }                   },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                   },
};
STYLE_HANDLE(bubbleContent);

static const lv_style_const_prop_t incomingPositionProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_TOP_LEFT } },
    { LV_STYLE_X,     { .num = 0 }                 },
    { LV_STYLE_Y,     { .num = 0 }                 },
};
STYLE_HANDLE(incomingPosition);

static const lv_style_const_prop_t outgoingPositionProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_TOP_RIGHT } },
    { LV_STYLE_X,     { .num = 0 }                  },
    { LV_STYLE_Y,     { .num = 0 }                  },
};
STYLE_HANDLE(outgoingPosition);

static const lv_style_const_prop_t messageMetadataProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }               },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                           },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                           },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                           },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                           },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                           },
    { LV_STYLE_RADIUS,       { .num = 0 }                           },
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }                 },
    { LV_STYLE_HEIGHT,       { .num = LV_SIZE_CONTENT }             },
    { LV_STYLE_LAYOUT,       { .num = FlexLayout }                  },
    { FlexFlow,              { .num = LV_FLEX_FLOW_ROW }            },
    { FlexMain,              { .num = LV_FLEX_ALIGN_SPACE_BETWEEN } },
    { FlexCross,             { .num = LV_FLEX_ALIGN_START }         },
};
STYLE_HANDLE(messageMetadata);

static const lv_style_const_prop_t leftCenteredProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_LEFT_MID } },
    { LV_STYLE_X,     { .num = 0 }                 },
    { LV_STYLE_Y,     { .num = 0 }                 },
};
STYLE_HANDLE(leftCentered);

static const lv_style_const_prop_t rightCenteredProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,     { .num = 0 }                  },
    { LV_STYLE_Y,     { .num = 0 }                  },
};
STYLE_HANDLE(rightCentered);

static const lv_style_const_prop_t textRightProps[] = {
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_RIGHT } },
};
STYLE_HANDLE(textRight);

static const lv_style_const_prop_t actionMenuPaddingProps[] = {
    { LV_STYLE_PAD_LEFT,   { .num = 8 } },
    { LV_STYLE_PAD_RIGHT,  { .num = 8 } },
    { LV_STYLE_PAD_TOP,    { .num = 8 } },
    { LV_STYLE_PAD_BOTTOM, { .num = 8 } },
};
STYLE_HANDLE(actionMenuPadding);

static const lv_style_const_prop_t topCenteredProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_TOP_MID } },
    { LV_STYLE_X,     { .num = 0 }                },
    { LV_STYLE_Y,     { .num = 0 }                },
};
STYLE_HANDLE(topCentered);

static const lv_style_const_prop_t actionMenuHintProps[] = {
    { LV_STYLE_WIDTH,      { .num = 224 }                  },
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_CENTER } },
    { LV_STYLE_X,          { .num = 0 }                    },
    { LV_STYLE_Y,          { .num = 22 }                   },
};
STYLE_HANDLE(actionMenuHint);

static const lv_style_const_prop_t actionSurfaceProps[] = {
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }            },
    { LV_STYLE_RADIUS,       { .num = 4 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
};
STYLE_HANDLE(actionSurface);

static const lv_style_const_prop_t settingsListProps[] = {
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }         },
    { LV_STYLE_HEIGHT,       { .num = LV_PCT(100) }         },
    { LV_STYLE_X,            { .num = 0 }                   },
    { LV_STYLE_Y,            { .num = 0 }                   },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }        },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                   },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                   },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                   },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                   },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                   },
    { LV_STYLE_PAD_ROW,      { .num = 0 }                   },
    { LV_STYLE_RADIUS,       { .num = 0 }                   },
    { LV_STYLE_LAYOUT,       { .num = FlexLayout }          },
    { FlexFlow,              { .num = LV_FLEX_FLOW_COLUMN } },
};
STYLE_HANDLE(settingsList);

static const lv_style_const_prop_t sectionTitleRowProps[] = {
    { LV_STYLE_WIDTH,        { .num = 800 }                   },
    { LV_STYLE_HEIGHT,       { .num = 40 }                    },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }          },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                     },
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_BOTTOM } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                     },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                     },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                     },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                     },
    { LV_STYLE_RADIUS,       { .num = 0 }                     },
};
STYLE_HANDLE(sectionTitleRow);

static const lv_style_const_prop_t rowHeadingProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_LEFT_MID } },
    { LV_STYLE_X,     { .num = 8 }                 },
    { LV_STYLE_Y,     { .num = 0 }                 },
};
STYLE_HANDLE(rowHeading);

static const lv_style_const_prop_t rowIndicatorProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,     { .num = -8 }                 },
    { LV_STYLE_Y,     { .num = 0 }                  },
};
STYLE_HANDLE(rowIndicator);

static const lv_style_const_prop_t categoryRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = 800 } },
    { LV_STYLE_HEIGHT, { .num = 52 }  },
};
STYLE_HANDLE(categoryRow);

static const lv_style_const_prop_t categoryNameProps[] = {
    { LV_STYLE_WIDTH, { .num = 756 }               },
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_TOP_LEFT } },
    { LV_STYLE_X,     { .num = 12 }                },
    { LV_STYLE_Y,     { .num = 4 }                 },
};
STYLE_HANDLE(categoryName);

static const lv_style_const_prop_t categorySummaryProps[] = {
    { LV_STYLE_WIDTH, { .num = 756 }                  },
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_BOTTOM_LEFT } },
    { LV_STYLE_X,     { .num = 20 }                   },
    { LV_STYLE_Y,     { .num = -4 }                   },
};
STYLE_HANDLE(categorySummary);

static const lv_style_const_prop_t categoryHeadingProps[] = {
    { LV_STYLE_WIDTH, { .num = 672 }               },
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_LEFT_MID } },
    { LV_STYLE_X,     { .num = 8 }                 },
    { LV_STYLE_Y,     { .num = 0 }                 },
};
STYLE_HANDLE(categoryHeading);

static const lv_style_const_prop_t noticeRowProps[] = {
    { LV_STYLE_WIDTH,        { .num = 800 }                   },
    { LV_STYLE_HEIGHT,       { .num = 20 }                    },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER }          },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                     },
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_BOTTOM } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                     },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                     },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                     },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                     },
    { LV_STYLE_RADIUS,       { .num = 0 }                     },
};
STYLE_HANDLE(noticeRow);

static const lv_style_const_prop_t noticeCaptionProps[] = {
    { LV_STYLE_WIDTH, { .num = 784 }               },
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_LEFT_MID } },
    { LV_STYLE_X,     { .num = 8 }                 },
    { LV_STYLE_Y,     { .num = 0 }                 },
};
STYLE_HANDLE(noticeCaption);

static const lv_style_const_prop_t confirmationRowProps[] = {
    { LV_STYLE_WIDTH,        { .num = 800 }          },
    { LV_STYLE_HEIGHT,       { .num = 56 }           },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 2 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_RADIUS,       { .num = 0 }            },
};
STYLE_HANDLE(confirmationRow);

static const lv_style_const_prop_t confirmationTitleProps[] = {
    { LV_STYLE_WIDTH, { .num = 784 }               },
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_TOP_LEFT } },
    { LV_STYLE_X,     { .num = 8 }                 },
    { LV_STYLE_Y,     { .num = 3 }                 },
};
STYLE_HANDLE(confirmationTitle);

static const lv_style_const_prop_t confirmationDetailProps[] = {
    { LV_STYLE_WIDTH, { .num = 784 }                  },
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_BOTTOM_LEFT } },
    { LV_STYLE_X,     { .num = 8 }                    },
    { LV_STYLE_Y,     { .num = -3 }                   },
};
STYLE_HANDLE(confirmationDetail);

static const lv_style_const_prop_t regularRowHeightProps[] = {
    { LV_STYLE_HEIGHT, { .num = 52 } },
};
STYLE_HANDLE(regularRowHeight);

static const lv_style_const_prop_t settingNameWidthProps[] = {
    { LV_STYLE_WIDTH, { .num = 500 } },
};
STYLE_HANDLE(settingNameWidth);

static const lv_style_const_prop_t settingHintProps[] = {
    { LV_STYLE_TEXT_LINE_SPACE, { .num = 2 }                    },
    { LV_STYLE_ALIGN,           { .num = LV_ALIGN_BOTTOM_LEFT } },
    { LV_STYLE_X,               { .num = 8 }                    },
    { LV_STYLE_Y,               { .num = -3 }                   },
};
STYLE_HANDLE(settingHint);

static const lv_style_const_prop_t settingFootnoteProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_BOTTOM_LEFT } },
    { LV_STYLE_X,     { .num = 8 }                    },
    { LV_STYLE_Y,     { .num = -2 }                   },
};
STYLE_HANDLE(settingFootnote);

static const lv_style_const_prop_t scanHeaderProps[] = {
    { LV_STYLE_WIDTH,        { .num = 800 }                   },
    { LV_STYLE_HEIGHT,       { .num = 40 }                    },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }         },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }                     },
    { LV_STYLE_BORDER_SIDE,  { .num = LV_BORDER_SIDE_BOTTOM } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                     },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                     },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                     },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                     },
    { LV_STYLE_RADIUS,       { .num = 0 }                     },
};
STYLE_HANDLE(scanHeader);

static const lv_style_const_prop_t scanTitleProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_LEFT_MID } },
    { LV_STYLE_X,     { .num = 4 }                 },
    { LV_STYLE_Y,     { .num = 0 }                 },
};
STYLE_HANDLE(scanTitle);

static const lv_style_const_prop_t signalCaptionProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,     { .num = -4 }                 },
    { LV_STYLE_Y,     { .num = 0 }                  },
};
STYLE_HANDLE(signalCaption);

static const lv_style_const_prop_t retryRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = 800 } },
    { LV_STYLE_HEIGHT, { .num = 40 }  },
};
STYLE_HANDLE(retryRow);

static const lv_style_const_prop_t editingValueProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,     { .num = -8 }                 },
    { LV_STYLE_Y,     { .num = -9 }                 },
};
STYLE_HANDLE(editingValue);

static const lv_style_const_prop_t serviceHeadingProps[] = {
    { LV_STYLE_WIDTH,  { .num = 784 }               },
    { LV_STYLE_HEIGHT, { .num = 24 }                },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_TOP_LEFT } },
    { LV_STYLE_X,      { .num = 8 }                 },
    { LV_STYLE_Y,      { .num = 6 }                 },
};
STYLE_HANDLE(serviceHeading);

static const lv_style_const_prop_t serviceDetailProps[] = {
    { LV_STYLE_WIDTH,  { .num = 784 }               },
    { LV_STYLE_HEIGHT, { .num = 19 }                },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_TOP_LEFT } },
    { LV_STYLE_X,      { .num = 8 }                 },
    { LV_STYLE_Y,      { .num = 28 }                },
};
STYLE_HANDLE(serviceDetail);

static const lv_style_const_prop_t screenOverlayProps[] = {
    { LV_STYLE_WIDTH,  { .num = 800 } },
    { LV_STYLE_HEIGHT, { .num = 480 } },
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 0 }   },
};
STYLE_HANDLE(screenOverlay);

static const lv_style_const_prop_t plainPanelProps[] = {
    { LV_STYLE_BORDER_WIDTH, { .num = 0 } },
    { LV_STYLE_PAD_LEFT,     { .num = 0 } },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 } },
    { LV_STYLE_PAD_TOP,      { .num = 0 } },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 } },
};
STYLE_HANDLE(plainPanel);

static const lv_style_const_prop_t codeHeadingProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_TOP_MID } },
    { LV_STYLE_X,     { .num = 0 }                },
    { LV_STYLE_Y,     { .num = 6 }                },
};
STYLE_HANDLE(codeHeading);

static const lv_style_const_prop_t codePaperProps[] = {
    { LV_STYLE_X,            { .num = 8 }            },
    { LV_STYLE_Y,            { .num = 32 }           },
    { LV_STYLE_WIDTH,        { .num = 440 }          },
    { LV_STYLE_HEIGHT,       { .num = 440 }          },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_RADIUS,       { .num = 0 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
};
STYLE_HANDLE(codePaper);

static const lv_style_const_prop_t codeFormatProps[] = {
    { LV_STYLE_X, { .num = 456 } },
    { LV_STYLE_Y, { .num = 38 }  },
};
STYLE_HANDLE(codeFormat);

static const lv_style_const_prop_t codeAddressProps[] = {
    { LV_STYLE_WIDTH, { .num = 336 } },
    { LV_STYLE_X,     { .num = 456 } },
    { LV_STYLE_Y,     { .num = 60 }  },
};
STYLE_HANDLE(codeAddress);

static const lv_style_const_prop_t codeToggleProps[] = {
    { LV_STYLE_X,      { .num = 456 } },
    { LV_STYLE_Y,      { .num = 390 } },
    { LV_STYLE_WIDTH,  { .num = 336 } },
    { LV_STYLE_HEIGHT, { .num = 32 }  },
};
STYLE_HANDLE(codeToggle);

static const lv_style_const_prop_t codeCloseProps[] = {
    { LV_STYLE_X,      { .num = 456 } },
    { LV_STYLE_Y,      { .num = 432 } },
    { LV_STYLE_WIDTH,  { .num = 336 } },
    { LV_STYLE_HEIGHT, { .num = 32 }  },
};
STYLE_HANDLE(codeClose);

static const lv_style_const_prop_t peerIndicatorProps[] = {
    { LV_STYLE_WIDTH,        { .num = 6 }            },
    { LV_STYLE_HEIGHT,       { .num = 6 }            },
    { LV_STYLE_RADIUS,       { .num = 3 }            },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
};
STYLE_HANDLE(peerIndicator);

static const lv_style_const_prop_t discoveryTitleProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_TOP_MID } },
    { LV_STYLE_X,     { .num = 0 }                },
    { LV_STYLE_Y,     { .num = 34 }               },
};
STYLE_HANDLE(discoveryTitle);

static const lv_style_const_prop_t discoveryHintProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_TOP_MID } },
    { LV_STYLE_X,     { .num = 0 }                },
    { LV_STYLE_Y,     { .num = 55 }               },
};
STYLE_HANDLE(discoveryHint);

static const lv_style_const_prop_t peerListProps[] = {
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 19 }  },
    { LV_STYLE_WIDTH,  { .num = 800 } },
    { LV_STYLE_HEIGHT, { .num = 385 } },
};
STYLE_HANDLE(peerList);

static const lv_style_const_prop_t scrollExtentProps[] = {
    { LV_STYLE_WIDTH,  { .num = 1 } },
    { LV_STYLE_HEIGHT, { .num = 1 } },
};
STYLE_HANDLE(scrollExtent);

static const lv_style_const_prop_t listCaptionProps[] = {
    { LV_STYLE_X, { .num = 8 } },
    { LV_STYLE_Y, { .num = 2 } },
};
STYLE_HANDLE(listCaption);

static const lv_style_const_prop_t peerRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = 800 } },
    { LV_STYLE_HEIGHT, { .num = 56 }  },
};
STYLE_HANDLE(peerRow);

static const lv_style_const_prop_t peerNameProps[] = {
    { LV_STYLE_X,      { .num = 8 }   },
    { LV_STYLE_Y,      { .num = 3 }   },
    { LV_STYLE_WIDTH,  { .num = 668 } },
    { LV_STYLE_HEIGHT, { .num = 24 }  },
};
STYLE_HANDLE(peerName);

static const lv_style_const_prop_t peerAgeProps[] = {
    { LV_STYLE_TEXT_ALIGN, { .num = LV_TEXT_ALIGN_RIGHT } },
    { LV_STYLE_X,          { .num = 676 }                 },
    { LV_STYLE_Y,          { .num = 5 }                   },
    { LV_STYLE_WIDTH,      { .num = 116 }                 },
    { LV_STYLE_HEIGHT,     { .num = 19 }                  },
};
STYLE_HANDLE(peerAge);

static const lv_style_const_prop_t peerAddressProps[] = {
    { LV_STYLE_X,      { .num = 8 }   },
    { LV_STYLE_Y,      { .num = 28 }  },
    { LV_STYLE_WIDTH,  { .num = 784 } },
    { LV_STYLE_HEIGHT, { .num = 19 }  },
};
STYLE_HANDLE(peerAddress);

static const lv_style_const_prop_t peerDialogProps[] = {
    { LV_STYLE_WIDTH,  { .num = 420 }             },
    { LV_STYLE_HEIGHT, { .num = 260 }             },
    { LV_STYLE_X,      { .num = 0 }               },
    { LV_STYLE_Y,      { .num = 0 }               },
    { LV_STYLE_ALIGN,  { .num = LV_ALIGN_CENTER } },
};
STYLE_HANDLE(peerDialog);

static const lv_style_const_prop_t peerDialogTitleProps[] = {
    { LV_STYLE_WIDTH, { .num = 236 } },
    { LV_STYLE_X,     { .num = 12 }  },
    { LV_STYLE_Y,     { .num = 9 }   },
};
STYLE_HANDLE(peerDialogTitle);

static const lv_style_const_prop_t peerDialogMetaProps[] = {
    { LV_STYLE_WIDTH, { .num = 236 } },
    { LV_STYLE_X,     { .num = 12 }  },
    { LV_STYLE_Y,     { .num = 38 }  },
};
STYLE_HANDLE(peerDialogMeta);

static const lv_style_const_prop_t peerDialogReachProps[] = {
    { LV_STYLE_WIDTH, { .num = 236 } },
    { LV_STYLE_X,     { .num = 12 }  },
    { LV_STYLE_Y,     { .num = 57 }  },
};
STYLE_HANDLE(peerDialogReach);

static const lv_style_const_prop_t dialogActionProps[] = {
    { LV_STYLE_WIDTH,        { .num = 396 }          },
    { LV_STYLE_HEIGHT,       { .num = 48 }           },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_RADIUS,       { .num = 4 }            },
};
STYLE_HANDLE(dialogAction);

static const lv_style_const_prop_t nicknamePanelProps[] = {
    { LV_STYLE_WIDTH,        { .num = 236 }           },
    { LV_STYLE_HEIGHT,       { .num = 86 }            },
    { LV_STYLE_X,            { .num = 12 }            },
    { LV_STYLE_Y,            { .num = 63 }            },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }             },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }             },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }             },
    { LV_STYLE_PAD_TOP,      { .num = 0 }             },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }             },
};
STYLE_HANDLE(nicknamePanel);

static const lv_style_const_prop_t originProps[] = {
    { LV_STYLE_X, { .num = 0 } },
    { LV_STYLE_Y, { .num = 0 } },
};
STYLE_HANDLE(origin);

static const lv_style_const_prop_t nicknameValueProps[] = {
    { LV_STYLE_WIDTH, { .num = 232 } },
    { LV_STYLE_X,     { .num = 0 }   },
    { LV_STYLE_Y,     { .num = 22 }  },
};
STYLE_HANDLE(nicknameValue);

static const lv_style_const_prop_t nicknameHintProps[] = {
    { LV_STYLE_X, { .num = 0 }  },
    { LV_STYLE_Y, { .num = 46 } },
};
STYLE_HANDLE(nicknameHint);

static const lv_style_const_prop_t nicknameCancelProps[] = {
    { LV_STYLE_X,      { .num = 154 } },
    { LV_STYLE_Y,      { .num = 61 }  },
    { LV_STYLE_WIDTH,  { .num = 80 }  },
    { LV_STYLE_HEIGHT, { .num = 22 }  },
};
STYLE_HANDLE(nicknameCancel);

static const lv_style_const_prop_t browserColumnProps[] = {
    { LV_STYLE_PAD_LEFT,     { .num = 4 }                   },
    { LV_STYLE_PAD_RIGHT,    { .num = 4 }                   },
    { LV_STYLE_PAD_TOP,      { .num = 4 }                   },
    { LV_STYLE_PAD_BOTTOM,   { .num = 4 }                   },
    { LV_STYLE_PAD_ROW,      { .num = 3 }                   },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                   },
    { FlexFlow,              { .num = LV_FLEX_FLOW_COLUMN } },
};
STYLE_HANDLE(browserColumn);

static const lv_style_const_prop_t browserTitleProps[] = {
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 0 }   },
    { LV_STYLE_WIDTH,  { .num = 784 } },
    { LV_STYLE_HEIGHT, { .num = 24 }  },
};
STYLE_HANDLE(browserTitle);

static const lv_style_const_prop_t browserInputProps[] = {
    { LV_STYLE_WIDTH,  { .num = 788 } },
    { LV_STYLE_HEIGHT, { .num = 32 }  },
};
STYLE_HANDLE(browserInput);

static const lv_style_const_prop_t browserActionProps[] = {
    { LV_STYLE_WIDTH,  { .num = 788 } },
    { LV_STYLE_HEIGHT, { .num = 29 }  },
};
STYLE_HANDLE(browserAction);

static const lv_style_const_prop_t fieldPaddingProps[] = {
    { LV_STYLE_PAD_LEFT,   { .num = 4 } },
    { LV_STYLE_PAD_RIGHT,  { .num = 4 } },
    { LV_STYLE_PAD_TOP,    { .num = 4 } },
    { LV_STYLE_PAD_BOTTOM, { .num = 4 } },
};
STYLE_HANDLE(fieldPadding);

static const lv_style_const_prop_t browserCaptionProps[] = {
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 0 }   },
    { LV_STYLE_WIDTH,  { .num = 776 } },
    { LV_STYLE_HEIGHT, { .num = 24 }  },
};
STYLE_HANDLE(browserCaption);

static const lv_style_const_prop_t loadingCaptionProps[] = {
    { LV_STYLE_X,     { .num = 0 }   },
    { LV_STYLE_Y,     { .num = 0 }   },
    { LV_STYLE_WIDTH, { .num = 784 } },
};
STYLE_HANDLE(loadingCaption);

static const lv_style_const_prop_t resultCaptionProps[] = {
    { LV_STYLE_X,     { .num = 0 }   },
    { LV_STYLE_Y,     { .num = 0 }   },
    { LV_STYLE_WIDTH, { .num = 776 } },
};
STYLE_HANDLE(resultCaption);

static const lv_style_const_prop_t bodyLineProps[] = {
    { LV_STYLE_HEIGHT, { .num = 24 } },
};
STYLE_HANDLE(bodyLine);

static const lv_style_const_prop_t dividerProps[] = {
    { LV_STYLE_WIDTH,        { .num = 784 }          },
    { LV_STYLE_HEIGHT,       { .num = 1 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_RADIUS,       { .num = 0 }            },
};
STYLE_HANDLE(divider);

static const lv_style_const_prop_t fullScreenPositionProps[] = {
    { LV_STYLE_X,      { .num = 0 }           },
    { LV_STYLE_Y,      { .num = 0 }           },
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = LV_PCT(100) } },
};
STYLE_HANDLE(fullScreenPosition);

static const lv_style_const_prop_t contentViewportProps[] = {
    { LV_STYLE_X,      { .num = 0 }           },
    { LV_STYLE_Y,      { .num = 28 }          },
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = 404 }         },
};
STYLE_HANDLE(contentViewport);

static const lv_style_const_prop_t keyboardRowProps[] = {
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }      },
    { LV_STYLE_HEIGHT,       { .num = 50 }               },
    { LV_STYLE_LAYOUT,       { .num = FlexLayout }       },
    { FlexFlow,              { .num = LV_FLEX_FLOW_ROW } },
    { LV_STYLE_PAD_COLUMN,   { .num = 4 }                },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }    },
};
STYLE_HANDLE(keyboardRow);

static const lv_style_const_prop_t keyboardKeyProps[] = {
    { LV_STYLE_WIDTH,     { .num = 0 }                  },
    { LV_STYLE_HEIGHT,    { .num = LV_PCT(100) }        },
    { FlexGrow,           { .num = 1 }                  },
    { LV_STYLE_RADIUS,    { .num = 5 }                  },
    { LV_STYLE_BG_OPA,    { .num = LV_OPA_COVER }       },
    { LV_STYLE_TEXT_FONT, { .ptr = &lv_font_rsdeck_14 } },
};
STYLE_HANDLE(keyboardKey);

static const lv_style_const_prop_t spaceKeyProps[] = {
    { FlexGrow, { .num = 4 } },
};
STYLE_HANDLE(spaceKey);

static const lv_style_const_prop_t wideKeyProps[] = {
    { FlexGrow, { .num = 2 } },
};
STYLE_HANDLE(wideKey);

static const lv_style_const_prop_t footerCancelProps[] = {
    { LV_STYLE_ALIGN,     { .num = LV_ALIGN_BOTTOM_LEFT } },
    { LV_STYLE_WIDTH,     { .num = 150 }                  },
    { LV_STYLE_HEIGHT,    { .num = 44 }                   },
    { LV_STYLE_RADIUS,    { .num = 5 }                    },
    { LV_STYLE_BG_OPA,    { .num = LV_OPA_COVER }         },
    { LV_STYLE_TEXT_FONT, { .ptr = &lv_font_rsdeck_14 }   },
};
STYLE_HANDLE(footerCancel);

static const lv_style_const_prop_t footerAcceptProps[] = {
    { LV_STYLE_ALIGN,     { .num = LV_ALIGN_BOTTOM_RIGHT } },
    { LV_STYLE_WIDTH,     { .num = 150 }                   },
    { LV_STYLE_HEIGHT,    { .num = 44 }                    },
    { LV_STYLE_RADIUS,    { .num = 5 }                     },
    { LV_STYLE_BG_OPA,    { .num = LV_OPA_COVER }          },
    { LV_STYLE_TEXT_FONT, { .ptr = &lv_font_rsdeck_14 }    },
};
STYLE_HANDLE(footerAccept);

static const lv_style_const_prop_t screenRootProps[] = {
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }  },
    { LV_STYLE_HEIGHT,       { .num = LV_PCT(100) }  },
    { LV_STYLE_X,            { .num = 0 }            },
    { LV_STYLE_Y,            { .num = 0 }            },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_RADIUS,       { .num = 0 }            },
};
STYLE_HANDLE(screenRoot);

static const lv_style_const_prop_t primaryActionProps[] = {
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 350 } },
    { LV_STYLE_WIDTH,  { .num = 776 } },
    { LV_STYLE_HEIGHT, { .num = 40 }  },
};
STYLE_HANDLE(primaryAction);

static const lv_style_const_prop_t secondaryActionLeftProps[] = {
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 400 } },
    { LV_STYLE_WIDTH,  { .num = 385 } },
    { LV_STYLE_HEIGHT, { .num = 32 }  },
};
STYLE_HANDLE(secondaryActionLeft);

static const lv_style_const_prop_t secondaryActionRightProps[] = {
    { LV_STYLE_X,      { .num = 391 } },
    { LV_STYLE_Y,      { .num = 400 } },
    { LV_STYLE_WIDTH,  { .num = 385 } },
    { LV_STYLE_HEIGHT, { .num = 32 }  },
};
STYLE_HANDLE(secondaryActionRight);

static const lv_style_const_prop_t confirmActionLeftProps[] = {
    { LV_STYLE_X,      { .num = 0 }   },
    { LV_STYLE_Y,      { .num = 380 } },
    { LV_STYLE_WIDTH,  { .num = 385 } },
    { LV_STYLE_HEIGHT, { .num = 40 }  },
};
STYLE_HANDLE(confirmActionLeft);

static const lv_style_const_prop_t confirmActionRightProps[] = {
    { LV_STYLE_X,      { .num = 391 } },
    { LV_STYLE_Y,      { .num = 380 } },
    { LV_STYLE_WIDTH,  { .num = 385 } },
    { LV_STYLE_HEIGHT, { .num = 40 }  },
};
STYLE_HANDLE(confirmActionRight);

static const lv_style_const_prop_t borderStrongProps[] = {
    { LV_STYLE_BORDER_WIDTH, { .num = 2 } },
};
STYLE_HANDLE(borderStrong);

static const lv_style_const_prop_t bottomEdgeProps[] = {
    { LV_STYLE_BORDER_SIDE, { .num = LV_BORDER_SIDE_BOTTOM } },
};
STYLE_HANDLE(bottomEdge);

static const lv_style_const_prop_t pendingEdgeProps[] = {
    { LV_STYLE_BORDER_SIDE, { .num = LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_BOTTOM } },
};
STYLE_HANDLE(pendingEdge);

static const lv_style_const_prop_t settingRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = 52 }          },
};
STYLE_HANDLE(settingRow);

static const lv_style_const_prop_t settingDetailRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = 64 }          },
};
STYLE_HANDLE(settingDetailRow);

static const lv_style_const_prop_t serviceRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = 32 }          },
};
STYLE_HANDLE(serviceRow);

static const lv_style_const_prop_t serviceDetailRowProps[] = {
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = 48 }          },
};
STYLE_HANDLE(serviceDetailRow);

static const lv_style_const_prop_t settingNameHintProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_LEFT_MID } },
    { LV_STYLE_X,     { .num = 8 }                 },
    { LV_STYLE_Y,     { .num = -9 }                },
};
STYLE_HANDLE(settingNameHint);

static const lv_style_const_prop_t settingNameDetailsProps[] = {
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_LEFT_MID } },
    { LV_STYLE_X,     { .num = 8 }                 },
    { LV_STYLE_Y,     { .num = -18 }               },
};
STYLE_HANDLE(settingNameDetails);

static const lv_style_const_prop_t settingValueProps[] = {
    { LV_STYLE_WIDTH, { .num = 280 }                },
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,     { .num = -8 }                 },
    { LV_STYLE_Y,     { .num = 0 }                  },
};
STYLE_HANDLE(settingValue);

static const lv_style_const_prop_t settingValueHintProps[] = {
    { LV_STYLE_WIDTH, { .num = 280 }                },
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,     { .num = -8 }                 },
    { LV_STYLE_Y,     { .num = -9 }                 },
};
STYLE_HANDLE(settingValueHint);

static const lv_style_const_prop_t settingValueDetailsProps[] = {
    { LV_STYLE_WIDTH, { .num = 280 }                },
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,     { .num = -8 }                 },
    { LV_STYLE_Y,     { .num = -18 }                },
};
STYLE_HANDLE(settingValueDetails);

static const lv_style_const_prop_t settingFullValueProps[] = {
    { LV_STYLE_WIDTH, { .num = LV_PCT(98) }         },
    { LV_STYLE_ALIGN, { .num = LV_ALIGN_RIGHT_MID } },
    { LV_STYLE_X,     { .num = -8 }                 },
    { LV_STYLE_Y,     { .num = 10 }                 },
};
STYLE_HANDLE(settingFullValue);

static const lv_style_const_prop_t pageNavigationProps[] = {
    { LV_STYLE_ALIGN,        { .num = LV_ALIGN_BOTTOM_MID } },
    { LV_STYLE_Y,            { .num = -3 }                  },
    { LV_STYLE_WIDTH,        { .num = LV_PCT(99) }          },
    { LV_STYLE_HEIGHT,       { .num = 30 }                  },
    { LV_STYLE_LAYOUT,       { .num = FlexLayout }          },
    { FlexFlow,              { .num = LV_FLEX_FLOW_ROW }    },
    { LV_STYLE_PAD_COLUMN,   { .num = 3 }                   },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                   },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                   },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                   },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                   },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                   },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }       },
};
STYLE_HANDLE(pageNavigation);

static const lv_style_const_prop_t pageNavigationButtonProps[] = {
    { LV_STYLE_WIDTH,      { .num = 0 }           },
    { LV_STYLE_HEIGHT,     { .num = LV_PCT(100) } },
    { FlexGrow,            { .num = 1 }           },
    { LV_STYLE_PAD_LEFT,   { .num = 0 }           },
    { LV_STYLE_PAD_RIGHT,  { .num = 0 }           },
    { LV_STYLE_PAD_TOP,    { .num = 0 }           },
    { LV_STYLE_PAD_BOTTOM, { .num = 0 }           },
    { LV_STYLE_LINE_WIDTH, { .num = 2 }           },
};
STYLE_HANDLE(pageNavigationButton);

static const lv_style_const_prop_t conversationListProps[] = {
    { LV_STYLE_X,      { .num = 0 }           },
    { LV_STYLE_Y,      { .num = 0 }           },
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = 368 }         },
};
STYLE_HANDLE(conversationList);

static const lv_style_const_prop_t conversationFamilyListProps[] = {
    { LV_STYLE_X,      { .num = 0 }           },
    { LV_STYLE_Y,      { .num = 24 }          },
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = 344 }         },
};
STYLE_HANDLE(conversationFamilyList);

static const lv_style_const_prop_t familyLeftProps[] = {
    { LV_STYLE_X,      { .num = 0 }          },
    { LV_STYLE_Y,      { .num = 0 }          },
    { LV_STYLE_WIDTH,  { .num = LV_PCT(50) } },
    { LV_STYLE_HEIGHT, { .num = 24 }         },
};
STYLE_HANDLE(familyLeft);

static const lv_style_const_prop_t familyRightProps[] = {
    { LV_STYLE_X,      { .num = LV_PCT(50) } },
    { LV_STYLE_Y,      { .num = 0 }          },
    { LV_STYLE_WIDTH,  { .num = LV_PCT(50) } },
    { LV_STYLE_HEIGHT, { .num = 24 }         },
};
STYLE_HANDLE(familyRight);

static const lv_style_const_prop_t browserContentProps[] = {
    { LV_STYLE_X,      { .num = 0 }           },
    { LV_STYLE_Y,      { .num = 24 }          },
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = 380 }         },
};
STYLE_HANDLE(browserContent);

static const lv_style_const_prop_t actionMenuProps[] = {
    { LV_STYLE_WIDTH,      { .num = 244 }                 },
    { LV_STYLE_HEIGHT,     { .num = LV_SIZE_CONTENT }     },
    { LV_STYLE_MAX_HEIGHT, { .num = 404 }                 },
    { LV_STYLE_LAYOUT,     { .num = FlexLayout }          },
    { FlexFlow,            { .num = LV_FLEX_FLOW_COLUMN } },
    { LV_STYLE_PAD_ROW,    { .num = 4 }                   },
    { LV_STYLE_ALIGN,      { .num = LV_ALIGN_CENTER }     },
};
STYLE_HANDLE(actionMenu);

static const lv_style_const_prop_t menuActionsProps[] = {
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }         },
    { LV_STYLE_HEIGHT,       { .num = LV_SIZE_CONTENT }     },
    { LV_STYLE_LAYOUT,       { .num = FlexLayout }          },
    { FlexFlow,              { .num = LV_FLEX_FLOW_COLUMN } },
    { LV_STYLE_PAD_ROW,      { .num = 4 }                   },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }                   },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }                   },
    { LV_STYLE_PAD_TOP,      { .num = 0 }                   },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }                   },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }                   },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }       },
};
STYLE_HANDLE(menuActions);

static const lv_style_const_prop_t confirmationActionsProps[] = {
    { FlexFlow,            { .num = LV_FLEX_FLOW_ROW } },
    { LV_STYLE_PAD_COLUMN, { .num = 8 }                },
};
STYLE_HANDLE(confirmationActions);

static const lv_style_const_prop_t menuActionProps[] = {
    { LV_STYLE_WIDTH,  { .num = LV_PCT(100) } },
    { LV_STYLE_HEIGHT, { .num = 24 }          },
};
STYLE_HANDLE(menuAction);

static const lv_style_const_prop_t confirmationActionProps[] = {
    { LV_STYLE_WIDTH,  { .num = 0 }  },
    { FlexGrow,        { .num = 1 }  },
    { LV_STYLE_HEIGHT, { .num = 32 } },
};
STYLE_HANDLE(confirmationAction);

static const lv_style_const_prop_t messageBubbleProps[] = {
    { LV_STYLE_WIDTH, { .num = LV_PCT(75) } },
};
STYLE_HANDLE(messageBubble);

static const lv_style_const_prop_t readMessageActionProps[] = {
    { LV_STYLE_WIDTH,  { .num = 78 } },
    { LV_STYLE_HEIGHT, { .num = 28 } },
};
STYLE_HANDLE(readMessageAction);

static const lv_style_const_prop_t audioDetailProps[] = {
    { LV_STYLE_Y, { .num = 18 } },
};
STYLE_HANDLE(audioDetail);

static const lv_style_const_prop_t identityAvatarProps[] = {
    { LV_STYLE_WIDTH,  { .num = 64 }               },
    { LV_STYLE_HEIGHT, { .num = 64 }               },
    { LV_STYLE_X,      { .num = 11 }               },
    { LV_STYLE_Y,      { .num = 16 }               },
    { LV_STYLE_RADIUS, { .num = LV_RADIUS_CIRCLE } },
};
STYLE_HANDLE(identityAvatar);

static const lv_style_const_prop_t contactAvatarProps[] = {
    { LV_STYLE_WIDTH,  { .num = 32 }               },
    { LV_STYLE_HEIGHT, { .num = 32 }               },
    { LV_STYLE_X,      { .num = 8 }                },
    { LV_STYLE_Y,      { .num = 3 }                },
    { LV_STYLE_RADIUS, { .num = LV_RADIUS_CIRCLE } },
};
STYLE_HANDLE(contactAvatar);

static const lv_style_const_prop_t conversationAvatarProps[] = {
    { LV_STYLE_WIDTH,  { .num = 32 }               },
    { LV_STYLE_HEIGHT, { .num = 32 }               },
    { LV_STYLE_X,      { .num = 12 }               },
    { LV_STYLE_Y,      { .num = 28 }               },
    { LV_STYLE_RADIUS, { .num = LV_RADIUS_CIRCLE } },
};
STYLE_HANDLE(conversationAvatar);

static const lv_style_const_prop_t avatarCanvasProps[] = {
    { LV_STYLE_WIDTH,        { .num = LV_PCT(100) }     },
    { LV_STYLE_HEIGHT,       { .num = LV_PCT(100) }     },
    { LV_STYLE_ALIGN,        { .num = LV_ALIGN_CENTER } },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_TRANSP }   },
    { LV_STYLE_BORDER_WIDTH, { .num = 0 }               },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }               },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }               },
    { LV_STYLE_PAD_TOP,      { .num = 0 }               },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }               },
};
STYLE_HANDLE(avatarCanvas);

static const lv_style_const_prop_t peerDot0Props[] = {
    { LV_STYLE_X, { .num = 104 } },
    { LV_STYLE_Y, { .num = 14 }  },
};
STYLE_HANDLE(peerDot0);

static const lv_style_const_prop_t messageDot0Props[] = {
    { LV_STYLE_X, { .num = 117 } },
    { LV_STYLE_Y, { .num = 20 }  },
};
STYLE_HANDLE(messageDot0);

static const lv_style_const_prop_t peerAction0Props[] = {
    { LV_STYLE_X, { .num = 12 } },
    { LV_STYLE_Y, { .num = 90 } },
};
STYLE_HANDLE(peerAction0);

static const lv_style_const_prop_t peerDot1Props[] = {
    { LV_STYLE_X, { .num = 117 } },
    { LV_STYLE_Y, { .num = 14 }  },
};
STYLE_HANDLE(peerDot1);

static const lv_style_const_prop_t messageDot1Props[] = {
    { LV_STYLE_X, { .num = 124 } },
    { LV_STYLE_Y, { .num = 20 }  },
};
STYLE_HANDLE(messageDot1);

static const lv_style_const_prop_t peerAction1Props[] = {
    { LV_STYLE_X, { .num = 12 }  },
    { LV_STYLE_Y, { .num = 142 } },
};
STYLE_HANDLE(peerAction1);

static const lv_style_const_prop_t peerDot2Props[] = {
    { LV_STYLE_X, { .num = 130 } },
    { LV_STYLE_Y, { .num = 14 }  },
};
STYLE_HANDLE(peerDot2);

static const lv_style_const_prop_t messageDot2Props[] = {
    { LV_STYLE_X, { .num = 131 } },
    { LV_STYLE_Y, { .num = 20 }  },
};
STYLE_HANDLE(messageDot2);

static const lv_style_const_prop_t peerAction2Props[] = {
    { LV_STYLE_X, { .num = 12 }  },
    { LV_STYLE_Y, { .num = 194 } },
};
STYLE_HANDLE(peerAction2);

static const lv_style_const_prop_t contactCodeProps[] = {
    { LV_STYLE_WIDTH,  { .num = 416 } },
    { LV_STYLE_HEIGHT, { .num = 416 } },
};
STYLE_HANDLE(contactCode);

static const lv_style_const_prop_t legacyContactCodeProps[] = {
    { LV_STYLE_WIDTH,  { .num = 392 } },
    { LV_STYLE_HEIGHT, { .num = 392 } },
};
STYLE_HANDLE(legacyContactCode);

static const lv_style_const_prop_t peerDot3Props[] = {
    { LV_STYLE_X, { .num = 143 } },
    { LV_STYLE_Y, { .num = 14 }  },
};
STYLE_HANDLE(peerDot3);

static const lv_style_const_prop_t tabBarSizeProps[] = {
    { LV_STYLE_HEIGHT, { .num = 48 } },
};
STYLE_HANDLE(tabBarSize);

static const lv_style_const_prop_t choiceButtonProps[] = {
    { LV_STYLE_WIDTH,        { .num = 126 }          },
    { LV_STYLE_HEIGHT,       { .num = 34 }           },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }            },
    { LV_STYLE_RADIUS,       { .num = 4 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
};
STYLE_HANDLE(choiceButton);

static const lv_style_const_prop_t stepButtonProps[] = {
    { LV_STYLE_WIDTH,  { .num = 74 } },
    { LV_STYLE_HEIGHT, { .num = 34 } },
};
STYLE_HANDLE(stepButton);

static const lv_style_const_prop_t summaryTileProps[] = {
    { LV_STYLE_WIDTH,        { .num = 253 }          },
    { LV_STYLE_HEIGHT,       { .num = 72 }           },
    { LV_STYLE_BG_OPA,       { .num = LV_OPA_COVER } },
    { LV_STYLE_BORDER_WIDTH, { .num = 1 }            },
    { LV_STYLE_RADIUS,       { .num = 4 }            },
    { LV_STYLE_PAD_LEFT,     { .num = 0 }            },
    { LV_STYLE_PAD_RIGHT,    { .num = 0 }            },
    { LV_STYLE_PAD_TOP,      { .num = 0 }            },
    { LV_STYLE_PAD_BOTTOM,   { .num = 0 }            },
    { LV_STYLE_SHADOW_WIDTH, { .num = 0 }            },
};
STYLE_HANDLE(summaryTile);

#undef STYLE_HANDLE

} // namespace Style
