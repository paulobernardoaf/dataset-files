typedef struct bridged_es_t
{
es_format_t fmt;
picture_t *p_picture;
picture_t **pp_last;
bool b_empty;
char *psz_id;
int i_alpha;
int i_x;
int i_y;
} bridged_es_t;
typedef struct bridge_t
{
bridged_es_t **pp_es;
int i_es_num;
} bridge_t;
static bridge_t *GetBridge( vlc_object_t *p_object )
{
return var_GetAddress(VLC_OBJECT(vlc_object_instance(p_object)),
"mosaic-struct");
}
#define GetBridge(a) GetBridge( VLC_OBJECT(a) )
