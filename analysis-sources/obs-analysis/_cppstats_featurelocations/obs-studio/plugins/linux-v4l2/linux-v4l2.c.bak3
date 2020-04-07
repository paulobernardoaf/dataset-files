















#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("linux-v4l2", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
return "Video4Linux2(V4L2) sources";
}

extern struct obs_source_info v4l2_input;

bool obs_module_load(void)
{
obs_register_source(&v4l2_input);
return true;
}
