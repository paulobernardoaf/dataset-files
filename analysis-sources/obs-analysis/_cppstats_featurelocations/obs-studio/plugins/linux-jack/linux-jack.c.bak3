















#include <obs-module.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("linux-jack", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
return "JACK Audio Connection Kit output capture";
}

extern struct obs_source_info jack_output_capture;

bool obs_module_load(void)
{
obs_register_source(&jack_output_capture);
return true;
}
