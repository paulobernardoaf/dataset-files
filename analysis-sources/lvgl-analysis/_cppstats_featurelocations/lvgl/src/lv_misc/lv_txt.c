







#include "lv_txt.h"
#include "lv_math.h"
#include "lv_log.h"




#define NO_BREAK_FOUND UINT32_MAX








static inline bool is_break_char(uint32_t letter);

#if LV_TXT_ENC == LV_TXT_ENC_UTF8
static uint8_t lv_txt_utf8_size(const char * str);
static uint32_t lv_txt_unicode_to_utf8(uint32_t letter_uni);
static uint32_t lv_txt_utf8_conv_wc(uint32_t c);
static uint32_t lv_txt_utf8_next(const char * txt, uint32_t * i);
static uint32_t lv_txt_utf8_prev(const char * txt, uint32_t * i_start);
static uint32_t lv_txt_utf8_get_byte_id(const char * txt, uint32_t utf8_id);
static uint32_t lv_txt_utf8_get_char_id(const char * txt, uint32_t byte_id);
static uint32_t lv_txt_utf8_get_length(const char * txt);
#elif LV_TXT_ENC == LV_TXT_ENC_ASCII
static uint8_t lv_txt_iso8859_1_size(const char * str);
static uint32_t lv_txt_unicode_to_iso8859_1(uint32_t letter_uni);
static uint32_t lv_txt_iso8859_1_conv_wc(uint32_t c);
static uint32_t lv_txt_iso8859_1_next(const char * txt, uint32_t * i);
static uint32_t lv_txt_iso8859_1_prev(const char * txt, uint32_t * i_start);
static uint32_t lv_txt_iso8859_1_get_byte_id(const char * txt, uint32_t utf8_id);
static uint32_t lv_txt_iso8859_1_get_char_id(const char * txt, uint32_t byte_id);
static uint32_t lv_txt_iso8859_1_get_length(const char * txt);
#endif







#if LV_TXT_ENC == LV_TXT_ENC_UTF8
uint8_t (*lv_txt_encoded_size)(const char *) = lv_txt_utf8_size;
uint32_t (*lv_txt_unicode_to_encoded)(uint32_t) = lv_txt_unicode_to_utf8;
uint32_t (*lv_txt_encoded_conv_wc)(uint32_t) = lv_txt_utf8_conv_wc;
uint32_t (*lv_txt_encoded_next)(const char *, uint32_t *) = lv_txt_utf8_next;
uint32_t (*lv_txt_encoded_prev)(const char *, uint32_t *) = lv_txt_utf8_prev;
uint32_t (*lv_txt_encoded_get_byte_id)(const char *, uint32_t) = lv_txt_utf8_get_byte_id;
uint32_t (*lv_txt_encoded_get_char_id)(const char *, uint32_t) = lv_txt_utf8_get_char_id;
uint32_t (*lv_txt_get_encoded_length)(const char *) = lv_txt_utf8_get_length;
#elif LV_TXT_ENC == LV_TXT_ENC_ASCII
uint8_t (*lv_txt_encoded_size)(const char *) = lv_txt_iso8859_1_size;
uint32_t (*lv_txt_unicode_to_encoded)(uint32_t) = lv_txt_unicode_to_iso8859_1;
uint32_t (*lv_txt_encoded_conv_wc)(uint32_t) = lv_txt_iso8859_1_conv_wc;
uint32_t (*lv_txt_encoded_next)(const char *, uint32_t *) = lv_txt_iso8859_1_next;
uint32_t (*lv_txt_encoded_prev)(const char *, uint32_t *) = lv_txt_iso8859_1_prev;
uint32_t (*lv_txt_encoded_get_byte_id)(const char *, uint32_t) = lv_txt_iso8859_1_get_byte_id;
uint32_t (*lv_txt_encoded_get_char_id)(const char *, uint32_t) = lv_txt_iso8859_1_get_char_id;
uint32_t (*lv_txt_get_encoded_length)(const char *) = lv_txt_iso8859_1_get_length;

#endif




















void lv_txt_get_size(lv_point_t * size_res, const char * text, const lv_font_t * font, lv_coord_t letter_space,
lv_coord_t line_space, lv_coord_t max_width, lv_txt_flag_t flag)
{
size_res->x = 0;
size_res->y = 0;

if(text == NULL) return;
if(font == NULL) return;

if(flag & LV_TXT_FLAG_EXPAND) max_width = LV_COORD_MAX;

uint32_t line_start = 0;
uint32_t new_line_start = 0;
lv_coord_t act_line_length;
uint8_t letter_height = lv_font_get_line_height(font);


while(text[line_start] != '\0') {
new_line_start += lv_txt_get_next_line(&text[line_start], font, letter_space, max_width, flag);

if ((unsigned long)size_res->y + (unsigned long)letter_height + (unsigned long)line_space > LV_MAX_OF(lv_coord_t)) {
LV_LOG_WARN("lv_txt_get_size: integer overflow while calculating text height");
return;
} else {
size_res->y += letter_height;
size_res->y += line_space;
}


act_line_length = lv_txt_get_width(&text[line_start], new_line_start - line_start, font, letter_space, flag);

size_res->x = LV_MATH_MAX(act_line_length, size_res->x);
line_start = new_line_start;
}


if((line_start != 0) && (text[line_start - 1] == '\n' || text[line_start - 1] == '\r')) {
size_res->y += letter_height + line_space;
}


if(size_res->y == 0)
size_res->y = letter_height;
else
size_res->y -= line_space;
}
































static uint16_t lv_txt_get_next_word(const char * txt, const lv_font_t * font,
lv_coord_t letter_space, lv_coord_t max_width,
lv_txt_flag_t flag, uint32_t *word_w_ptr, lv_txt_cmd_state_t * cmd_state, bool force)
{
if(txt == NULL || txt[0] == '\0') return 0;
if(font == NULL) return 0;

if(flag & LV_TXT_FLAG_EXPAND) max_width = LV_COORD_MAX;

uint32_t i = 0, i_next = 0, i_next_next = 0; 
uint32_t letter = 0; 
uint32_t letter_next = 0; 
lv_coord_t letter_w;
lv_coord_t cur_w = 0; 
uint32_t word_len = 0; 
uint32_t break_index = NO_BREAK_FOUND; 
uint32_t break_letter_count = 0; 

letter = lv_txt_encoded_next(txt, &i_next);
i_next_next = i_next;


while(txt[i] != '\0') {
letter_next = lv_txt_encoded_next(txt, &i_next_next);
word_len++;


if((flag & LV_TXT_FLAG_RECOLOR) != 0) {
if(lv_txt_is_cmd(cmd_state, letter) != false) {
i = i_next;
i_next = i_next_next;
letter = letter_next;
continue; 
}
}

letter_w = lv_font_get_glyph_width(font, letter, letter_next);
cur_w += letter_w;

if(letter_w > 0) {
cur_w += letter_space;
}


if(break_index == NO_BREAK_FOUND && (cur_w - letter_space) > max_width) {
break_index = i; 
break_letter_count = word_len - 1;

}


if(letter == '\n' || letter == '\r' || is_break_char(letter)) {


if(i == 0 && break_index == NO_BREAK_FOUND && word_w_ptr != NULL) *word_w_ptr = cur_w;
word_len--;
break;
}


if( word_w_ptr != NULL && break_index == NO_BREAK_FOUND ) *word_w_ptr = cur_w;


i = i_next;
i_next = i_next_next;
letter = letter_next;
}


if( break_index == NO_BREAK_FOUND ) {
if( word_len == 0 || (letter == '\r' && letter_next == '\n') ) i = i_next;
return i;
}

#if LV_TXT_LINE_BREAK_LONG_LEN > 0

if(word_len < LV_TXT_LINE_BREAK_LONG_LEN) {
if( force ) return break_index;
if(word_w_ptr != NULL) *word_w_ptr = 0; 
return 0;
}


if(break_letter_count < LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN) {
if( force ) return break_index;
if(word_w_ptr != NULL) *word_w_ptr = 0;
return 0;
}


{
i = break_index;
int32_t n_move = LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN - (word_len - break_letter_count);

for(;n_move>0; n_move--){
lv_txt_encoded_prev(txt, &i);


}
}
return i;
#else
if( force ) return break_index;
if(word_w_ptr != NULL) *word_w_ptr = 0; 
(void) break_letter_count;
return 0;
#endif
}













uint16_t lv_txt_get_next_line(const char * txt, const lv_font_t * font,
lv_coord_t letter_space, lv_coord_t max_width, lv_txt_flag_t flag)
{
if(txt == NULL) return 0;
if(font == NULL) return 0;

if(flag & LV_TXT_FLAG_EXPAND) max_width = LV_COORD_MAX;
lv_txt_cmd_state_t cmd_state = LV_TXT_CMD_STATE_WAIT;
uint32_t i = 0; 

while(txt[i] != '\0' && max_width > 0) {
uint32_t word_w = 0;
uint32_t advance = lv_txt_get_next_word(&txt[i], font, letter_space, max_width, flag, &word_w, &cmd_state, i==0);
max_width -= word_w;

if( advance == 0 ){
if(i == 0) lv_txt_encoded_next(txt, &i); 
break;
}

i += advance;

if(txt[0] == '\n' || txt[0] == '\r') break;

if(txt[i] == '\n' || txt[i] == '\r'){
i++; 
break;
}

}


if(i == 0) {
lv_txt_encoded_next(txt, &i);
}

return i;
}











lv_coord_t lv_txt_get_width(const char * txt, uint16_t length, const lv_font_t * font, lv_coord_t letter_space,
lv_txt_flag_t flag)
{
if(txt == NULL) return 0;
if(font == NULL) return 0;

uint32_t i = 0;
lv_coord_t width = 0;
lv_txt_cmd_state_t cmd_state = LV_TXT_CMD_STATE_WAIT;
uint32_t letter;
uint32_t letter_next;

if(length != 0) {
while(i < length) {
letter = lv_txt_encoded_next(txt, &i);
letter_next = lv_txt_encoded_next(&txt[i], NULL);
if((flag & LV_TXT_FLAG_RECOLOR) != 0) {
if(lv_txt_is_cmd(&cmd_state, letter) != false) {
continue;
}
}

lv_coord_t char_width = lv_font_get_glyph_width(font, letter, letter_next);
if(char_width > 0) {
width += char_width;
width += letter_space;
}
}

if(width > 0) {
width -= letter_space; 

}
}

return width;
}









bool lv_txt_is_cmd(lv_txt_cmd_state_t * state, uint32_t c)
{
bool ret = false;

if(c == (uint32_t)LV_TXT_COLOR_CMD[0]) {
if(*state == LV_TXT_CMD_STATE_WAIT) { 
*state = LV_TXT_CMD_STATE_PAR;
ret = true;
}

else if(*state == LV_TXT_CMD_STATE_PAR) {
*state = LV_TXT_CMD_STATE_WAIT;
}

else if(*state == LV_TXT_CMD_STATE_IN) {
*state = LV_TXT_CMD_STATE_WAIT;
ret = true;
}
}


if(*state == LV_TXT_CMD_STATE_PAR) {
if(c == ' ') {
*state = LV_TXT_CMD_STATE_IN; 
}
ret = true;
}

return ret;
}








void lv_txt_ins(char * txt_buf, uint32_t pos, const char * ins_txt)
{
size_t old_len = strlen(txt_buf);
size_t ins_len = strlen(ins_txt);
size_t new_len = ins_len + old_len;
pos = lv_txt_encoded_get_byte_id(txt_buf, pos); 


size_t i;
for(i = new_len; i >= pos + ins_len; i--) {
txt_buf[i] = txt_buf[i - ins_len];
}


memcpy(txt_buf + pos, ins_txt, ins_len);
}








void lv_txt_cut(char * txt, uint32_t pos, uint32_t len)
{

size_t old_len = strlen(txt);

pos = lv_txt_encoded_get_byte_id(txt, pos); 
len = lv_txt_encoded_get_byte_id(&txt[pos], len);


uint32_t i;
for(i = pos; i <= old_len - len; i++) {
txt[i] = txt[i + len];
}
}

#if LV_TXT_ENC == LV_TXT_ENC_UTF8









static uint8_t lv_txt_utf8_size(const char * str)
{
if((str[0] & 0x80) == 0)
return 1;
else if((str[0] & 0xE0) == 0xC0)
return 2;
else if((str[0] & 0xF0) == 0xE0)
return 3;
else if((str[0] & 0xF8) == 0xF0)
return 4;
return 0; 
}






static uint32_t lv_txt_unicode_to_utf8(uint32_t letter_uni)
{
if(letter_uni < 128) return letter_uni;
uint8_t bytes[4];

if(letter_uni < 0x0800) {
bytes[0] = ((letter_uni >> 6) & 0x1F) | 0xC0;
bytes[1] = ((letter_uni >> 0) & 0x3F) | 0x80;
bytes[2] = 0;
bytes[3] = 0;
} else if(letter_uni < 0x010000) {
bytes[0] = ((letter_uni >> 12) & 0x0F) | 0xE0;
bytes[1] = ((letter_uni >> 6) & 0x3F) | 0x80;
bytes[2] = ((letter_uni >> 0) & 0x3F) | 0x80;
bytes[3] = 0;
} else if(letter_uni < 0x110000) {
bytes[0] = ((letter_uni >> 18) & 0x07) | 0xF0;
bytes[1] = ((letter_uni >> 12) & 0x3F) | 0x80;
bytes[2] = ((letter_uni >> 6) & 0x3F) | 0x80;
bytes[3] = ((letter_uni >> 0) & 0x3F) | 0x80;
}

uint32_t * res_p = (uint32_t *)bytes;
return *res_p;
}






static uint32_t lv_txt_utf8_conv_wc(uint32_t c)
{

if((c & 0x80) != 0) {
uint32_t swapped;
uint8_t c8[4];
memcpy(c8, &c, 4);
swapped = (c8[0] << 24) + (c8[1] << 16) + (c8[2] << 8) + (c8[3]);
uint8_t i;
for(i = 0; i < 4; i++) {
if((swapped & 0xFF) == 0)
swapped = (swapped >> 8); 
}
c = swapped;
}

return c;
}









static uint32_t lv_txt_utf8_next(const char * txt, uint32_t * i)
{







uint32_t result = 0;


uint32_t i_tmp = 0;
if(i == NULL) i = &i_tmp;


if((txt[*i] & 0x80) == 0) {
result = txt[*i];
(*i)++;
}

else {

if((txt[*i] & 0xE0) == 0xC0) {
result = (uint32_t)(txt[*i] & 0x1F) << 6;
(*i)++;
if((txt[*i] & 0xC0) != 0x80) return 0; 
result += (txt[*i] & 0x3F);
(*i)++;
}

else if((txt[*i] & 0xF0) == 0xE0) {
result = (uint32_t)(txt[*i] & 0x0F) << 12;
(*i)++;

if((txt[*i] & 0xC0) != 0x80) return 0; 
result += (uint32_t)(txt[*i] & 0x3F) << 6;
(*i)++;

if((txt[*i] & 0xC0) != 0x80) return 0; 
result += (txt[*i] & 0x3F);
(*i)++;
}

else if((txt[*i] & 0xF8) == 0xF0) {
result = (uint32_t)(txt[*i] & 0x07) << 18;
(*i)++;

if((txt[*i] & 0xC0) != 0x80) return 0; 
result += (uint32_t)(txt[*i] & 0x3F) << 12;
(*i)++;

if((txt[*i] & 0xC0) != 0x80) return 0; 
result += (uint32_t)(txt[*i] & 0x3F) << 6;
(*i)++;

if((txt[*i] & 0xC0) != 0x80) return 0; 
result += txt[*i] & 0x3F;
(*i)++;
} else {
(*i)++; 
}
}
return result;
}








static uint32_t lv_txt_utf8_prev(const char * txt, uint32_t * i)
{
uint8_t c_size;
uint8_t cnt = 0;


(*i)--;
do {
if(cnt >= 4) return 0; 

c_size = lv_txt_encoded_size(&txt[*i]);
if(c_size == 0) {
if(*i != 0)
(*i)--;
else
return 0;
}
cnt++;
} while(c_size == 0);

uint32_t i_tmp = *i;
uint32_t letter = lv_txt_encoded_next(txt, &i_tmp); 

return letter;
}








static uint32_t lv_txt_utf8_get_byte_id(const char * txt, uint32_t utf8_id)
{
uint32_t i;
uint32_t byte_cnt = 0;
for(i = 0; i < utf8_id; i++) {
uint8_t c_size = lv_txt_encoded_size(&txt[byte_cnt]);
byte_cnt += c_size > 0 ? c_size : 1;
}

return byte_cnt;
}








static uint32_t lv_txt_utf8_get_char_id(const char * txt, uint32_t byte_id)
{
uint32_t i = 0;
uint32_t char_cnt = 0;

while(i < byte_id) {
lv_txt_encoded_next(txt, &i); 
char_cnt++;
}

return char_cnt;
}







static uint32_t lv_txt_utf8_get_length(const char * txt)
{
uint32_t len = 0;
uint32_t i = 0;

while(txt[i] != '\0') {
lv_txt_encoded_next(txt, &i);
len++;
}

return len;
}

#elif LV_TXT_ENC == LV_TXT_ENC_ASCII









static uint8_t lv_txt_iso8859_1_size(const char * str)
{
(void)str; 
return 1;
}






static uint32_t lv_txt_unicode_to_iso8859_1(uint32_t letter_uni)
{
if(letter_uni < 128)
return letter_uni;
else
return ' ';
}







static uint32_t lv_txt_iso8859_1_conv_wc(uint32_t c)
{
return c;
}









static uint32_t lv_txt_iso8859_1_next(const char * txt, uint32_t * i)
{
if(i == NULL) return txt[1]; 

uint8_t letter = txt[*i];
(*i)++;
return letter;
}







static uint32_t lv_txt_iso8859_1_prev(const char * txt, uint32_t * i)
{
if(i == NULL) return *(txt - 1); 

(*i)--;
uint8_t letter = txt[*i];

return letter;
}








static uint32_t lv_txt_iso8859_1_get_byte_id(const char * txt, uint32_t utf8_id)
{
(void)txt; 
return utf8_id; 
}








static uint32_t lv_txt_iso8859_1_get_char_id(const char * txt, uint32_t byte_id)
{
(void)txt; 
return byte_id; 
}







static uint32_t lv_txt_iso8859_1_get_length(const char * txt)
{
return strlen(txt);
}
#else

#error "Invalid character encoding. See `LV_TXT_ENC` in `lv_conf.h`"

#endif










static inline bool is_break_char(uint32_t letter)
{
uint8_t i;
bool ret = false;


for(i = 0; LV_TXT_BREAK_CHARS[i] != '\0'; i++) {
if(letter == (uint32_t)LV_TXT_BREAK_CHARS[i]) {
ret = true; 
break;
}
}

return ret;
}
