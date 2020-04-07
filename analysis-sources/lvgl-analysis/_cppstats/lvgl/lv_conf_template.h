#if 0 
#if !defined(LV_CONF_H)
#define LV_CONF_H
#include <stdint.h>
#define LV_HOR_RES_MAX (480)
#define LV_VER_RES_MAX (320)
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_COLOR_SCREEN_TRANSP 0
#define LV_COLOR_TRANSP LV_COLOR_LIME 
#define LV_INDEXED_CHROMA 1
#define LV_ANTIALIAS 1
#define LV_DISP_DEF_REFR_PERIOD 30 
#define LV_DPI 100 
typedef int16_t lv_coord_t;
#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 0
#define LV_MEM_SIZE (32U * 1024U)
#define LV_MEM_ATTR
#define LV_MEM_ADR 0
#define LV_MEM_AUTO_DEFRAG 1
#else 
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h> 
#define LV_MEM_CUSTOM_ALLOC malloc 
#define LV_MEM_CUSTOM_FREE free 
#endif 
#define LV_ENABLE_GC 0
#if LV_ENABLE_GC != 0
#define LV_GC_INCLUDE "gc.h" 
#define LV_MEM_CUSTOM_REALLOC your_realloc 
#define LV_MEM_CUSTOM_GET_SIZE your_mem_get_size 
#endif 
#define LV_INDEV_DEF_READ_PERIOD 30
#define LV_INDEV_DEF_DRAG_LIMIT 10
#define LV_INDEV_DEF_DRAG_THROW 20
#define LV_INDEV_DEF_LONG_PRESS_TIME 400
#define LV_INDEV_DEF_LONG_PRESS_REP_TIME 100
#define LV_USE_ANIMATION 1
#if LV_USE_ANIMATION
typedef void * lv_anim_user_data_t;
#endif
#define LV_USE_SHADOW 1
#define LV_USE_GROUP 1
#if LV_USE_GROUP
typedef void * lv_group_user_data_t;
#endif 
#define LV_USE_GPU 1
#define LV_USE_FILESYSTEM 1
#if LV_USE_FILESYSTEM
typedef void * lv_fs_drv_user_data_t;
#endif
#define LV_USE_USER_DATA 0
#define LV_IMG_CF_INDEXED 1
#define LV_IMG_CF_ALPHA 1
#define LV_IMG_CACHE_DEF_SIZE 1
typedef void * lv_img_decoder_user_data_t;
#define LV_ATTRIBUTE_TICK_INC
#define LV_ATTRIBUTE_TASK_HANDLER
#define LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_LARGE_CONST
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning
#define LV_TICK_CUSTOM 0
#if LV_TICK_CUSTOM == 1
#define LV_TICK_CUSTOM_INCLUDE "something.h" 
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis()) 
#endif 
typedef void * lv_disp_drv_user_data_t; 
typedef void * lv_indev_drv_user_data_t; 
#define LV_USE_LOG 0
#if LV_USE_LOG
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_LOG_PRINTF 0
#endif 
#define LV_USE_DEBUG 1
#if LV_USE_DEBUG
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MEM 1
#define LV_USE_ASSERT_STR 0
#define LV_USE_ASSERT_OBJ 0
#define LV_USE_ASSERT_STYLE 1
#endif 
#define LV_THEME_LIVE_UPDATE 0 
#define LV_USE_THEME_TEMPL 0 
#define LV_USE_THEME_DEFAULT 0 
#define LV_USE_THEME_ALIEN 0 
#define LV_USE_THEME_NIGHT 0 
#define LV_USE_THEME_MONO 0 
#define LV_USE_THEME_MATERIAL 0 
#define LV_USE_THEME_ZEN 0 
#define LV_USE_THEME_NEMO 0 
#define LV_FONT_ROBOTO_12 0
#define LV_FONT_ROBOTO_16 1
#define LV_FONT_ROBOTO_22 0
#define LV_FONT_ROBOTO_28 0
#define LV_FONT_ROBOTO_12_SUBPX 1
#define LV_FONT_ROBOTO_28_COMPRESSED 1 
#define LV_FONT_UNSCII_8 0
#define LV_FONT_CUSTOM_DECLARE
#define LV_FONT_DEFAULT &lv_font_roboto_16
#define LV_FONT_FMT_TXT_LARGE 0
#define LV_FONT_SUBPX_BGR 0
typedef void * lv_font_user_data_t;
#define LV_TXT_ENC LV_TXT_ENC_UTF8
#define LV_TXT_BREAK_CHARS " ,.;:-_"
#define LV_TXT_LINE_BREAK_LONG_LEN 12
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN 3
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3
#define LV_TXT_COLOR_CMD "#"
#define LV_USE_BIDI 0
#if LV_USE_BIDI
#define LV_BIDI_BASE_DIR_DEF LV_BIDI_DIR_AUTO
#endif
#define LV_SPRINTF_CUSTOM 0
#if LV_SPRINTF_CUSTOM
#define LV_SPRINTF_INCLUDE <stdio.h>
#define lv_snprintf snprintf
#define lv_vsnprintf vsnprintf
#endif 
typedef void * lv_obj_user_data_t;
#define LV_USE_OBJ_REALIGN 1
#define LV_USE_EXT_CLICK_AREA LV_EXT_CLICK_AREA_OFF
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#if LV_USE_BTN != 0
#define LV_BTN_INK_EFFECT 0
#endif
#define LV_USE_BTNM 1
#define LV_USE_CALENDAR 1
#define LV_USE_CANVAS 1
#define LV_USE_CB 1
#define LV_USE_CHART 1
#if LV_USE_CHART
#define LV_CHART_AXIS_TICK_LABEL_MAX_LEN 20
#endif
#define LV_USE_CONT 1
#define LV_USE_CPICKER 1
#define LV_USE_DDLIST 1
#if LV_USE_DDLIST != 0
#define LV_DDLIST_DEF_ANIM_TIME 200
#endif
#define LV_USE_GAUGE 1
#define LV_USE_IMG 1
#define LV_USE_IMGBTN 1
#if LV_USE_IMGBTN
#define LV_IMGBTN_TILED 0
#endif
#define LV_USE_KB 1
#define LV_USE_LABEL 1
#if LV_USE_LABEL != 0
#define LV_LABEL_DEF_SCROLL_SPEED 25
#define LV_LABEL_WAIT_CHAR_COUNT 3
#define LV_LABEL_TEXT_SEL 0
#define LV_LABEL_LONG_TXT_HINT 0
#endif
#define LV_USE_LED 1
#define LV_USE_LINE 1
#define LV_USE_LIST 1
#if LV_USE_LIST != 0
#define LV_LIST_DEF_ANIM_TIME 100
#endif
#define LV_USE_LMETER 1
#define LV_USE_MBOX 1
#define LV_USE_PAGE 1
#if LV_USE_PAGE != 0
#define LV_PAGE_DEF_ANIM_TIME 400
#endif
#define LV_USE_PRELOAD 1
#if LV_USE_PRELOAD != 0
#define LV_PRELOAD_DEF_ARC_LENGTH 60 
#define LV_PRELOAD_DEF_SPIN_TIME 1000 
#define LV_PRELOAD_DEF_ANIM LV_PRELOAD_TYPE_SPINNING_ARC
#endif
#define LV_USE_ROLLER 1
#if LV_USE_ROLLER != 0
#define LV_ROLLER_DEF_ANIM_TIME 200
#define LV_ROLLER_INF_PAGES 7
#endif
#define LV_USE_SLIDER 1
#define LV_USE_SPINBOX 1
#define LV_USE_SW 1
#define LV_USE_TA 1
#if LV_USE_TA != 0
#define LV_TA_DEF_CURSOR_BLINK_TIME 400 
#define LV_TA_DEF_PWD_SHOW_TIME 1500 
#endif
#define LV_USE_TABLE 1
#if LV_USE_TABLE
#define LV_TABLE_COL_MAX 12
#endif
#define LV_USE_TABVIEW 1
#if LV_USE_TABVIEW != 0
#define LV_TABVIEW_DEF_ANIM_TIME 300
#endif
#define LV_USE_TILEVIEW 1
#if LV_USE_TILEVIEW
#define LV_TILEVIEW_DEF_ANIM_TIME 300
#endif
#define LV_USE_WIN 1
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS) 
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "lvgl/src/lv_conf_checker.h"
#endif 
#endif 
