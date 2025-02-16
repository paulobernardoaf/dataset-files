#include <stdint.h>
#if !defined(LV_HOR_RES_MAX)
#define LV_HOR_RES_MAX (480)
#endif
#if !defined(LV_VER_RES_MAX)
#define LV_VER_RES_MAX (320)
#endif
#if !defined(LV_COLOR_DEPTH)
#define LV_COLOR_DEPTH 16
#endif
#if !defined(LV_COLOR_16_SWAP)
#define LV_COLOR_16_SWAP 0
#endif
#if !defined(LV_COLOR_SCREEN_TRANSP)
#define LV_COLOR_SCREEN_TRANSP 0
#endif
#if !defined(LV_COLOR_TRANSP)
#define LV_COLOR_TRANSP LV_COLOR_LIME 
#endif
#if !defined(LV_INDEXED_CHROMA)
#define LV_INDEXED_CHROMA 1
#endif
#if !defined(LV_ANTIALIAS)
#define LV_ANTIALIAS 1
#endif
#if !defined(LV_DISP_DEF_REFR_PERIOD)
#define LV_DISP_DEF_REFR_PERIOD 30 
#endif
#if !defined(LV_DPI)
#define LV_DPI 100 
#endif
#if !defined(LV_MEM_CUSTOM)
#define LV_MEM_CUSTOM 0
#endif
#if LV_MEM_CUSTOM == 0
#if !defined(LV_MEM_SIZE)
#define LV_MEM_SIZE (32U * 1024U)
#endif
#if !defined(LV_MEM_ATTR)
#define LV_MEM_ATTR
#endif
#if !defined(LV_MEM_ADR)
#define LV_MEM_ADR 0
#endif
#if !defined(LV_MEM_AUTO_DEFRAG)
#define LV_MEM_AUTO_DEFRAG 1
#endif
#else 
#if !defined(LV_MEM_CUSTOM_INCLUDE)
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h> 
#endif
#if !defined(LV_MEM_CUSTOM_ALLOC)
#define LV_MEM_CUSTOM_ALLOC malloc 
#endif
#if !defined(LV_MEM_CUSTOM_FREE)
#define LV_MEM_CUSTOM_FREE free 
#endif
#endif 
#if !defined(LV_ENABLE_GC)
#define LV_ENABLE_GC 0
#endif
#if LV_ENABLE_GC != 0
#if !defined(LV_GC_INCLUDE)
#define LV_GC_INCLUDE "gc.h" 
#endif
#if !defined(LV_MEM_CUSTOM_REALLOC)
#define LV_MEM_CUSTOM_REALLOC your_realloc 
#endif
#if !defined(LV_MEM_CUSTOM_GET_SIZE)
#define LV_MEM_CUSTOM_GET_SIZE your_mem_get_size 
#endif
#endif 
#if !defined(LV_INDEV_DEF_READ_PERIOD)
#define LV_INDEV_DEF_READ_PERIOD 30
#endif
#if !defined(LV_INDEV_DEF_DRAG_LIMIT)
#define LV_INDEV_DEF_DRAG_LIMIT 10
#endif
#if !defined(LV_INDEV_DEF_DRAG_THROW)
#define LV_INDEV_DEF_DRAG_THROW 20
#endif
#if !defined(LV_INDEV_DEF_LONG_PRESS_TIME)
#define LV_INDEV_DEF_LONG_PRESS_TIME 400
#endif
#if !defined(LV_INDEV_DEF_LONG_PRESS_REP_TIME)
#define LV_INDEV_DEF_LONG_PRESS_REP_TIME 100
#endif
#if !defined(LV_USE_ANIMATION)
#define LV_USE_ANIMATION 1
#endif
#if LV_USE_ANIMATION
#endif
#if !defined(LV_USE_SHADOW)
#define LV_USE_SHADOW 1
#endif
#if !defined(LV_USE_GROUP)
#define LV_USE_GROUP 1
#endif
#if LV_USE_GROUP
#endif 
#if !defined(LV_USE_GPU)
#define LV_USE_GPU 1
#endif
#if !defined(LV_USE_FILESYSTEM)
#define LV_USE_FILESYSTEM 1
#endif
#if LV_USE_FILESYSTEM
#endif
#if !defined(LV_USE_USER_DATA)
#define LV_USE_USER_DATA 0
#endif
#if !defined(LV_IMG_CF_INDEXED)
#define LV_IMG_CF_INDEXED 1
#endif
#if !defined(LV_IMG_CF_ALPHA)
#define LV_IMG_CF_ALPHA 1
#endif
#if !defined(LV_IMG_CACHE_DEF_SIZE)
#define LV_IMG_CACHE_DEF_SIZE 1
#endif
#if !defined(LV_ATTRIBUTE_TICK_INC)
#define LV_ATTRIBUTE_TICK_INC
#endif
#if !defined(LV_ATTRIBUTE_TASK_HANDLER)
#define LV_ATTRIBUTE_TASK_HANDLER
#endif
#if !defined(LV_ATTRIBUTE_MEM_ALIGN)
#define LV_ATTRIBUTE_MEM_ALIGN
#endif
#if !defined(LV_ATTRIBUTE_LARGE_CONST)
#define LV_ATTRIBUTE_LARGE_CONST
#endif
#if !defined(LV_EXPORT_CONST_INT)
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning
#endif
#if !defined(LV_TICK_CUSTOM)
#define LV_TICK_CUSTOM 0
#endif
#if LV_TICK_CUSTOM == 1
#if !defined(LV_TICK_CUSTOM_INCLUDE)
#define LV_TICK_CUSTOM_INCLUDE "something.h" 
#endif
#if !defined(LV_TICK_CUSTOM_SYS_TIME_EXPR)
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis()) 
#endif
#endif 
#if !defined(LV_USE_LOG)
#define LV_USE_LOG 0
#endif
#if LV_USE_LOG
#if !defined(LV_LOG_LEVEL)
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#endif
#if !defined(LV_LOG_PRINTF)
#define LV_LOG_PRINTF 0
#endif
#endif 
#if !defined(LV_USE_DEBUG)
#define LV_USE_DEBUG 1
#endif
#if LV_USE_DEBUG
#if !defined(LV_USE_ASSERT_NULL)
#define LV_USE_ASSERT_NULL 1
#endif
#if !defined(LV_USE_ASSERT_MEM)
#define LV_USE_ASSERT_MEM 1
#endif
#if !defined(LV_USE_ASSERT_STR)
#define LV_USE_ASSERT_STR 0
#endif
#if !defined(LV_USE_ASSERT_OBJ)
#define LV_USE_ASSERT_OBJ 0
#endif
#if !defined(LV_USE_ASSERT_STYLE)
#define LV_USE_ASSERT_STYLE 1
#endif
#endif 
#if !defined(LV_THEME_LIVE_UPDATE)
#define LV_THEME_LIVE_UPDATE 0 
#endif
#if !defined(LV_USE_THEME_TEMPL)
#define LV_USE_THEME_TEMPL 0 
#endif
#if !defined(LV_USE_THEME_DEFAULT)
#define LV_USE_THEME_DEFAULT 0 
#endif
#if !defined(LV_USE_THEME_ALIEN)
#define LV_USE_THEME_ALIEN 0 
#endif
#if !defined(LV_USE_THEME_NIGHT)
#define LV_USE_THEME_NIGHT 0 
#endif
#if !defined(LV_USE_THEME_MONO)
#define LV_USE_THEME_MONO 0 
#endif
#if !defined(LV_USE_THEME_MATERIAL)
#define LV_USE_THEME_MATERIAL 0 
#endif
#if !defined(LV_USE_THEME_ZEN)
#define LV_USE_THEME_ZEN 0 
#endif
#if !defined(LV_USE_THEME_NEMO)
#define LV_USE_THEME_NEMO 0 
#endif
#if !defined(LV_FONT_ROBOTO_12)
#define LV_FONT_ROBOTO_12 0
#endif
#if !defined(LV_FONT_ROBOTO_16)
#define LV_FONT_ROBOTO_16 1
#endif
#if !defined(LV_FONT_ROBOTO_22)
#define LV_FONT_ROBOTO_22 0
#endif
#if !defined(LV_FONT_ROBOTO_28)
#define LV_FONT_ROBOTO_28 0
#endif
#if !defined(LV_FONT_ROBOTO_12_SUBPX)
#define LV_FONT_ROBOTO_12_SUBPX 1
#endif
#if !defined(LV_FONT_ROBOTO_28_COMPRESSED)
#define LV_FONT_ROBOTO_28_COMPRESSED 1 
#endif
#if !defined(LV_FONT_UNSCII_8)
#define LV_FONT_UNSCII_8 0
#endif
#if !defined(LV_FONT_CUSTOM_DECLARE)
#define LV_FONT_CUSTOM_DECLARE
#endif
#if !defined(LV_FONT_DEFAULT)
#define LV_FONT_DEFAULT &lv_font_roboto_16
#endif
#if !defined(LV_FONT_FMT_TXT_LARGE)
#define LV_FONT_FMT_TXT_LARGE 0
#endif
#if !defined(LV_FONT_SUBPX_BGR)
#define LV_FONT_SUBPX_BGR 0
#endif
#if !defined(LV_TXT_ENC)
#define LV_TXT_ENC LV_TXT_ENC_UTF8
#endif
#if !defined(LV_TXT_BREAK_CHARS)
#define LV_TXT_BREAK_CHARS " ,.;:-_"
#endif
#if !defined(LV_TXT_LINE_BREAK_LONG_LEN)
#define LV_TXT_LINE_BREAK_LONG_LEN 12
#endif
#if !defined(LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN)
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN 3
#endif
#if !defined(LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN)
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3
#endif
#if !defined(LV_TXT_COLOR_CMD)
#define LV_TXT_COLOR_CMD "#"
#endif
#if !defined(LV_USE_BIDI)
#define LV_USE_BIDI 0
#endif
#if LV_USE_BIDI
#if !defined(LV_BIDI_BASE_DIR_DEF)
#define LV_BIDI_BASE_DIR_DEF LV_BIDI_DIR_AUTO
#endif
#endif
#if !defined(LV_SPRINTF_CUSTOM)
#define LV_SPRINTF_CUSTOM 0
#endif
#if LV_SPRINTF_CUSTOM
#if !defined(LV_SPRINTF_INCLUDE)
#define LV_SPRINTF_INCLUDE <stdio.h>
#endif
#if !defined(lv_snprintf)
#define lv_snprintf snprintf
#endif
#if !defined(lv_vsnprintf)
#define lv_vsnprintf vsnprintf
#endif
#endif 
#if !defined(LV_USE_OBJ_REALIGN)
#define LV_USE_OBJ_REALIGN 1
#endif
#if !defined(LV_USE_EXT_CLICK_AREA)
#define LV_USE_EXT_CLICK_AREA LV_EXT_CLICK_AREA_OFF
#endif
#if !defined(LV_USE_ARC)
#define LV_USE_ARC 1
#endif
#if !defined(LV_USE_BAR)
#define LV_USE_BAR 1
#endif
#if !defined(LV_USE_BTN)
#define LV_USE_BTN 1
#endif
#if LV_USE_BTN != 0
#if !defined(LV_BTN_INK_EFFECT)
#define LV_BTN_INK_EFFECT 0
#endif
#endif
#if !defined(LV_USE_BTNM)
#define LV_USE_BTNM 1
#endif
#if !defined(LV_USE_CALENDAR)
#define LV_USE_CALENDAR 1
#endif
#if !defined(LV_USE_CANVAS)
#define LV_USE_CANVAS 1
#endif
#if !defined(LV_USE_CB)
#define LV_USE_CB 1
#endif
#if !defined(LV_USE_CHART)
#define LV_USE_CHART 1
#endif
#if LV_USE_CHART
#if !defined(LV_CHART_AXIS_TICK_LABEL_MAX_LEN)
#define LV_CHART_AXIS_TICK_LABEL_MAX_LEN 20
#endif
#endif
#if !defined(LV_USE_CONT)
#define LV_USE_CONT 1
#endif
#if !defined(LV_USE_CPICKER)
#define LV_USE_CPICKER 1
#endif
#if !defined(LV_USE_DDLIST)
#define LV_USE_DDLIST 1
#endif
#if LV_USE_DDLIST != 0
#if !defined(LV_DDLIST_DEF_ANIM_TIME)
#define LV_DDLIST_DEF_ANIM_TIME 200
#endif
#endif
#if !defined(LV_USE_GAUGE)
#define LV_USE_GAUGE 1
#endif
#if !defined(LV_USE_IMG)
#define LV_USE_IMG 1
#endif
#if !defined(LV_USE_IMGBTN)
#define LV_USE_IMGBTN 1
#endif
#if LV_USE_IMGBTN
#if !defined(LV_IMGBTN_TILED)
#define LV_IMGBTN_TILED 0
#endif
#endif
#if !defined(LV_USE_KB)
#define LV_USE_KB 1
#endif
#if !defined(LV_USE_LABEL)
#define LV_USE_LABEL 1
#endif
#if LV_USE_LABEL != 0
#if !defined(LV_LABEL_DEF_SCROLL_SPEED)
#define LV_LABEL_DEF_SCROLL_SPEED 25
#endif
#if !defined(LV_LABEL_WAIT_CHAR_COUNT)
#define LV_LABEL_WAIT_CHAR_COUNT 3
#endif
#if !defined(LV_LABEL_TEXT_SEL)
#define LV_LABEL_TEXT_SEL 0
#endif
#if !defined(LV_LABEL_LONG_TXT_HINT)
#define LV_LABEL_LONG_TXT_HINT 0
#endif
#endif
#if !defined(LV_USE_LED)
#define LV_USE_LED 1
#endif
#if !defined(LV_USE_LINE)
#define LV_USE_LINE 1
#endif
#if !defined(LV_USE_LIST)
#define LV_USE_LIST 1
#endif
#if LV_USE_LIST != 0
#if !defined(LV_LIST_DEF_ANIM_TIME)
#define LV_LIST_DEF_ANIM_TIME 100
#endif
#endif
#if !defined(LV_USE_LMETER)
#define LV_USE_LMETER 1
#endif
#if !defined(LV_USE_MBOX)
#define LV_USE_MBOX 1
#endif
#if !defined(LV_USE_PAGE)
#define LV_USE_PAGE 1
#endif
#if LV_USE_PAGE != 0
#if !defined(LV_PAGE_DEF_ANIM_TIME)
#define LV_PAGE_DEF_ANIM_TIME 400
#endif
#endif
#if !defined(LV_USE_PRELOAD)
#define LV_USE_PRELOAD 1
#endif
#if LV_USE_PRELOAD != 0
#if !defined(LV_PRELOAD_DEF_ARC_LENGTH)
#define LV_PRELOAD_DEF_ARC_LENGTH 60 
#endif
#if !defined(LV_PRELOAD_DEF_SPIN_TIME)
#define LV_PRELOAD_DEF_SPIN_TIME 1000 
#endif
#if !defined(LV_PRELOAD_DEF_ANIM)
#define LV_PRELOAD_DEF_ANIM LV_PRELOAD_TYPE_SPINNING_ARC
#endif
#endif
#if !defined(LV_USE_ROLLER)
#define LV_USE_ROLLER 1
#endif
#if LV_USE_ROLLER != 0
#if !defined(LV_ROLLER_DEF_ANIM_TIME)
#define LV_ROLLER_DEF_ANIM_TIME 200
#endif
#if !defined(LV_ROLLER_INF_PAGES)
#define LV_ROLLER_INF_PAGES 7
#endif
#endif
#if !defined(LV_USE_SLIDER)
#define LV_USE_SLIDER 1
#endif
#if !defined(LV_USE_SPINBOX)
#define LV_USE_SPINBOX 1
#endif
#if !defined(LV_USE_SW)
#define LV_USE_SW 1
#endif
#if !defined(LV_USE_TA)
#define LV_USE_TA 1
#endif
#if LV_USE_TA != 0
#if !defined(LV_TA_DEF_CURSOR_BLINK_TIME)
#define LV_TA_DEF_CURSOR_BLINK_TIME 400 
#endif
#if !defined(LV_TA_DEF_PWD_SHOW_TIME)
#define LV_TA_DEF_PWD_SHOW_TIME 1500 
#endif
#endif
#if !defined(LV_USE_TABLE)
#define LV_USE_TABLE 1
#endif
#if LV_USE_TABLE
#if !defined(LV_TABLE_COL_MAX)
#define LV_TABLE_COL_MAX 12
#endif
#endif
#if !defined(LV_USE_TABVIEW)
#define LV_USE_TABVIEW 1
#endif
#if LV_USE_TABVIEW != 0
#if !defined(LV_TABVIEW_DEF_ANIM_TIME)
#define LV_TABVIEW_DEF_ANIM_TIME 300
#endif
#endif
#if !defined(LV_USE_TILEVIEW)
#define LV_USE_TILEVIEW 1
#endif
#if LV_USE_TILEVIEW
#if !defined(LV_TILEVIEW_DEF_ANIM_TIME)
#define LV_TILEVIEW_DEF_ANIM_TIME 300
#endif
#endif
#if !defined(LV_USE_WIN)
#define LV_USE_WIN 1
#endif
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS) 
#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
