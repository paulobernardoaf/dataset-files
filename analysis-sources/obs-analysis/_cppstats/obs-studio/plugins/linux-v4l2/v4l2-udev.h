#pragma once
#include <inttypes.h>
#include <callback/signal.h>
#if defined(__cplusplus)
extern "C" {
#endif
void v4l2_init_udev(void);
void v4l2_unref_udev(void);
signal_handler_t *v4l2_get_udev_signalhandler(void);
#if defined(__cplusplus)
}
#endif
