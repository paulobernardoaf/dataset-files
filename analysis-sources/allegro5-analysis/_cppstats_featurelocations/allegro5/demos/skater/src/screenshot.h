





#if !defined(SCREENSHOT_H)
#define SCREENSHOT_H

#include <allegro5/allegro.h>

#if defined(__cplusplus)
extern "C" {
#endif








typedef struct SCREENSHOT {
int counter;
char *name;
char *ext;
} SCREENSHOT;



















SCREENSHOT *create_screenshot(char *name, char *ext);











void destroy_screenshot(SCREENSHOT * ss);















void take_screenshot(SCREENSHOT * ss);


#if defined(__cplusplus)
}
#endif
#endif
