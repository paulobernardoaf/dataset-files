#include <obs-module.h>
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("linux-pulseaudio", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
return "Linux PulseAudio input/output capture";
}
extern struct obs_source_info pulse_input_capture;
extern struct obs_source_info pulse_output_capture;
bool obs_module_load(void)
{
obs_register_source(&pulse_input_capture);
obs_register_source(&pulse_output_capture);
return true;
}
