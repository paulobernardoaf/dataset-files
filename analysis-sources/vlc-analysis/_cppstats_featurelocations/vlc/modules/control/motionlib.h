





















#if !defined(MOTION_H)
#define MOTION_H

typedef struct motion_sensors_t motion_sensors_t;

motion_sensors_t *motion_create( vlc_object_t *obj );
void motion_destroy( motion_sensors_t *motion );
int motion_get_angle( motion_sensors_t *motion );

#endif

