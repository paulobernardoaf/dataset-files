





















typedef struct visual_effect_t visual_effect_t;
typedef int (*visual_run_t)(visual_effect_t *, vlc_object_t *,
const block_t *, picture_t *);
typedef void (*visual_free_t)(void *);

struct visual_effect_t
{
visual_run_t pf_run;
visual_free_t pf_free;
void * p_data; 
int i_width;
int i_height;
int i_nb_chans;


int i_idx_left;
int i_idx_right;
};

extern const struct visual_cb_t
{
char name[16];
visual_run_t run_cb;
visual_free_t free_cb;
} effectv[];
extern const unsigned effectc;
