






















#if !defined(VLC_PUZZLE_H)
#define VLC_PUZZLE_H 1

#include "puzzle_mgt.h"

struct filter_sys_t {
bool b_init;
bool b_bake_request;
bool b_shape_init;
bool b_change_param;
bool b_finished;
bool b_shuffle_rqst;
bool b_mouse_drag;
bool b_mouse_mvt;

param_t s_allocated;
param_t s_current_param;
param_t s_new_param;

uint32_t i_done_count, i_tmp_done_count;

int32_t i_mouse_drag_pce;
int32_t i_mouse_x, i_mouse_y;
int16_t i_pointed_pce;
int8_t i_mouse_action;

uint32_t i_solve_acc_loop, i_solve_grp_loop, i_calc_corn_loop;
int32_t i_magnet_accuracy;
int32_t *pi_group_qty;

int32_t *pi_order; 
puzzle_array_t ***ps_puzzle_array; 
piece_shape_t **ps_pieces_shapes; 
piece_t *ps_pieces; 
piece_t *ps_pieces_tmp; 

puzzle_plane_t *ps_desk_planes;
puzzle_plane_t *ps_pict_planes;

uint8_t i_preview_pos;
int32_t i_selected;

vlc_mutex_t lock, pce_lock;

int32_t i_auto_shuffle_countdown_val, i_auto_solve_countdown_val;

point_t **ps_bezier_pts_H;
};

struct vlc_mouse_t;

picture_t *Filter( filter_t *, picture_t * );
int puzzle_Callback( vlc_object_t *, char const *, vlc_value_t, vlc_value_t, void * );
int puzzle_mouse( filter_t *, struct vlc_mouse_t *, const struct vlc_mouse_t *,
const struct vlc_mouse_t * );

#endif
