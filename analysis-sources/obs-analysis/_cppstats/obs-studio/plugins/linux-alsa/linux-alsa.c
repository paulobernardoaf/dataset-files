#include <obs-module.h>
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("linux-alsa", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
return "Linux ALSA audio input capture";
}
extern struct obs_source_info alsa_input_capture;
bool obs_module_load(void)
{
obs_register_source(&alsa_input_capture);
return true;
}
