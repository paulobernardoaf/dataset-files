




#if !defined(LV_VERSION_H)
#define LV_VERSION_H

#if defined(__cplusplus)
extern "C" {
#endif





#define LVGL_VERSION_MAJOR 6
#define LVGL_VERSION_MINOR 1
#define LVGL_VERSION_PATCH 2
#define LVGL_VERSION_INFO ""






































#define LV_VERSION_CHECK(x,y,z) (x == LVGL_VERSION_MAJOR && (y < LVGL_VERSION_MINOR || (y == LVGL_VERSION_MINOR && z <= LVGL_VERSION_PATCH)))


#if defined(__cplusplus)
} 
#endif

#endif 
