#include "../../lvgl.h"







#if !defined(LV_FONT_UNSCII_8)
#define LV_FONT_UNSCII_8 1
#endif

#if LV_FONT_UNSCII_8






static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {



0xf2,


0x99, 0x90,


0x49, 0x2f, 0xd2, 0xfd, 0x24, 0x80,


0x23, 0xe8, 0xe2, 0xf8, 0x80,


0xc7, 0x21, 0x8, 0x4e, 0x30,


0x62, 0x49, 0x18, 0x96, 0x27, 0x40,


0x2a, 0x0,


0x2a, 0x48, 0x88,


0x88, 0x92, 0xa0,


0x25, 0x5c, 0x47, 0x54, 0x80,


0x21, 0x3e, 0x42, 0x0,


0x58,


0xf8,


0x80,


0x2, 0x8, 0x20, 0x82, 0x8, 0x20, 0x0,


0x74, 0x67, 0x5c, 0xc5, 0xc0,


0x23, 0x28, 0x42, 0x13, 0xe0,


0x74, 0x42, 0x26, 0x43, 0xe0,


0x74, 0x42, 0x60, 0xc5, 0xc0,


0x11, 0x95, 0x2f, 0x88, 0x40,


0xfc, 0x3c, 0x10, 0xc5, 0xc0,


0x3a, 0x21, 0xe8, 0xc5, 0xc0,


0xf8, 0x44, 0x44, 0x21, 0x0,


0x74, 0x62, 0xe8, 0xc5, 0xc0,


0x74, 0x62, 0xf0, 0x8b, 0x80,


0x90,


0x41, 0x60,


0x12, 0x48, 0x42, 0x10,


0xf8, 0x3e,


0x84, 0x21, 0x24, 0x80,


0x7a, 0x10, 0x84, 0x10, 0x1, 0x0,


0x7a, 0x19, 0x6b, 0x9a, 0x7, 0x80,


0x31, 0x28, 0x7f, 0x86, 0x18, 0x40,


0xfa, 0x18, 0x7e, 0x86, 0x1f, 0x80,


0x7a, 0x18, 0x20, 0x82, 0x17, 0x80,


0xf2, 0x28, 0x61, 0x86, 0x2f, 0x0,


0xfe, 0x8, 0x3c, 0x82, 0xf, 0xc0,


0xfe, 0x8, 0x3c, 0x82, 0x8, 0x0,


0x7a, 0x18, 0x27, 0x86, 0x17, 0x80,


0x86, 0x18, 0x7f, 0x86, 0x18, 0x40,


0xe9, 0x24, 0xb8,


0x8, 0x42, 0x10, 0xc5, 0xc0,


0x86, 0x29, 0x38, 0x92, 0x28, 0x40,


0x82, 0x8, 0x20, 0x82, 0xf, 0xc0,


0x87, 0x3b, 0x61, 0x86, 0x18, 0x40,


0x87, 0x1a, 0x65, 0x8e, 0x18, 0x40,


0x7a, 0x18, 0x61, 0x86, 0x17, 0x80,


0xfa, 0x18, 0x7e, 0x82, 0x8, 0x0,


0x7a, 0x18, 0x61, 0x96, 0x27, 0x40,


0xfa, 0x18, 0x7e, 0x92, 0x28, 0x40,


0x7a, 0x18, 0x1e, 0x6, 0x17, 0x80,


0xf9, 0x8, 0x42, 0x10, 0x80,


0x86, 0x18, 0x61, 0x86, 0x17, 0x80,


0x86, 0x18, 0x61, 0x85, 0x23, 0x0,


0x86, 0x18, 0x61, 0xb7, 0x38, 0x40,


0x86, 0x14, 0x8c, 0x4a, 0x18, 0x40,


0x8c, 0x62, 0xe2, 0x10, 0x80,


0xf8, 0x44, 0x44, 0x43, 0xe0,


0xf2, 0x49, 0x38,


0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,


0xe4, 0x92, 0x78,


0x22, 0xa2,


0xf8,


0x88, 0x80,


0x70, 0x5f, 0x17, 0x80,


0x84, 0x3d, 0x18, 0xc7, 0xc0,


0x74, 0x61, 0x17, 0x0,


0x8, 0x5f, 0x18, 0xc5, 0xe0,


0x74, 0x7f, 0x7, 0x0,


0x18, 0x92, 0x3e, 0x20, 0x82, 0x0,


0x7c, 0x62, 0xf0, 0xb8,


0x84, 0x3d, 0x18, 0xc6, 0x20,


0x43, 0x24, 0xb8,


0x10, 0x31, 0x11, 0x96,


0x84, 0x23, 0x2e, 0x4a, 0x20,


0xc9, 0x24, 0xb8,


0xd5, 0x6b, 0x5a, 0x80,


0xf4, 0x63, 0x18, 0x80,


0x74, 0x63, 0x17, 0x0,


0xf4, 0x63, 0xe8, 0x40,


0x7c, 0x62, 0xf0, 0x84,


0xbe, 0x21, 0x8, 0x0,


0x7c, 0x1c, 0x1f, 0x0,


0x42, 0x3c, 0x84, 0x24, 0xc0,


0x8c, 0x63, 0x17, 0x0,


0x8c, 0x62, 0xa2, 0x0,


0x8d, 0x6b, 0x55, 0x0,


0x8a, 0x88, 0xa8, 0x80,


0x8c, 0x62, 0xf0, 0xb8,


0xf8, 0x88, 0x8f, 0x80,


0x34, 0x48, 0x44, 0x30,


0xff,


0xc2, 0x21, 0x22, 0xc0,


0x45, 0x44,


0xc1, 0x42, 0xbd, 0x2c, 0x40, 0x81, 0x0
};






static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
{.bitmap_index = 0, .adv_w = 0, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0} ,
{.bitmap_index = 0, .adv_w = 128, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0},
{.bitmap_index = 0, .adv_w = 128, .box_h = 7, .box_w = 1, .ofs_x = 3, .ofs_y = -1},
{.bitmap_index = 1, .adv_w = 128, .box_h = 3, .box_w = 4, .ofs_x = 2, .ofs_y = 3},
{.bitmap_index = 3, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 9, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 14, .adv_w = 128, .box_h = 6, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 19, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 25, .adv_w = 128, .box_h = 3, .box_w = 3, .ofs_x = 2, .ofs_y = 3},
{.bitmap_index = 27, .adv_w = 128, .box_h = 7, .box_w = 3, .ofs_x = 2, .ofs_y = -1},
{.bitmap_index = 30, .adv_w = 128, .box_h = 7, .box_w = 3, .ofs_x = 2, .ofs_y = -1},
{.bitmap_index = 33, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 38, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = 0},
{.bitmap_index = 42, .adv_w = 128, .box_h = 3, .box_w = 2, .ofs_x = 3, .ofs_y = -2},
{.bitmap_index = 43, .adv_w = 128, .box_h = 1, .box_w = 5, .ofs_x = 1, .ofs_y = 1},
{.bitmap_index = 44, .adv_w = 128, .box_h = 1, .box_w = 1, .ofs_x = 3, .ofs_y = -1},
{.bitmap_index = 45, .adv_w = 128, .box_h = 7, .box_w = 7, .ofs_x = 0, .ofs_y = -1},
{.bitmap_index = 52, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 57, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 62, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 67, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 72, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 77, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 82, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 87, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 92, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 97, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 102, .adv_w = 128, .box_h = 4, .box_w = 1, .ofs_x = 3, .ofs_y = 0},
{.bitmap_index = 103, .adv_w = 128, .box_h = 6, .box_w = 2, .ofs_x = 2, .ofs_y = -2},
{.bitmap_index = 105, .adv_w = 128, .box_h = 7, .box_w = 4, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 109, .adv_w = 128, .box_h = 3, .box_w = 5, .ofs_x = 1, .ofs_y = 1},
{.bitmap_index = 111, .adv_w = 128, .box_h = 7, .box_w = 4, .ofs_x = 2, .ofs_y = -1},
{.bitmap_index = 115, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 121, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 127, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 133, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 139, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 145, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 151, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 157, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 163, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 169, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 175, .adv_w = 128, .box_h = 7, .box_w = 3, .ofs_x = 2, .ofs_y = -1},
{.bitmap_index = 178, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 183, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 189, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 195, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 201, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 207, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 213, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 219, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 225, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 231, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 237, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 242, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 248, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 254, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 260, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 266, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 271, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 276, .adv_w = 128, .box_h = 7, .box_w = 3, .ofs_x = 2, .ofs_y = -1},
{.bitmap_index = 279, .adv_w = 128, .box_h = 7, .box_w = 7, .ofs_x = 0, .ofs_y = -1},
{.bitmap_index = 286, .adv_w = 128, .box_h = 7, .box_w = 3, .ofs_x = 2, .ofs_y = -1},
{.bitmap_index = 289, .adv_w = 128, .box_h = 3, .box_w = 5, .ofs_x = 1, .ofs_y = 3},
{.bitmap_index = 291, .adv_w = 128, .box_h = 1, .box_w = 5, .ofs_x = 1, .ofs_y = -2},
{.bitmap_index = 292, .adv_w = 128, .box_h = 3, .box_w = 3, .ofs_x = 2, .ofs_y = 3},
{.bitmap_index = 294, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 298, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 303, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 307, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 312, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 316, .adv_w = 128, .box_h = 7, .box_w = 6, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 322, .adv_w = 128, .box_h = 6, .box_w = 5, .ofs_x = 1, .ofs_y = -2},
{.bitmap_index = 326, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 331, .adv_w = 128, .box_h = 7, .box_w = 3, .ofs_x = 2, .ofs_y = -1},
{.bitmap_index = 334, .adv_w = 128, .box_h = 8, .box_w = 4, .ofs_x = 2, .ofs_y = -2},
{.bitmap_index = 338, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 343, .adv_w = 128, .box_h = 7, .box_w = 3, .ofs_x = 2, .ofs_y = -1},
{.bitmap_index = 346, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 350, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 354, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 358, .adv_w = 128, .box_h = 6, .box_w = 5, .ofs_x = 1, .ofs_y = -2},
{.bitmap_index = 362, .adv_w = 128, .box_h = 6, .box_w = 5, .ofs_x = 1, .ofs_y = -2},
{.bitmap_index = 366, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 370, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 374, .adv_w = 128, .box_h = 7, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 379, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 383, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 387, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 391, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 395, .adv_w = 128, .box_h = 6, .box_w = 5, .ofs_x = 1, .ofs_y = -2},
{.bitmap_index = 399, .adv_w = 128, .box_h = 5, .box_w = 5, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 403, .adv_w = 128, .box_h = 7, .box_w = 4, .ofs_x = 1, .ofs_y = -1},
{.bitmap_index = 407, .adv_w = 128, .box_h = 8, .box_w = 1, .ofs_x = 3, .ofs_y = -2},
{.bitmap_index = 408, .adv_w = 128, .box_h = 7, .box_w = 4, .ofs_x = 2, .ofs_y = -1},
{.bitmap_index = 412, .adv_w = 128, .box_h = 3, .box_w = 5, .ofs_x = 1, .ofs_y = 3},
{.bitmap_index = 414, .adv_w = 128, .box_h = 7, .box_w = 7, .ofs_x = 0, .ofs_y = -1}
};








static const lv_font_fmt_txt_cmap_t cmaps[] =
{
{
.range_start = 32, .range_length = 96, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
.glyph_id_start = 1, .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0
}
};








static lv_font_fmt_txt_dsc_t font_dsc = {
.glyph_bitmap = gylph_bitmap,
.glyph_dsc = glyph_dsc,
.cmaps = cmaps,
.cmap_num = 1,
.bpp = 1,

.kern_scale = 0,
.kern_dsc = NULL,
.kern_classes = 0,
};







lv_font_t lv_font_unscii_8 = {
.dsc = &font_dsc, 
.get_glyph_bitmap = lv_font_get_bitmap_fmt_txt, 
.get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt, 
.line_height = 8, 
.base_line = 2, 
};

#endif 
