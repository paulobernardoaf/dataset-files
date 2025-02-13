#include "lv_draw_img.h"
#include "lv_img_cache.h"
#include "../lv_misc/lv_log.h"
#include "../lv_misc/lv_mem.h"
static lv_res_t lv_img_draw_core(const lv_area_t * coords, const lv_area_t * mask, const void * src,
const lv_style_t * style, lv_opa_t opa_scale);
void lv_draw_img(const lv_area_t * coords, const lv_area_t * mask, const void * src, const lv_style_t * style,
lv_opa_t opa_scale)
{
if(src == NULL) {
LV_LOG_WARN("Image draw: src is NULL");
lv_draw_rect(coords, mask, &lv_style_plain, LV_OPA_COVER);
lv_draw_label(coords, mask, &lv_style_plain, LV_OPA_COVER, "No\ndata", LV_TXT_FLAG_NONE, NULL, NULL, NULL, LV_BIDI_DIR_LTR);
return;
}
lv_res_t res;
res = lv_img_draw_core(coords, mask, src, style, opa_scale);
if(res == LV_RES_INV) {
LV_LOG_WARN("Image draw error");
lv_draw_rect(coords, mask, &lv_style_plain, LV_OPA_COVER);
lv_draw_label(coords, mask, &lv_style_plain, LV_OPA_COVER, "No\ndata", LV_TXT_FLAG_NONE, NULL, NULL, NULL, LV_BIDI_DIR_LTR);
return;
}
}
lv_color_t lv_img_buf_get_px_color(lv_img_dsc_t * dsc, lv_coord_t x, lv_coord_t y, const lv_style_t * style)
{
lv_color_t p_color = LV_COLOR_BLACK;
if(x >= (lv_coord_t)dsc->header.w) {
x = dsc->header.w - 1;
LV_LOG_WARN("lv_canvas_get_px: x is too large (out of canvas)");
} else if(x < 0) {
x = 0;
LV_LOG_WARN("lv_canvas_get_px: x is < 0 (out of canvas)");
}
if(y >= (lv_coord_t)dsc->header.h) {
y = dsc->header.h - 1;
LV_LOG_WARN("lv_canvas_get_px: y is too large (out of canvas)");
} else if(y < 0) {
y = 0;
LV_LOG_WARN("lv_canvas_get_px: y is < 0 (out of canvas)");
}
uint8_t * buf_u8 = (uint8_t *)dsc->data;
if(dsc->header.cf == LV_IMG_CF_TRUE_COLOR || dsc->header.cf == LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED ||
dsc->header.cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {
uint8_t px_size = lv_img_color_format_get_px_size(dsc->header.cf) >> 3;
uint32_t px = dsc->header.w * y * px_size + x * px_size;
memcpy(&p_color, &buf_u8[px], sizeof(lv_color_t));
#if LV_COLOR_SIZE == 32
p_color.ch.alpha = 0xFF; 
#endif
} else if(dsc->header.cf == LV_IMG_CF_INDEXED_1BIT) {
buf_u8 += 4 * 2;
uint8_t bit = x & 0x7;
x = x >> 3;
uint32_t px = ((dsc->header.w + 7) >> 3) * y + x;
p_color.full = (buf_u8[px] & (1 << (7 - bit))) >> (7 - bit);
} else if(dsc->header.cf == LV_IMG_CF_INDEXED_2BIT) {
buf_u8 += 4 * 4;
uint8_t bit = (x & 0x3) * 2;
x = x >> 2;
uint32_t px = ((dsc->header.w + 3) >> 2) * y + x;
p_color.full = (buf_u8[px] & (3 << (6 - bit))) >> (6 - bit);
} else if(dsc->header.cf == LV_IMG_CF_INDEXED_4BIT) {
buf_u8 += 4 * 16;
uint8_t bit = (x & 0x1) * 4;
x = x >> 1;
uint32_t px = ((dsc->header.w + 1) >> 1) * y + x;
p_color.full = (buf_u8[px] & (0xF << (4 - bit))) >> (4 - bit);
} else if(dsc->header.cf == LV_IMG_CF_INDEXED_8BIT) {
buf_u8 += 4 * 256;
uint32_t px = dsc->header.w * y + x;
p_color.full = buf_u8[px];
} else if(dsc->header.cf == LV_IMG_CF_ALPHA_1BIT || dsc->header.cf == LV_IMG_CF_ALPHA_2BIT ||
dsc->header.cf == LV_IMG_CF_ALPHA_4BIT || dsc->header.cf == LV_IMG_CF_ALPHA_8BIT) {
if(style)
p_color = style->image.color;
else
p_color = LV_COLOR_BLACK;
}
return p_color;
}
lv_opa_t lv_img_buf_get_px_alpha(lv_img_dsc_t * dsc, lv_coord_t x, lv_coord_t y)
{
if(x >= (lv_coord_t)dsc->header.w) {
x = dsc->header.w - 1;
LV_LOG_WARN("lv_canvas_get_px: x is too large (out of canvas)");
} else if(x < 0) {
x = 0;
LV_LOG_WARN("lv_canvas_get_px: x is < 0 (out of canvas)");
}
if(y >= (lv_coord_t)dsc->header.h) {
y = dsc->header.h - 1;
LV_LOG_WARN("lv_canvas_get_px: y is too large (out of canvas)");
} else if(y < 0) {
y = 0;
LV_LOG_WARN("lv_canvas_get_px: y is < 0 (out of canvas)");
}
uint8_t * buf_u8 = (uint8_t *)dsc->data;
if(dsc->header.cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {
uint32_t px = dsc->header.w * y * LV_IMG_PX_SIZE_ALPHA_BYTE + x * LV_IMG_PX_SIZE_ALPHA_BYTE;
return buf_u8[px + LV_IMG_PX_SIZE_ALPHA_BYTE - 1];
} else if(dsc->header.cf == LV_IMG_CF_ALPHA_1BIT) {
uint8_t bit = x & 0x7;
x = x >> 3;
uint32_t px = ((dsc->header.w + 7) >> 3) * y + x;
uint8_t px_opa = (buf_u8[px] & (1 << (7 - bit))) >> (7 - bit);
return px_opa ? LV_OPA_TRANSP : LV_OPA_COVER;
} else if(dsc->header.cf == LV_IMG_CF_ALPHA_2BIT) {
const uint8_t opa_table[4] = {0, 85, 170, 255}; 
uint8_t bit = (x & 0x3) * 2;
x = x >> 2;
uint32_t px = ((dsc->header.w + 3) >> 2) * y + x;
uint8_t px_opa = (buf_u8[px] & (3 << (6 - bit))) >> (6 - bit);
return opa_table[px_opa];
} else if(dsc->header.cf == LV_IMG_CF_ALPHA_4BIT) {
const uint8_t opa_table[16] = {0, 17, 34, 51, 
68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255};
uint8_t bit = (x & 0x1) * 4;
x = x >> 1;
uint32_t px = ((dsc->header.w + 1) >> 1) * y + x;
uint8_t px_opa = (buf_u8[px] & (0xF << (4 - bit))) >> (4 - bit);
return opa_table[px_opa];
} else if(dsc->header.cf == LV_IMG_CF_ALPHA_8BIT) {
uint32_t px = dsc->header.w * y + x;
return buf_u8[px];
}
return LV_OPA_COVER;
}
void lv_img_buf_set_px_color(lv_img_dsc_t * dsc, lv_coord_t x, lv_coord_t y, lv_color_t c)
{
uint8_t * buf_u8 = (uint8_t *)dsc->data;
if(dsc->header.cf == LV_IMG_CF_TRUE_COLOR || dsc->header.cf == LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED) {
uint8_t px_size = lv_img_color_format_get_px_size(dsc->header.cf) >> 3;
uint32_t px = dsc->header.w * y * px_size + x * px_size;
memcpy(&buf_u8[px], &c, px_size);
} else if(dsc->header.cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {
uint8_t px_size = lv_img_color_format_get_px_size(dsc->header.cf) >> 3;
uint32_t px = dsc->header.w * y * px_size + x * px_size;
memcpy(&buf_u8[px], &c, px_size - 1); 
} else if(dsc->header.cf == LV_IMG_CF_INDEXED_1BIT) {
buf_u8 += sizeof(lv_color32_t) * 2; 
uint8_t bit = x & 0x7;
x = x >> 3;
uint32_t px = ((dsc->header.w + 7) >> 3) * y + x;
buf_u8[px] = buf_u8[px] & ~(1 << (7 - bit));
buf_u8[px] = buf_u8[px] | ((c.full & 0x1) << (7 - bit));
} else if(dsc->header.cf == LV_IMG_CF_INDEXED_2BIT) {
buf_u8 += sizeof(lv_color32_t) * 4; 
uint8_t bit = (x & 0x3) * 2;
x = x >> 2;
uint32_t px = ((dsc->header.w + 3) >> 2) * y + x;
buf_u8[px] = buf_u8[px] & ~(3 << (6 - bit));
buf_u8[px] = buf_u8[px] | ((c.full & 0x3) << (6 - bit));
} else if(dsc->header.cf == LV_IMG_CF_INDEXED_4BIT) {
buf_u8 += sizeof(lv_color32_t) * 16; 
uint8_t bit = (x & 0x1) * 4;
x = x >> 1;
uint32_t px = ((dsc->header.w + 1) >> 1) * y + x;
buf_u8[px] = buf_u8[px] & ~(0xF << (4 - bit));
buf_u8[px] = buf_u8[px] | ((c.full & 0xF) << (4 - bit));
} else if(dsc->header.cf == LV_IMG_CF_INDEXED_8BIT) {
buf_u8 += sizeof(lv_color32_t) * 256; 
uint32_t px = dsc->header.w * y + x;
buf_u8[px] = c.full;
}
}
void lv_img_buf_set_px_alpha(lv_img_dsc_t * dsc, lv_coord_t x, lv_coord_t y, lv_opa_t opa)
{
uint8_t * buf_u8 = (uint8_t *)dsc->data;
if(dsc->header.cf == LV_IMG_CF_TRUE_COLOR_ALPHA) {
uint8_t px_size = lv_img_color_format_get_px_size(dsc->header.cf) >> 3;
uint32_t px = dsc->header.w * y * px_size + x * px_size;
buf_u8[px + px_size - 1] = opa;
} else if(dsc->header.cf == LV_IMG_CF_ALPHA_1BIT) {
opa = opa >> 7; 
uint8_t bit = x & 0x7;
x = x >> 3;
uint32_t px = ((dsc->header.w + 7) >> 3) * y + x;
buf_u8[px] = buf_u8[px] & ~(1 << (7 - bit));
buf_u8[px] = buf_u8[px] | ((opa & 0x1) << (7 - bit));
} else if(dsc->header.cf == LV_IMG_CF_ALPHA_2BIT) {
opa = opa >> 6; 
uint8_t bit = (x & 0x3) * 2;
x = x >> 2;
uint32_t px = ((dsc->header.w + 3) >> 2) * y + x;
buf_u8[px] = buf_u8[px] & ~(3 << (6 - bit));
buf_u8[px] = buf_u8[px] | ((opa & 0x3) << (6 - bit));
} else if(dsc->header.cf == LV_IMG_CF_ALPHA_4BIT) {
opa = opa >> 4; 
uint8_t bit = (x & 0x1) * 4;
x = x >> 1;
uint32_t px = ((dsc->header.w + 1) >> 1) * y + x;
buf_u8[px] = buf_u8[px] & ~(0xF << (4 - bit));
buf_u8[px] = buf_u8[px] | ((opa & 0xF) << (4 - bit));
} else if(dsc->header.cf == LV_IMG_CF_ALPHA_8BIT) {
uint32_t px = dsc->header.w * y + x;
buf_u8[px] = opa;
}
}
void lv_img_buf_set_palette(lv_img_dsc_t * dsc, uint8_t id, lv_color_t c)
{
if((dsc->header.cf == LV_IMG_CF_ALPHA_1BIT && id > 1) || (dsc->header.cf == LV_IMG_CF_ALPHA_2BIT && id > 3) ||
(dsc->header.cf == LV_IMG_CF_ALPHA_4BIT && id > 15) || (dsc->header.cf == LV_IMG_CF_ALPHA_8BIT)) {
LV_LOG_WARN("lv_img_buf_set_px_alpha: invalid 'id'");
return;
}
lv_color32_t c32;
c32.full = lv_color_to32(c);
uint8_t * buf = (uint8_t *)dsc->data;
memcpy(&buf[id * sizeof(c32)], &c32, sizeof(c32));
}
uint8_t lv_img_color_format_get_px_size(lv_img_cf_t cf)
{
uint8_t px_size = 0;
switch(cf) {
case LV_IMG_CF_UNKNOWN:
case LV_IMG_CF_RAW: px_size = 0; break;
case LV_IMG_CF_TRUE_COLOR:
case LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED: px_size = LV_COLOR_SIZE; break;
case LV_IMG_CF_TRUE_COLOR_ALPHA: px_size = LV_IMG_PX_SIZE_ALPHA_BYTE << 3; break;
case LV_IMG_CF_INDEXED_1BIT:
case LV_IMG_CF_ALPHA_1BIT: px_size = 1; break;
case LV_IMG_CF_INDEXED_2BIT:
case LV_IMG_CF_ALPHA_2BIT: px_size = 2; break;
case LV_IMG_CF_INDEXED_4BIT:
case LV_IMG_CF_ALPHA_4BIT: px_size = 4; break;
case LV_IMG_CF_INDEXED_8BIT:
case LV_IMG_CF_ALPHA_8BIT: px_size = 8; break;
default: px_size = 0; break;
}
return px_size;
}
bool lv_img_color_format_is_chroma_keyed(lv_img_cf_t cf)
{
bool is_chroma_keyed = false;
switch(cf) {
case LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED:
case LV_IMG_CF_RAW_CHROMA_KEYED:
#if LV_INDEXED_CHROMA
case LV_IMG_CF_INDEXED_1BIT:
case LV_IMG_CF_INDEXED_2BIT:
case LV_IMG_CF_INDEXED_4BIT:
case LV_IMG_CF_INDEXED_8BIT:
#endif
is_chroma_keyed = true; break;
default: is_chroma_keyed = false; break;
}
return is_chroma_keyed;
}
bool lv_img_color_format_has_alpha(lv_img_cf_t cf)
{
bool has_alpha = false;
switch(cf) {
case LV_IMG_CF_TRUE_COLOR_ALPHA:
case LV_IMG_CF_RAW_ALPHA:
case LV_IMG_CF_INDEXED_1BIT:
case LV_IMG_CF_INDEXED_2BIT:
case LV_IMG_CF_INDEXED_4BIT:
case LV_IMG_CF_INDEXED_8BIT:
case LV_IMG_CF_ALPHA_1BIT:
case LV_IMG_CF_ALPHA_2BIT:
case LV_IMG_CF_ALPHA_4BIT:
case LV_IMG_CF_ALPHA_8BIT: has_alpha = true; break;
default: has_alpha = false; break;
}
return has_alpha;
}
lv_img_src_t lv_img_src_get_type(const void * src)
{
lv_img_src_t img_src_type = LV_IMG_SRC_UNKNOWN;
if(src == NULL) return img_src_type;
const uint8_t * u8_p = src;
if(u8_p[0] >= 0x20 && u8_p[0] <= 0x7F) {
img_src_type = LV_IMG_SRC_FILE; 
} else if(u8_p[0] >= 0x80) {
img_src_type = LV_IMG_SRC_SYMBOL; 
} else {
img_src_type = LV_IMG_SRC_VARIABLE; 
}
if(LV_IMG_SRC_UNKNOWN == img_src_type) {
LV_LOG_WARN("lv_img_src_get_type: unknown image type");
}
return img_src_type;
}
lv_img_dsc_t *lv_img_buf_alloc(lv_coord_t w, lv_coord_t h, lv_img_cf_t cf)
{
lv_img_dsc_t *dsc = lv_mem_alloc(sizeof(lv_img_dsc_t));
if(dsc == NULL)
return NULL;
memset(dsc, 0, sizeof(lv_img_dsc_t));
dsc->data_size = lv_img_buf_get_img_size(w, h, cf);
if(dsc->data_size == 0) {
lv_mem_free(dsc);
return NULL;
}
dsc->data = lv_mem_alloc(dsc->data_size);
if(dsc->data == NULL) {
lv_mem_free(dsc);
return NULL;
}
memset((uint8_t *)dsc->data, 0, dsc->data_size);
dsc->header.always_zero = 0;
dsc->header.w = w;
dsc->header.h = h;
dsc->header.cf = cf;
return dsc;
}
void lv_img_buf_free(lv_img_dsc_t *dsc)
{
if(dsc != NULL) {
if(dsc->data != NULL)
lv_mem_free(dsc->data);
lv_mem_free(dsc);
}
}
uint32_t lv_img_buf_get_img_size(lv_coord_t w, lv_coord_t h, lv_img_cf_t cf)
{
switch(cf) {
case LV_IMG_CF_TRUE_COLOR: return LV_IMG_BUF_SIZE_TRUE_COLOR(w, h);
case LV_IMG_CF_TRUE_COLOR_ALPHA: return LV_IMG_BUF_SIZE_TRUE_COLOR_ALPHA(w, h);
case LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED: return LV_IMG_BUF_SIZE_TRUE_COLOR_CHROMA_KEYED(w, h);
case LV_IMG_CF_ALPHA_1BIT: return LV_IMG_BUF_SIZE_ALPHA_1BIT(w, h);
case LV_IMG_CF_ALPHA_2BIT: return LV_IMG_BUF_SIZE_ALPHA_2BIT(w, h);
case LV_IMG_CF_ALPHA_4BIT: return LV_IMG_BUF_SIZE_ALPHA_4BIT(w, h);
case LV_IMG_CF_ALPHA_8BIT: return LV_IMG_BUF_SIZE_ALPHA_8BIT(w, h);
case LV_IMG_CF_INDEXED_1BIT: return LV_IMG_BUF_SIZE_INDEXED_1BIT(w, h);
case LV_IMG_CF_INDEXED_2BIT: return LV_IMG_BUF_SIZE_INDEXED_2BIT(w, h);
case LV_IMG_CF_INDEXED_4BIT: return LV_IMG_BUF_SIZE_INDEXED_4BIT(w, h);
case LV_IMG_CF_INDEXED_8BIT: return LV_IMG_BUF_SIZE_INDEXED_8BIT(w, h);
default: return 0;
}
}
static lv_res_t lv_img_draw_core(const lv_area_t * coords, const lv_area_t * mask, const void * src,
const lv_style_t * style, lv_opa_t opa_scale)
{
lv_area_t mask_com; 
bool union_ok;
union_ok = lv_area_intersect(&mask_com, mask, coords);
if(union_ok == false) {
return LV_RES_OK; 
}
lv_opa_t opa =
opa_scale == LV_OPA_COVER ? style->image.opa : (uint16_t)((uint16_t)style->image.opa * opa_scale) >> 8;
lv_img_cache_entry_t * cdsc = lv_img_cache_open(src, style);
if(cdsc == NULL) return LV_RES_INV;
bool chroma_keyed = lv_img_color_format_is_chroma_keyed(cdsc->dec_dsc.header.cf);
bool alpha_byte = lv_img_color_format_has_alpha(cdsc->dec_dsc.header.cf);
if(cdsc->dec_dsc.error_msg != NULL) {
LV_LOG_WARN("Image draw error");
lv_draw_rect(coords, mask, &lv_style_plain, LV_OPA_COVER);
lv_draw_label(coords, mask, &lv_style_plain, LV_OPA_COVER, cdsc->dec_dsc.error_msg, LV_TXT_FLAG_NONE, NULL, NULL, NULL, LV_BIDI_DIR_LTR);
}
else if(cdsc->dec_dsc.img_data) {
lv_draw_map(coords, mask, cdsc->dec_dsc.img_data, opa, chroma_keyed, alpha_byte, style->image.color,
style->image.intense);
}
else {
lv_coord_t width = lv_area_get_width(&mask_com);
uint8_t * buf = lv_draw_get_buf(lv_area_get_width(&mask_com) * LV_IMG_PX_SIZE_ALPHA_BYTE); 
lv_area_t line;
lv_area_copy(&line, &mask_com);
lv_area_set_height(&line, 1);
lv_coord_t x = mask_com.x1 - coords->x1;
lv_coord_t y = mask_com.y1 - coords->y1;
lv_coord_t row;
lv_res_t read_res;
for(row = mask_com.y1; row <= mask_com.y2; row++) {
read_res = lv_img_decoder_read_line(&cdsc->dec_dsc, x, y, width, buf);
if(read_res != LV_RES_OK) {
lv_img_decoder_close(&cdsc->dec_dsc);
LV_LOG_WARN("Image draw can't read the line");
return LV_RES_INV;
}
lv_draw_map(&line, mask, buf, opa, chroma_keyed, alpha_byte, style->image.color, style->image.intense);
line.y1++;
line.y2++;
y++;
}
}
return LV_RES_OK;
}
