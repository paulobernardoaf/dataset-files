#if 0
#include "lv_port_indev_template.h"
static void touchpad_init(void);
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y);
static void mouse_init(void);
static bool mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static bool mouse_is_pressed(void);
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y);
static void keypad_init(void);
static bool keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static uint32_t keypad_get_key(void);
static void encoder_init(void);
static bool encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static void encoder_handler(void);
static void button_init(void);
static bool button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static int8_t button_get_pressed_id(void);
static bool button_is_pressed(uint8_t id);
lv_indev_t * indev_touchpad;
lv_indev_t * indev_mouse;
lv_indev_t * indev_keypad;
lv_indev_t * indev_encoder;
lv_indev_t * indev_button;
static int32_t encoder_diff;
static lv_indev_state_t encoder_state;
void lv_port_indev_init(void)
{
lv_indev_drv_t indev_drv;
touchpad_init();
lv_indev_drv_init(&indev_drv);
indev_drv.type = LV_INDEV_TYPE_POINTER;
indev_drv.read_cb = touchpad_read;
indev_touchpad = lv_indev_drv_register(&indev_drv);
mouse_init();
lv_indev_drv_init(&indev_drv);
indev_drv.type = LV_INDEV_TYPE_POINTER;
indev_drv.read_cb = mouse_read;
indev_mouse = lv_indev_drv_register(&indev_drv);
lv_obj_t * mouse_cursor = lv_img_create(lv_disp_get_scr_act(NULL), NULL);
lv_img_set_src(mouse_cursor, LV_SYMBOL_HOME);
lv_indev_set_cursor(indev_mouse, mouse_cursor);
keypad_init();
lv_indev_drv_init(&indev_drv);
indev_drv.type = LV_INDEV_TYPE_KEYPAD;
indev_drv.read_cb = keypad_read;
indev_keypad = lv_indev_drv_register(&indev_drv);
encoder_init();
lv_indev_drv_init(&indev_drv);
indev_drv.type = LV_INDEV_TYPE_KEYPAD;
indev_drv.read_cb = encoder_read;
indev_encoder = lv_indev_drv_register(&indev_drv);
button_init();
lv_indev_drv_init(&indev_drv);
indev_drv.type = LV_INDEV_TYPE_BUTTON;
indev_drv.read_cb = button_read;
indev_button = lv_indev_drv_register(&indev_drv);
static const lv_point_t btn_points[2] = {
{10, 10}, 
{40, 100}, 
};
lv_indev_set_button_points(indev_button, btn_points);
}
static void touchpad_init(void)
{
}
static bool touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
static lv_coord_t last_x = 0;
static lv_coord_t last_y = 0;
if(touchpad_is_pressed()) {
touchpad_get_xy(&last_x, &last_y);
data->state = LV_INDEV_STATE_PR;
} else {
data->state = LV_INDEV_STATE_REL;
}
data->point.x = last_x;
data->point.y = last_y;
return false;
}
static bool touchpad_is_pressed(void)
{
return false;
}
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y)
{
(*x) = 0;
(*y) = 0;
}
static void mouse_init(void)
{
}
static bool mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
mouse_get_xy(&data->point.x, &data->point.y);
if(mouse_is_pressed()) {
data->state = LV_INDEV_STATE_PR;
} else {
data->state = LV_INDEV_STATE_REL;
}
return false;
}
static bool mouse_is_pressed(void)
{
return false;
}
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y)
{
(*x) = 0;
(*y) = 0;
}
static void keypad_init(void)
{
}
static bool keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
static uint32_t last_key = 0;
mouse_get_xy(&data->point.x, &data->point.y);
uint32_t act_key = keypad_get_key();
if(act_key != 0) {
data->state = LV_INDEV_STATE_PR;
switch(act_key) {
case 1:
act_key = LV_KEY_NEXT;
break;
case 2:
act_key = LV_KEY_PREV;
break;
case 3:
act_key = LV_KEY_LEFT;
break;
case 4:
act_key = LV_KEY_RIGHT;
break;
case 5:
act_key = LV_KEY_ENTER;
break;
}
last_key = act_key;
} else {
data->state = LV_INDEV_STATE_REL;
}
data->key = last_key;
return false;
}
static uint32_t keypad_get_key(void)
{
return 0;
}
static void encoder_init(void)
{
}
static bool encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
data->enc_diff = encoder_diff;
data->state = encoder_state;
return false;
}
static void encoder_handler(void)
{
encoder_diff += 0;
encoder_state = LV_INDEV_STATE_REL;
}
static void button_init(void)
{
}
static bool button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
static uint8_t last_btn = 0;
int8_t btn_act = button_get_pressed_id();
if(btn_act >= 0) {
data->state = LV_INDEV_STATE_PR;
last_btn = btn_act;
} else {
data->state = LV_INDEV_STATE_REL;
}
data->btn_id = last_btn;
return false;
}
static int8_t button_get_pressed_id(void)
{
uint8_t i;
for(i = 0; i < 2; i++) {
if(button_is_pressed(i)) {
return i;
}
}
return -1;
}
static bool button_is_pressed(uint8_t id)
{
return false;
}
#else 
typedef int keep_pedantic_happy;
#endif
