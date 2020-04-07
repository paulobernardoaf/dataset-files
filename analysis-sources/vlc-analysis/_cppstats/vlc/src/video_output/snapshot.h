#include <vlc_picture.h>
typedef struct vout_snapshot vout_snapshot_t;
vout_snapshot_t *vout_snapshot_New(void);
void vout_snapshot_Destroy(vout_snapshot_t *);
void vout_snapshot_End(vout_snapshot_t *);
picture_t *vout_snapshot_Get(vout_snapshot_t *, vlc_tick_t timeout);
bool vout_snapshot_IsRequested(vout_snapshot_t *);
void vout_snapshot_Set(vout_snapshot_t *, const video_format_t *, picture_t *);
char *vout_snapshot_GetDirectory(void);
typedef struct {
bool is_sequential;
int sequence;
char *path;
char *format;
char *prefix_fmt;
} vout_snapshot_save_cfg_t;
int vout_snapshot_SaveImage(char **name, int *sequential,
const block_t *image,
vout_thread_t *p_vout,
const vout_snapshot_save_cfg_t *cfg);
