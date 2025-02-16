





#if 0




#include "lv_port_disp_template.h"












static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
#if LV_USE_GPU
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
const lv_area_t * fill_area, lv_color_t color);
#endif













void lv_port_disp_init(void)
{



disp_init();
























static lv_disp_buf_t disp_buf_1;
static lv_color_t buf1_1[LV_HOR_RES_MAX * 10]; 
lv_disp_buf_init(&disp_buf_1, buf1_1, NULL, LV_HOR_RES_MAX * 10); 


static lv_disp_buf_t disp_buf_2;
static lv_color_t buf2_1[LV_HOR_RES_MAX * 10]; 
static lv_color_t buf2_2[LV_HOR_RES_MAX * 10]; 
lv_disp_buf_init(&disp_buf_2, buf2_1, buf2_2, LV_HOR_RES_MAX * 10); 


static lv_disp_buf_t disp_buf_3;
static lv_color_t buf3_1[LV_HOR_RES_MAX * LV_VER_RES_MAX]; 
static lv_color_t buf3_2[LV_HOR_RES_MAX * LV_VER_RES_MAX]; 
lv_disp_buf_init(&disp_buf_3, buf3_1, buf3_2, LV_HOR_RES_MAX * LV_VER_RES_MAX); 






lv_disp_drv_t disp_drv; 
lv_disp_drv_init(&disp_drv); 




disp_drv.hor_res = 480;
disp_drv.ver_res = 320;


disp_drv.flush_cb = disp_flush;


disp_drv.buffer = &disp_buf_2;

#if LV_USE_GPU



disp_drv.gpu_blend_cb = gpu_blend;


disp_drv.gpu_fill_cb = gpu_fill;
#endif


lv_disp_drv_register(&disp_drv);
}






static void disp_init(void)
{

}




static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{


int32_t x;
int32_t y;
for(y = area->y1; y <= area->y2; y++) {
for(x = area->x1; x <= area->x2; x++) {


color_p++;
}
}



lv_disp_flush_ready(disp_drv);
}



#if LV_USE_GPU



static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{

uint32_t i;
for(i = 0; i < length; i++) {
dest[i] = lv_color_mix(dest[i], src[i], opa);
}
}



static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
const lv_area_t * fill_area, lv_color_t color)
{

int32_t x, y;
dest_buf += dest_width * fill_area->y1; 

for(y = fill_area->y1; y <= fill_area->y2; y++) {
for(x = fill_area->x1; x <= fill_area->x2; x++) {
dest_buf[x] = color;
}
dest_buf+=dest_width; 
}
}

#endif 

#else 


typedef int keep_pedantic_happy;
#endif
