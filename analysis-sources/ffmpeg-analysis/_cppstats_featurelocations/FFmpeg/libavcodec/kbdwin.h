

















#if !defined(AVCODEC_KBDWIN_H)
#define AVCODEC_KBDWIN_H

#include <stdint.h>




#define FF_KBD_WINDOW_MAX 1024







void ff_kbd_window_init(float *window, float alpha, int n);
void ff_kbd_window_init_fixed(int32_t *window, float alpha, int n);

#endif 
