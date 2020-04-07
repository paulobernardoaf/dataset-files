#pragma once
#include <obs-module.h>
#if defined(__cplusplus)
extern "C" {
#endif
int_fast32_t v4l2_update_controls(int_fast32_t dev, obs_properties_t *props,
obs_data_t *settings);
#if defined(__cplusplus)
}
#endif
