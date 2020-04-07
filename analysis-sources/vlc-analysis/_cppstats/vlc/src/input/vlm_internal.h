#include <vlc_vlm.h>
#include <vlc_player.h>
#include "input_interface.h"
typedef struct
{
char *psz_name;
int i_index;
vlc_object_t *p_parent;
input_item_t *p_item;
vlc_player_t *player;
vlc_player_listener_id *listener;
} vlm_media_instance_sys_t;
typedef struct
{
struct vlc_object_t obj;
vlm_media_t cfg;
int i_instance;
vlm_media_instance_sys_t **instance;
} vlm_media_sys_t;
typedef struct
{
char *psz_name;
bool b_enabled;
int i_command;
char **command;
time_t date;
time_t period;
int i_repeat;
} vlm_schedule_sys_t;
struct vlm_t
{
struct vlc_object_t obj;
vlc_mutex_t lock;
vlc_thread_t thread;
vlc_mutex_t lock_manage;
vlc_cond_t wait_manage;
unsigned users;
bool input_state_changed;
bool exiting;
int64_t i_id;
int i_media;
vlm_media_sys_t **media;
int i_schedule;
vlm_schedule_sys_t **schedule;
};
int vlm_ControlInternal( vlm_t *p_vlm, int i_query, ... );
int ExecuteCommand( vlm_t *, const char *, vlm_message_t ** );
void vlm_ScheduleDelete( vlm_t *vlm, vlm_schedule_sys_t *sched );
