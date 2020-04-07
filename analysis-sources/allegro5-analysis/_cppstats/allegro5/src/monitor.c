#include "allegro5/allegro.h"
#include "allegro5/internal/aintern_system.h"
int al_get_num_video_adapters(void)
{
ALLEGRO_SYSTEM *system = al_get_system_driver();
if (system && system->vt && system->vt->get_num_video_adapters) {
return system->vt->get_num_video_adapters();
}
return 0;
}
bool al_get_monitor_info(int adapter, ALLEGRO_MONITOR_INFO *info)
{
ALLEGRO_SYSTEM *system = al_get_system_driver();
if (adapter < al_get_num_video_adapters()) {
if (system && system->vt && system->vt->get_monitor_info) {
return system->vt->get_monitor_info(adapter, info);
}
}
info->x1 = info->y1 = info->x2 = info->y2 = INT_MAX;
return false;
}
int al_get_monitor_dpi(int adapter)
{
ALLEGRO_SYSTEM *system = al_get_system_driver();
if (adapter < al_get_num_video_adapters()) {
if (system && system->vt && system->vt->get_monitor_dpi) {
return system->vt->get_monitor_dpi(adapter);
}
}
return 0;
}
