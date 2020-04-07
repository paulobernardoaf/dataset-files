#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <math.h>
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_filter.h>
#include <vlc_picture.h>
#include <vlc_rand.h>
#include "filter_picture.h"
#include "puzzle_bezier.h"
#include "puzzle_lib.h"
#include "puzzle_pce.h"
#include "puzzle_mgt.h"
int puzzle_bake( filter_t *p_filter, picture_t *p_pic_out, picture_t *p_pic_in)
{
filter_sys_t *p_sys = p_filter->p_sys;
int i_ret = 0;
puzzle_free_ps_puzzle_array ( p_filter );
puzzle_free_ps_pieces_shapes ( p_filter);
puzzle_free_ps_pieces ( p_filter );
p_sys->s_allocated.i_rows = p_sys->s_current_param.i_rows;
p_sys->s_allocated.i_cols = p_sys->s_current_param.i_cols;
p_sys->s_allocated.i_planes = p_sys->s_current_param.i_planes;
p_sys->s_allocated.i_piece_types = ((p_sys->s_current_param.b_advanced)?PIECE_TYPE_NBR:0);
p_sys->s_allocated.i_pieces_nbr = p_sys->s_allocated.i_rows * p_sys->s_allocated.i_cols;
p_sys->s_allocated.b_preview = p_sys->s_current_param.b_preview;
p_sys->s_allocated.i_preview_size = p_sys->s_current_param.i_preview_size;
p_sys->s_allocated.i_border = p_sys->s_current_param.i_border;
p_sys->s_allocated.b_blackslot = p_sys->s_current_param.b_blackslot;
p_sys->s_allocated.b_near = p_sys->s_current_param.b_near;
p_sys->s_allocated.i_shape_size = p_sys->s_current_param.i_shape_size;
p_sys->s_allocated.b_advanced = p_sys->s_current_param.b_advanced;
p_sys->s_allocated.i_auto_shuffle_speed = p_sys->s_current_param.i_auto_shuffle_speed;
p_sys->s_allocated.i_auto_solve_speed = p_sys->s_current_param.i_auto_solve_speed;
p_sys->s_allocated.i_rotate = p_sys->s_current_param.i_rotate;
p_sys->ps_puzzle_array = malloc( sizeof( puzzle_array_t** ) * (p_sys->s_allocated.i_rows + 1));
if( !p_sys->ps_puzzle_array )
return VLC_ENOMEM;
for (int32_t r=0; r < p_sys->s_allocated.i_rows + 1; r++) {
p_sys->ps_puzzle_array[r] = malloc( sizeof( puzzle_array_t* ) * (p_sys->s_allocated.i_cols + 1));
if( !p_sys->ps_puzzle_array[r] )
return VLC_ENOMEM;
for (int32_t c=0; c < p_sys->s_allocated.i_cols + 1; c++) {
p_sys->ps_puzzle_array[r][c] = malloc( sizeof( puzzle_array_t ) * p_sys->s_allocated.i_planes);
if( !p_sys->ps_puzzle_array[r][c] )
return VLC_ENOMEM;
}
}
p_sys->ps_desk_planes = malloc( sizeof( puzzle_plane_t ) * p_sys->s_allocated.i_planes);
if( !p_sys->ps_desk_planes )
return VLC_ENOMEM;
p_sys->ps_pict_planes = malloc( sizeof( puzzle_plane_t ) * p_sys->s_allocated.i_planes);
if( !p_sys->ps_pict_planes )
return VLC_ENOMEM;
for (uint8_t i_plane = 0; i_plane < p_sys->s_allocated.i_planes; i_plane++) {
p_sys->ps_desk_planes[i_plane].i_lines = p_pic_out->p[i_plane].i_visible_lines;
p_sys->ps_desk_planes[i_plane].i_pitch = p_pic_out->p[i_plane].i_pitch;
p_sys->ps_desk_planes[i_plane].i_visible_pitch = p_pic_out->p[i_plane].i_visible_pitch;
p_sys->ps_desk_planes[i_plane].i_pixel_pitch = p_pic_out->p[i_plane].i_pixel_pitch;
p_sys->ps_desk_planes[i_plane].i_width = p_pic_out->p[i_plane].i_visible_pitch / p_pic_out->p[i_plane].i_pixel_pitch;
p_sys->ps_desk_planes[i_plane].i_preview_width = p_sys->ps_desk_planes[i_plane].i_width * p_sys->s_current_param.i_preview_size / 100;
p_sys->ps_desk_planes[i_plane].i_preview_lines = p_sys->ps_desk_planes[i_plane].i_lines * p_sys->s_current_param.i_preview_size / 100;
p_sys->ps_desk_planes[i_plane].i_border_width = p_sys->ps_desk_planes[i_plane].i_width * p_sys->s_current_param.i_border / 2 / 100;
p_sys->ps_desk_planes[i_plane].i_border_lines = p_sys->ps_desk_planes[i_plane].i_lines * p_sys->s_current_param.i_border / 2 / 100;
p_sys->ps_desk_planes[i_plane].i_pce_max_width = (( p_sys->ps_desk_planes[i_plane].i_width
- 2 * p_sys->ps_desk_planes[i_plane].i_border_width ) + p_sys->s_allocated.i_cols - 1 ) / p_sys->s_allocated.i_cols;
p_sys->ps_desk_planes[i_plane].i_pce_max_lines = (( p_sys->ps_desk_planes[i_plane].i_lines
- 2 * p_sys->ps_desk_planes[i_plane].i_border_lines ) + p_sys->s_allocated.i_rows - 1 ) / p_sys->s_allocated.i_rows;
p_sys->ps_pict_planes[i_plane].i_lines = p_pic_in->p[i_plane].i_visible_lines;
p_sys->ps_pict_planes[i_plane].i_pitch = p_pic_in->p[i_plane].i_pitch;
p_sys->ps_pict_planes[i_plane].i_visible_pitch = p_pic_in->p[i_plane].i_visible_pitch;
p_sys->ps_pict_planes[i_plane].i_pixel_pitch = p_pic_in->p[i_plane].i_pixel_pitch;
p_sys->ps_pict_planes[i_plane].i_width = p_pic_in->p[i_plane].i_visible_pitch / p_pic_in->p[i_plane].i_pixel_pitch;
p_sys->ps_pict_planes[i_plane].i_preview_width = p_sys->ps_pict_planes[i_plane].i_width * p_sys->s_current_param.i_preview_size / 100;
p_sys->ps_pict_planes[i_plane].i_preview_lines = p_sys->ps_pict_planes[i_plane].i_lines * p_sys->s_current_param.i_preview_size / 100;
p_sys->ps_pict_planes[i_plane].i_border_width = p_sys->ps_pict_planes[i_plane].i_width * p_sys->s_current_param.i_border / 2 / 100;
p_sys->ps_pict_planes[i_plane].i_border_lines = p_sys->ps_pict_planes[i_plane].i_lines * p_sys->s_current_param.i_border / 2 / 100;
p_sys->ps_pict_planes[i_plane].i_pce_max_width = (( p_sys->ps_desk_planes[i_plane].i_width
- 2 * p_sys->ps_pict_planes[i_plane].i_border_width ) + p_sys->s_allocated.i_cols - 1 ) / p_sys->s_allocated.i_cols;
p_sys->ps_pict_planes[i_plane].i_pce_max_lines = (( p_sys->ps_pict_planes[i_plane].i_lines
- 2 * p_sys->ps_pict_planes[i_plane].i_border_lines ) + p_sys->s_allocated.i_rows - 1 ) / p_sys->s_allocated.i_rows;
for (int32_t r = 0; r < p_sys->s_allocated.i_rows; r++)
for (int32_t c = 0; c < p_sys->s_allocated.i_cols; c++) {
if ( r == 0 )
p_sys->ps_puzzle_array[r][c][i_plane].i_y = p_sys->ps_pict_planes[i_plane].i_border_lines;
if ( c == 0 )
p_sys->ps_puzzle_array[r][c][i_plane].i_x = p_sys->ps_pict_planes[i_plane].i_border_width;
p_sys->ps_puzzle_array[r][c][i_plane].i_width =
(p_sys->ps_pict_planes[i_plane].i_width - p_sys->ps_pict_planes[i_plane].i_border_width
- p_sys->ps_puzzle_array[r][c][i_plane].i_x) / ( p_sys->s_allocated.i_cols - c );
p_sys->ps_puzzle_array[r][c][i_plane].i_lines =
(p_sys->ps_pict_planes[i_plane].i_lines - p_sys->ps_pict_planes[i_plane].i_border_lines
- p_sys->ps_puzzle_array[r][c][i_plane].i_y) / ( p_sys->s_allocated.i_rows - r );
p_sys->ps_puzzle_array[r][c + 1][i_plane].i_x =
p_sys->ps_puzzle_array[r][c][i_plane].i_x + p_sys->ps_puzzle_array[r][c][i_plane].i_width;
p_sys->ps_puzzle_array[r + 1][c][i_plane].i_y =
p_sys->ps_puzzle_array[r][c][i_plane].i_y + p_sys->ps_puzzle_array[r][c][i_plane].i_lines;
}
}
p_sys->i_magnet_accuracy = ( p_sys->s_current_param.i_pict_width / 50) + 3;
if (p_sys->s_current_param.b_advanced && p_sys->s_allocated.i_shape_size != 0) {
i_ret = puzzle_bake_pieces_shapes ( p_filter );
if (i_ret != VLC_SUCCESS)
return i_ret;
}
i_ret = puzzle_bake_piece ( p_filter );
if (i_ret != VLC_SUCCESS)
return i_ret;
if ( (p_sys->pi_order != NULL) && (p_sys->ps_desk_planes != NULL) && (p_sys->ps_pict_planes != NULL)
&& (p_sys->ps_puzzle_array != NULL) && (p_sys->ps_pieces != NULL) )
p_sys->b_init = true;
if ( (p_sys->ps_pieces_shapes == NULL) && (p_sys->s_current_param.b_advanced)
&& (p_sys->s_current_param.i_shape_size != 0) )
p_sys->b_init = false;
return VLC_SUCCESS;
}
void puzzle_free_ps_puzzle_array( filter_t *p_filter)
{
filter_sys_t *p_sys = p_filter->p_sys;
if (p_sys->ps_puzzle_array != NULL) {
for (int32_t r=0; r < p_sys->s_allocated.i_rows + 1; r++) {
for (int32_t c=0; c < p_sys->s_allocated.i_cols + 1; c++)
free( p_sys->ps_puzzle_array[r][c] );
free( p_sys->ps_puzzle_array[r] );
}
free( p_sys->ps_puzzle_array );
}
p_sys->ps_puzzle_array = NULL;
free ( p_sys->ps_desk_planes );
p_sys->ps_desk_planes = NULL;
free ( p_sys->ps_pict_planes );
p_sys->ps_pict_planes = NULL;
return;
}
int puzzle_bake_piece( filter_t *p_filter)
{
int i_ret = puzzle_allocate_ps_pieces( p_filter);
if (i_ret != VLC_SUCCESS)
return i_ret;
filter_sys_t *p_sys = p_filter->p_sys;
i_ret = puzzle_shuffle( p_filter );
if (i_ret != VLC_SUCCESS)
return i_ret;
int32_t i = 0;
for (int32_t row = 0; row < p_sys->s_allocated.i_rows; row++) {
for (int32_t col = 0; col < p_sys->s_allocated.i_cols; col++) {
int32_t orow = row;
int32_t ocol = col;
if (p_sys->pi_order != NULL) {
orow = p_sys->pi_order[i] / (p_sys->s_allocated.i_cols);
ocol = p_sys->pi_order[i] % (p_sys->s_allocated.i_cols);
}
p_sys->ps_pieces[i].i_original_row = orow;
p_sys->ps_pieces[i].i_original_col = ocol;
p_sys->ps_pieces[i].i_left_shape = 0;
p_sys->ps_pieces[i].i_top_shape = 2;
p_sys->ps_pieces[i].i_btm_shape = 4;
p_sys->ps_pieces[i].i_right_shape = 6;
if (p_sys->s_allocated.i_shape_size > 0) {
if (orow < p_sys->s_allocated.i_rows - 1)
p_sys->ps_pieces[i].i_btm_shape = 4 + 8 + 8*(( (unsigned) vlc_mrand48()) % ( SHAPES_QTY ) ) + (vlc_mrand48() & 0x01);
if (ocol < p_sys->s_allocated.i_cols - 1)
p_sys->ps_pieces[i].i_right_shape = 6 + 8 + 8*(( (unsigned) vlc_mrand48()) % ( SHAPES_QTY ) ) + (vlc_mrand48() & 0x01);
}
p_sys->ps_pieces[i].i_actual_angle = 0;
p_sys->ps_pieces[i].b_overlap = false;
p_sys->ps_pieces[i].i_actual_mirror = +1;
p_sys->ps_pieces[i].b_finished = ((ocol == col) && (orow == row));
p_sys->ps_pieces[i].i_group_ID = i;
int32_t i_rand_x = 0;
int32_t i_rand_y = 0;
if (p_sys->s_current_param.b_advanced) {
i_rand_x = (( (unsigned) vlc_mrand48()) % ( p_sys->ps_desk_planes[0].i_pce_max_width + 1 ) ) - (int32_t) p_sys->ps_desk_planes[0].i_pce_max_width / 2;
i_rand_y = (( (unsigned) vlc_mrand48()) % ( p_sys->ps_desk_planes[0].i_pce_max_lines + 1 ) ) - (int32_t) p_sys->ps_desk_planes[0].i_pce_max_lines / 2;
}
if (p_sys->ps_puzzle_array != NULL) {
for (uint8_t i_plane = 0; i_plane < p_sys->s_allocated.i_planes; i_plane++) {
p_sys->ps_pieces[i].ps_piece_in_plane[i_plane].i_width = p_sys->ps_puzzle_array[row][col][i_plane].i_width;
p_sys->ps_pieces[i].ps_piece_in_plane[i_plane].i_lines = p_sys->ps_puzzle_array[row][col][i_plane].i_lines;
p_sys->ps_pieces[i].ps_piece_in_plane[i_plane].i_original_x = p_sys->ps_puzzle_array[orow][ocol][i_plane].i_x;
p_sys->ps_pieces[i].ps_piece_in_plane[i_plane].i_original_y = p_sys->ps_puzzle_array[orow][ocol][i_plane].i_y;
p_sys->ps_pieces[i].ps_piece_in_plane[i_plane].i_actual_x = p_sys->ps_puzzle_array[row][col][i_plane].i_x + i_rand_x *
p_sys->ps_desk_planes[i_plane].i_width / p_sys->ps_desk_planes[0].i_width;
p_sys->ps_pieces[i].ps_piece_in_plane[i_plane].i_actual_y = p_sys->ps_puzzle_array[row][col][i_plane].i_y + i_rand_y *
p_sys->ps_desk_planes[i_plane].i_lines / p_sys->ps_desk_planes[0].i_lines;
if (i_plane == 0) {
p_sys->ps_pieces[i].i_OLx = p_sys->ps_pieces[i].ps_piece_in_plane[0].i_original_x;
p_sys->ps_pieces[i].i_OTy = p_sys->ps_pieces[i].ps_piece_in_plane[0].i_original_y;
p_sys->ps_pieces[i].i_ORx = p_sys->ps_pieces[i].ps_piece_in_plane[0].i_original_x + p_sys->ps_pieces[i].ps_piece_in_plane[0].i_width - 1;
p_sys->ps_pieces[i].i_OBy = p_sys->ps_pieces[i].ps_piece_in_plane[0].i_original_y + p_sys->ps_pieces[i].ps_piece_in_plane[0].i_lines - 1;
puzzle_calculate_corners( p_filter, i );
}
}
}
i++;
}
}
puzzle_set_left_top_shapes( p_filter);
puzzle_random_rotate( p_filter);
return VLC_SUCCESS;
}
void puzzle_set_left_top_shapes( filter_t *p_filter)
{
filter_sys_t *p_sys = p_filter->p_sys;
for (uint16_t i_pce_B=0; i_pce_B < p_sys->s_allocated.i_pieces_nbr; i_pce_B++)
for (uint16_t i_pce_A=0; i_pce_A < p_sys->s_allocated.i_pieces_nbr; i_pce_A++)
{
if ( ( p_sys->ps_pieces[i_pce_A].i_original_row == p_sys->ps_pieces[i_pce_B].i_original_row )
&& ( p_sys->ps_pieces[i_pce_A].i_original_col == p_sys->ps_pieces[i_pce_B].i_original_col-1 ) )
p_sys->ps_pieces[i_pce_B].i_left_shape = (p_sys->ps_pieces[i_pce_A].i_right_shape - 6 ) ^ 0x01;
if ( ( p_sys->ps_pieces[i_pce_A].i_original_row == p_sys->ps_pieces[i_pce_B].i_original_row - 1 )
&& ( p_sys->ps_pieces[i_pce_A].i_original_col == p_sys->ps_pieces[i_pce_B].i_original_col ) )
p_sys->ps_pieces[i_pce_B].i_top_shape = (p_sys->ps_pieces[i_pce_A].i_btm_shape - 2 ) ^ 0x01;
}
}
void puzzle_random_rotate( filter_t *p_filter)
{
filter_sys_t *p_sys = p_filter->p_sys;
for (uint32_t i = 0; i < p_sys->s_allocated.i_pieces_nbr; i++)
{
p_sys->ps_pieces[i].i_actual_angle = 0;
p_sys->ps_pieces[i].i_actual_mirror = +1;
switch ( p_sys->s_current_param.i_rotate )
{
case 1:
puzzle_rotate_pce( p_filter, i, (( (unsigned) vlc_mrand48()) % ( 2 ) ) * 2, p_sys->ps_pieces[i].i_center_x, p_sys->ps_pieces[i].i_center_y, false );
break;
case 2:
puzzle_rotate_pce( p_filter, i, (( (unsigned) vlc_mrand48()) % ( 4 ) ), p_sys->ps_pieces[i].i_center_x, p_sys->ps_pieces[i].i_center_y, false );
break;
case 3:
puzzle_rotate_pce( p_filter, i, (( (unsigned) vlc_mrand48()) % ( 8 ) ), p_sys->ps_pieces[i].i_center_x, p_sys->ps_pieces[i].i_center_y, false );
break;
}
puzzle_calculate_corners( p_filter, i );
}
}
void puzzle_free_ps_pieces( filter_t *p_filter)
{
filter_sys_t *p_sys = p_filter->p_sys;
if (p_sys->ps_pieces != NULL) {
for (uint32_t i_pce = 0; i_pce < p_sys->s_allocated.i_pieces_nbr; i_pce++)
free( p_sys->ps_pieces[i_pce].ps_piece_in_plane );
free( p_sys->ps_pieces );
}
p_sys->ps_pieces = NULL;
free( p_sys->pi_order );
p_sys->pi_order = NULL;
free( p_sys->ps_pieces_tmp );
p_sys->ps_pieces_tmp = NULL;
free( p_sys->pi_group_qty );
p_sys->pi_group_qty = NULL;
return;
}
int puzzle_allocate_ps_pieces( filter_t *p_filter)
{
filter_sys_t *p_sys = p_filter->p_sys;
puzzle_free_ps_pieces(p_filter);
p_sys->s_allocated.i_pieces_nbr = p_sys->s_allocated.i_rows * p_sys->s_allocated.i_cols;
p_sys->ps_pieces = malloc( sizeof( piece_t) * p_sys->s_allocated.i_pieces_nbr );
if( !p_sys->ps_pieces )
return VLC_ENOMEM;
for (uint32_t p = 0; p < p_sys->s_allocated.i_pieces_nbr; p++) {
p_sys->ps_pieces[p].ps_piece_in_plane = malloc( sizeof( piece_in_plane_t) * p_sys->s_allocated.i_planes );
if( !p_sys->ps_pieces[p].ps_piece_in_plane ) {
for (uint32_t i=0;i<p;i++)
free(p_sys->ps_pieces[i].ps_piece_in_plane);
free(p_sys->ps_pieces);
p_sys->ps_pieces = NULL;
return VLC_ENOMEM;
}
}
p_sys->ps_pieces_tmp = malloc( sizeof( piece_t) * p_sys->s_allocated.i_pieces_nbr );
if( !p_sys->ps_pieces_tmp ) {
for (uint32_t i=0;i<p_sys->s_allocated.i_pieces_nbr;i++)
free(p_sys->ps_pieces[i].ps_piece_in_plane);
free(p_sys->ps_pieces);
p_sys->ps_pieces = NULL;
return VLC_ENOMEM;
}
p_sys->pi_group_qty = malloc( sizeof( int32_t ) * (p_sys->s_allocated.i_pieces_nbr));
if( !p_sys->pi_group_qty ) {
for (uint32_t i=0;i<p_sys->s_allocated.i_pieces_nbr;i++)
free(p_sys->ps_pieces[i].ps_piece_in_plane);
free(p_sys->ps_pieces);
p_sys->ps_pieces = NULL;
free(p_sys->ps_pieces_tmp);
p_sys->ps_pieces_tmp = NULL;
return VLC_ENOMEM;
}
return VLC_SUCCESS;
}
bool puzzle_is_valid( filter_sys_t *p_sys, int32_t *pi_pce_lst )
{
const int32_t i_count = p_sys->s_allocated.i_pieces_nbr;
if( !p_sys->s_current_param.b_blackslot )
return true;
int32_t d = 0;
for( int32_t i = 0; i < i_count; i++ ) {
if( pi_pce_lst[i] == i_count - 1 ) {
d += i / p_sys->s_allocated.i_cols + 1;
continue;
}
for( int32_t j = i+1; j < i_count; j++ ) {
if( pi_pce_lst[j] == i_count - 1 )
continue;
if( pi_pce_lst[i] > pi_pce_lst[j] )
d++;
}
}
return (d%2) == 0;
}
int puzzle_shuffle( filter_t *p_filter )
{
filter_sys_t *p_sys = p_filter->p_sys;
int32_t i_pieces_nbr = p_sys->s_allocated.i_pieces_nbr;
do
{
int i_ret = puzzle_generate_rand_pce_list( p_filter, &p_sys->pi_order );
if (i_ret != VLC_SUCCESS)
return i_ret;
} while( puzzle_is_finished( p_sys, p_sys->pi_order ) || !puzzle_is_valid( p_sys, p_sys->pi_order ) );
if( p_sys->s_current_param.b_blackslot ) {
for( int32_t i = 0; i < i_pieces_nbr; i++ )
if( p_sys->pi_order[i] == i_pieces_nbr - 1 ) {
p_sys->i_selected = i;
break;
}
}
else {
p_sys->i_selected = NO_PCE;
}
p_sys->b_shuffle_rqst = false;
p_sys->b_finished = false;
return VLC_SUCCESS;
}
int puzzle_generate_rand_pce_list( filter_t *p_filter, int32_t **pi_pce_lst )
{
filter_sys_t *p_sys = p_filter->p_sys;
int32_t i_pieces_nbr = p_sys->s_allocated.i_pieces_nbr;
free( *pi_pce_lst );
*pi_pce_lst = calloc( i_pieces_nbr, sizeof(**pi_pce_lst) );
if( !*pi_pce_lst )
return VLC_ENOMEM;
for( int32_t i = 0; i < i_pieces_nbr; i++ )
(*pi_pce_lst)[i] = NO_PCE;
for( int32_t c = 0; c < i_pieces_nbr; ) {
int32_t i = ((unsigned)vlc_mrand48()) % i_pieces_nbr;
if( (*pi_pce_lst)[i] == NO_PCE )
(*pi_pce_lst)[i] = c++;
}
return VLC_SUCCESS;
}
bool puzzle_is_finished( filter_sys_t *p_sys, int32_t *pi_pce_lst )
{
for( uint32_t i = 0; i < p_sys->s_allocated.i_pieces_nbr; i++ )
if( (int32_t)i != pi_pce_lst[i] )
return false;
return true;
}
int puzzle_piece_foreground( filter_t *p_filter, int32_t i_piece) {
filter_sys_t *p_sys = p_filter->p_sys;
piece_t *ps_pieces_tmp; 
uint32_t i_group_ID = p_sys->ps_pieces[i_piece].i_group_ID;
ps_pieces_tmp = malloc( sizeof( piece_t) * p_sys->s_allocated.i_pieces_nbr );
if (!ps_pieces_tmp)
return VLC_ENOMEM;
int32_t j=0;
memcpy( &ps_pieces_tmp[j], &p_sys->ps_pieces[i_piece], sizeof(piece_t) );
j++;
for (uint32_t i = 0; i < p_sys->s_allocated.i_pieces_nbr; i++) {
if ( ( p_sys->ps_pieces[i].i_group_ID == i_group_ID ) && ( (int32_t)i != i_piece ) ) {
memcpy( &ps_pieces_tmp[j], &p_sys->ps_pieces[i], sizeof(piece_t));
j++;
}
}
for (uint32_t i = 0; i < p_sys->s_allocated.i_pieces_nbr; i++) {
if ( p_sys->ps_pieces[i].i_group_ID != i_group_ID ) {
memcpy( &ps_pieces_tmp[j], &p_sys->ps_pieces[i], sizeof(piece_t));
j++;
}
}
free( p_sys->ps_pieces );
p_sys->ps_pieces = ps_pieces_tmp;
return VLC_SUCCESS;
}
void puzzle_count_pce_group( filter_t *p_filter) { 
filter_sys_t *p_sys = p_filter->p_sys;
memset ( p_sys->pi_group_qty, 0, sizeof( int32_t ) * (p_sys->s_allocated.i_pieces_nbr) );
for (uint32_t i_pce = 0; i_pce < p_sys->s_allocated.i_pieces_nbr; i_pce++)
p_sys->pi_group_qty[p_sys->ps_pieces[i_pce].i_group_ID]++;
}
void puzzle_solve_pces_group( filter_t *p_filter) {
filter_sys_t *p_sys = p_filter->p_sys;
int32_t i_dx, i_dy;
p_sys->i_solve_grp_loop++;
p_sys->i_solve_grp_loop %= p_sys->s_allocated.i_pieces_nbr;
int32_t i_piece_A = p_sys->i_solve_grp_loop;
piece_t *ps_piece_A = &p_sys->ps_pieces[i_piece_A];
for (uint32_t i_piece_B = 0; i_piece_B < p_sys->s_allocated.i_pieces_nbr; i_piece_B++)
{
piece_t *ps_piece_B = &p_sys->ps_pieces[i_piece_B];
if ( ps_piece_A->i_actual_angle != ps_piece_B->i_actual_angle || ps_piece_A->i_actual_mirror != ps_piece_B->i_actual_mirror )
continue;
if ( (ps_piece_B->i_group_ID != p_sys->ps_pieces[i_piece_A].i_group_ID ) )
{
if ( abs(ps_piece_A->i_OTy - ps_piece_B->i_OTy )<3)
{
if ( abs( ps_piece_A->i_ORx - ps_piece_B->i_OLx + 1 ) < 3
&& abs( ps_piece_A->i_TRx - ps_piece_B->i_TLx + 1 ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_A->i_TRy - ps_piece_B->i_TLy ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_A->i_BRx - ps_piece_B->i_BLx + 1 ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_A->i_BRy - ps_piece_B->i_BLy ) < p_sys->i_magnet_accuracy )
{
i_dx = ps_piece_A->i_TRx - ps_piece_B->i_TLx + ps_piece_A->i_step_x_x;
i_dy = ps_piece_A->i_TRy - ps_piece_B->i_TLy;
if (!ps_piece_B->b_finished)
puzzle_move_group( p_filter, i_piece_B, i_dx, i_dy);
else
puzzle_move_group( p_filter, i_piece_A, -i_dx, -i_dy);
uint32_t i_group_ID = ps_piece_B->i_group_ID;
for (uint32_t i_for = 0; i_for < p_sys->s_allocated.i_pieces_nbr; i_for++)
if ( p_sys->ps_pieces[i_for].i_group_ID == i_group_ID)
p_sys->ps_pieces[i_for].i_group_ID = p_sys->ps_pieces[i_piece_A].i_group_ID;
}
}
else if ( abs(ps_piece_A->i_OLx - ps_piece_B->i_OLx ) < 3 )
{
if ( abs(ps_piece_A->i_OBy - ps_piece_B->i_OTy + 1 ) < 3
&& abs( ps_piece_B->i_TLx - ps_piece_A->i_BLx ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_B->i_TLy - 1 - ps_piece_A->i_BLy ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_B->i_TRx - ps_piece_A->i_BRx ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_B->i_TRy - 1 - ps_piece_A->i_BRy ) < p_sys->i_magnet_accuracy )
{
i_dx = ps_piece_A->i_BLx - ps_piece_B->i_TLx;
i_dy = ps_piece_A->i_BLy - ps_piece_B->i_TLy + ps_piece_A->i_step_y_y;
if (!ps_piece_B->b_finished)
puzzle_move_group( p_filter, i_piece_B, i_dx, i_dy);
else
puzzle_move_group( p_filter, i_piece_A, -i_dx, -i_dy);
uint32_t i_group_ID = ps_piece_B->i_group_ID;
for (uint32_t i_for = 0; i_for < p_sys->s_allocated.i_pieces_nbr; i_for++)
if ( p_sys->ps_pieces[i_for].i_group_ID == i_group_ID)
p_sys->ps_pieces[i_for].i_group_ID = p_sys->ps_pieces[i_piece_A].i_group_ID;
}
}
}
if ( abs( ps_piece_A->i_OTy - ps_piece_B->i_OTy ) < 3 )
{
if ( abs( ps_piece_A->i_ORx - ps_piece_B->i_OLx + 1 ) < 3
&& abs( ps_piece_A->i_TRx - ps_piece_B->i_TLx + 1 ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_A->i_TRy - ps_piece_B->i_TLy ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_A->i_BRx - ps_piece_B->i_BLx + 1 ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_A->i_BRy - ps_piece_B->i_BLy ) < p_sys->i_magnet_accuracy )
{
ps_piece_B->i_left_shape = 0;
ps_piece_A->i_right_shape = 6;
}
}
else if ( abs( ps_piece_A->i_OLx - ps_piece_B->i_OLx )<3 )
{
if ( abs( ps_piece_A->i_OBy - ps_piece_B->i_OTy + 1 )<3
&& abs( ps_piece_B->i_TLx - ps_piece_A->i_BLx ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_B->i_TLy - 1 - ps_piece_A->i_BLy ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_B->i_TRx - ps_piece_A->i_BRx ) < p_sys->i_magnet_accuracy
&& abs( ps_piece_B->i_TRy - 1 - ps_piece_A->i_BRy ) < p_sys->i_magnet_accuracy )
{
ps_piece_B->i_top_shape = 2;
ps_piece_A->i_btm_shape = 4;
}
}
}
}
void puzzle_solve_pces_accuracy( filter_t *p_filter) {
filter_sys_t *p_sys = p_filter->p_sys;
p_sys->i_solve_acc_loop++;
if (p_sys->i_solve_acc_loop >= p_sys->s_allocated.i_pieces_nbr) {
p_sys->i_done_count = p_sys->i_tmp_done_count;
p_sys->i_tmp_done_count = 0;
p_sys->i_solve_acc_loop = 0;
p_sys->b_finished = (p_sys->i_done_count == p_sys->s_allocated.i_pieces_nbr);
}
piece_t *ps_piece = &p_sys->ps_pieces[p_sys->i_solve_acc_loop];
ps_piece->b_finished = false;
if ( ps_piece->i_actual_mirror == 1
&& abs( ps_piece->i_TRx - ps_piece->i_ORx ) < p_sys->i_magnet_accuracy
&& abs( ps_piece->i_TRy - ps_piece->i_OTy ) < p_sys->i_magnet_accuracy
&& abs( ps_piece->i_TLx - ps_piece->i_OLx ) < p_sys->i_magnet_accuracy
&& abs( ps_piece->i_TLy - ps_piece->i_OTy ) < p_sys->i_magnet_accuracy )
{
uint32_t i_group_ID = ps_piece->i_group_ID;
p_sys->i_tmp_done_count++;
for ( uint32_t i = 0; i < p_sys->s_allocated.i_pieces_nbr; i++) {
ps_piece = &p_sys->ps_pieces[i];
if ( ( ps_piece->i_group_ID == i_group_ID ) && ( !ps_piece->b_finished ) ) {
ps_piece->ps_piece_in_plane[0].i_actual_x = ps_piece->i_OLx;
ps_piece->ps_piece_in_plane[0].i_actual_y = ps_piece->i_OTy;
ps_piece->i_actual_mirror = +1;
puzzle_calculate_corners( p_filter, i );
ps_piece->b_finished = true;
}
}
}
}
int puzzle_sort_layers( filter_t *p_filter)
{
filter_sys_t *p_sys = p_filter->p_sys;
int32_t i_idx = 0;
for (uint32_t i_qty = 1; i_qty <= p_sys->s_current_param.i_pieces_nbr; i_qty++) {
for (uint32_t i_pce_loop = 0; i_pce_loop < p_sys->s_current_param.i_pieces_nbr; i_pce_loop++) {
uint32_t i_grp = p_sys->ps_pieces[i_pce_loop].i_group_ID;
if ( p_sys->pi_group_qty[i_grp] == (int32_t)i_qty ) {
bool b_check_ok = true;
for (int32_t i_pce_check = 0; i_pce_check < i_idx; i_pce_check++)
if ( p_sys->ps_pieces_tmp[i_pce_check].i_group_ID == i_grp )
b_check_ok = false;
if ( b_check_ok )
for (uint32_t i_pce = i_pce_loop; i_pce < p_sys->s_current_param.i_pieces_nbr; i_pce++)
if (( p_sys->ps_pieces[i_pce].i_group_ID == i_grp ) && !p_sys->ps_pieces[i_pce].b_finished ) {
memcpy( &p_sys->ps_pieces_tmp[i_idx], &p_sys->ps_pieces[i_pce], sizeof(piece_t));
i_idx++;
}
}
}
for (uint32_t i_pce_loop = 0; i_pce_loop < p_sys->s_current_param.i_pieces_nbr; i_pce_loop++) {
uint32_t i_grp = p_sys->ps_pieces[i_pce_loop].i_group_ID;
if ( p_sys->pi_group_qty[i_grp] == (int32_t)i_qty ) {
bool b_check_ok = true;
for (int32_t i_pce_check = 0; i_pce_check < i_idx; i_pce_check++)
if ( p_sys->ps_pieces_tmp[i_pce_check].i_group_ID == i_grp && p_sys->ps_pieces_tmp[i_pce_check].b_finished )
b_check_ok = false;
if ( b_check_ok )
for (uint32_t i_pce = i_pce_loop; i_pce < p_sys->s_current_param.i_pieces_nbr; i_pce++)
if (( p_sys->ps_pieces[i_pce].i_group_ID == i_grp ) && p_sys->ps_pieces[i_pce].b_finished ) {
memcpy( &p_sys->ps_pieces_tmp[i_idx], &p_sys->ps_pieces[i_pce], sizeof(piece_t));
i_idx++;
}
}
}
}
free( p_sys->ps_pieces );
p_sys->ps_pieces = p_sys->ps_pieces_tmp;
p_sys->ps_pieces_tmp = malloc( sizeof( piece_t) * p_sys->s_allocated.i_pieces_nbr );
if (!p_sys->ps_pieces_tmp)
return VLC_ENOMEM;
return VLC_SUCCESS;
}
void puzzle_auto_solve( filter_t *p_filter)
{
filter_sys_t *p_sys = p_filter->p_sys;
if ( p_sys->s_current_param.i_auto_solve_speed < 500 )
return;
if ( --p_sys->i_auto_solve_countdown_val > 0 )
return;
p_sys->i_auto_solve_countdown_val = init_countdown(p_sys->s_current_param.i_auto_solve_speed);
int32_t i_start = ((unsigned)vlc_mrand48()) % p_sys->s_allocated.i_pieces_nbr;
for (uint32_t i_l = 0; i_l < p_sys->s_allocated.i_pieces_nbr; i_l++) {
int32_t i = ( i_l + i_start ) % p_sys->s_allocated.i_pieces_nbr;
if ( !p_sys->ps_pieces[i].b_finished ) {
for (uint32_t j = 0; j < p_sys->s_allocated.i_pieces_nbr; j++) {
if ( p_sys->ps_pieces[j].i_group_ID == p_sys->ps_pieces[i].i_group_ID ) {
p_sys->ps_pieces[j].i_actual_angle = 0;
p_sys->ps_pieces[j].i_actual_mirror = +1;
p_sys->ps_pieces[j].ps_piece_in_plane[0].i_actual_x = p_sys->ps_pieces[j].ps_piece_in_plane[0].i_original_x;
p_sys->ps_pieces[j].ps_piece_in_plane[0].i_actual_y = p_sys->ps_pieces[j].ps_piece_in_plane[0].i_original_y;
puzzle_calculate_corners( p_filter, j );
}
}
break;
}
}
}
void puzzle_auto_shuffle( filter_t *p_filter)
{
filter_sys_t *p_sys = p_filter->p_sys;
if ( p_sys->s_current_param.i_auto_shuffle_speed < 500 )
return;
if ( --p_sys->i_auto_shuffle_countdown_val > 0 )
return;
p_sys->i_auto_shuffle_countdown_val = init_countdown(p_sys->s_current_param.i_auto_shuffle_speed);
int32_t i_start = ((unsigned)vlc_mrand48()) % p_sys->s_allocated.i_pieces_nbr;
for (uint32_t i_l = 0; i_l < p_sys->s_allocated.i_pieces_nbr; i_l++){
int32_t i = ( i_l + i_start ) % p_sys->s_allocated.i_pieces_nbr;
if ( p_sys->pi_group_qty[p_sys->ps_pieces[i].i_group_ID] > 1 ) {
uint32_t i_new_group;
for ( i_new_group = 0 ; i_new_group < p_sys->s_allocated.i_pieces_nbr ; i_new_group ++ )
if ( p_sys->pi_group_qty[i_new_group] == 0 )
break;
p_sys->ps_pieces[i].i_group_ID = i_new_group;
p_sys->ps_pieces[i].b_finished = false;
switch ( p_sys->s_current_param.i_rotate )
{
case 1:
puzzle_rotate_pce( p_filter, i, (( (unsigned) vlc_mrand48()) % ( 2 ) ) * 2, p_sys->ps_pieces[i].i_center_x, p_sys->ps_pieces[i].i_center_y, false );
break;
case 2:
puzzle_rotate_pce( p_filter, i, (( (unsigned) vlc_mrand48()) % ( 4 ) ), p_sys->ps_pieces[i].i_center_x, p_sys->ps_pieces[i].i_center_y, false );
break;
case 3:
puzzle_rotate_pce( p_filter, i, (( (unsigned) vlc_mrand48()) % ( 8 ) ), p_sys->ps_pieces[i].i_center_x, p_sys->ps_pieces[i].i_center_y, false );
break;
}
p_sys->ps_pieces[i].ps_piece_in_plane[0].i_actual_x =
p_sys->ps_desk_planes[0].i_border_width
+ ( (unsigned) vlc_mrand48()) % ( p_sys->ps_desk_planes[0].i_width - 2*p_sys->ps_desk_planes[0].i_border_width - p_sys->ps_pieces[i].ps_piece_in_plane[0].i_width)
+ p_sys->ps_pieces[i].ps_piece_in_plane[0].i_width / 2 * ( 1 - p_sys->ps_pieces[i].i_step_x_x )
- (p_sys->ps_pieces[i].ps_piece_in_plane[0].i_lines / 2) * p_sys->ps_pieces[i].i_step_y_x;
p_sys->ps_pieces[i].ps_piece_in_plane[0].i_actual_y =
p_sys->ps_desk_planes[0].i_border_lines
+ ( (unsigned) vlc_mrand48()) % ( p_sys->ps_desk_planes[0].i_lines - 2*p_sys->ps_desk_planes[0].i_border_lines - p_sys->ps_pieces[i].ps_piece_in_plane[0].i_lines)
+ p_sys->ps_pieces[i].ps_piece_in_plane[0].i_lines / 2 * ( 1 - p_sys->ps_pieces[i].i_step_y_y )
- (p_sys->ps_pieces[i].ps_piece_in_plane[0].i_width / 2) * p_sys->ps_pieces[i].i_step_x_y;
uint32_t i_left_pce = 0;
uint32_t i_right_pce = 6;
uint32_t i_top_pce = 2;
uint32_t i_btm_pce = 4;
uint32_t i_pce = 0;
for (int32_t i_row = 0; i_row < p_sys->s_allocated.i_rows; i_row++)
for (int32_t i_col = 0; i_col < p_sys->s_allocated.i_cols; i_col++) {
if (p_sys->ps_pieces[i].i_original_row == p_sys->ps_pieces[i_pce].i_original_row) {
if (p_sys->ps_pieces[i].i_original_col == p_sys->ps_pieces[i_pce].i_original_col - 1)
i_right_pce = i_pce;
else if (p_sys->ps_pieces[i].i_original_col == p_sys->ps_pieces[i_pce].i_original_col + 1)
i_left_pce = i_pce;
}
else if (p_sys->ps_pieces[i].i_original_col == p_sys->ps_pieces[i_pce].i_original_col) {
if (p_sys->ps_pieces[i].i_original_row == p_sys->ps_pieces[i_pce].i_original_row - 1)
i_btm_pce = i_pce;
else if (p_sys->ps_pieces[i].i_original_row == p_sys->ps_pieces[i_pce].i_original_row + 1)
i_top_pce = i_pce;
}
i_pce++;
}
if ((p_sys->ps_pieces[i].i_left_shape == 0) && (p_sys->ps_pieces[i].i_original_col != 0)) {
p_sys->ps_pieces[i_left_pce].i_right_shape = 6 + 8 + 8*(( (unsigned) vlc_mrand48()) % ( SHAPES_QTY ) ) + (vlc_mrand48() & 0x01);
p_sys->ps_pieces[i].i_left_shape = (p_sys->ps_pieces[i_left_pce].i_right_shape - 6 ) ^ 0x01;
}
if ((p_sys->ps_pieces[i].i_right_shape == 6) && (p_sys->ps_pieces[i].i_original_col != p_sys->s_allocated.i_cols-1)) {
p_sys->ps_pieces[i].i_right_shape = 6 + 8 + 8*(( (unsigned) vlc_mrand48()) % ( SHAPES_QTY ) ) + (vlc_mrand48() & 0x01);
p_sys->ps_pieces[i_right_pce].i_left_shape = (p_sys->ps_pieces[i].i_right_shape - 6 ) ^ 0x01;
}
if ((p_sys->ps_pieces[i].i_top_shape == 2) && (p_sys->ps_pieces[i].i_original_row != 0)) {
p_sys->ps_pieces[i_top_pce].i_btm_shape = 4 + 8 + 8*(( (unsigned) vlc_mrand48()) % ( SHAPES_QTY ) ) + (vlc_mrand48() & 0x01);
p_sys->ps_pieces[i].i_top_shape = (p_sys->ps_pieces[i_top_pce].i_btm_shape - 2 ) ^ 0x01;
}
if ((p_sys->ps_pieces[i].i_btm_shape == 4) && (p_sys->ps_pieces[i].i_original_row != p_sys->s_allocated.i_rows-1)) {
p_sys->ps_pieces[i].i_btm_shape = 4 + 8 + 8*(( (unsigned) vlc_mrand48()) % ( SHAPES_QTY ) ) + (vlc_mrand48() & 0x01);
p_sys->ps_pieces[i_btm_pce].i_top_shape = (p_sys->ps_pieces[i].i_btm_shape - 2 ) ^ 0x01;
}
puzzle_calculate_corners( p_filter, i );
break;
}
}
}
save_game_t* puzzle_save(filter_t *p_filter)
{
filter_sys_t *p_sys = p_filter->p_sys;
save_game_t *ps_save_game = calloc(1, sizeof(*ps_save_game));
if (!ps_save_game)
return NULL;
ps_save_game->i_cols = p_sys->s_allocated.i_cols;
ps_save_game->i_rows = p_sys->s_allocated.i_rows;
ps_save_game->i_rotate = p_sys->s_allocated.i_rotate;
ps_save_game->ps_pieces = calloc( ps_save_game->i_cols * ps_save_game->i_rows , sizeof(*ps_save_game->ps_pieces));
if (!ps_save_game->ps_pieces) {
free(ps_save_game);
return NULL;
}
int32_t i_border_width = p_sys->ps_desk_planes[0].i_border_width;
int32_t i_border_lines = p_sys->ps_desk_planes[0].i_border_lines;
for (int32_t i_pce = 0; i_pce < ps_save_game->i_cols * ps_save_game->i_rows; i_pce++) {
ps_save_game->ps_pieces[i_pce].i_original_row = p_sys->ps_pieces[i_pce].i_original_row;
ps_save_game->ps_pieces[i_pce].i_original_col = p_sys->ps_pieces[i_pce].i_original_col;
ps_save_game->ps_pieces[i_pce].i_top_shape = p_sys->ps_pieces[i_pce].i_top_shape;
ps_save_game->ps_pieces[i_pce].i_btm_shape = p_sys->ps_pieces[i_pce].i_btm_shape;
ps_save_game->ps_pieces[i_pce].i_right_shape = p_sys->ps_pieces[i_pce].i_right_shape;
ps_save_game->ps_pieces[i_pce].i_left_shape = p_sys->ps_pieces[i_pce].i_left_shape;
ps_save_game->ps_pieces[i_pce].f_pos_x =(p_sys->ps_pieces[i_pce].ps_piece_in_plane[0].i_actual_x - i_border_width ) / ((float)p_sys->ps_desk_planes[0].i_width - 2*i_border_width);
ps_save_game->ps_pieces[i_pce].f_pos_y =(p_sys->ps_pieces[i_pce].ps_piece_in_plane[0].i_actual_y - i_border_lines ) / ((float)p_sys->ps_desk_planes[0].i_lines - 2*i_border_lines);
ps_save_game->ps_pieces[i_pce].i_actual_angle = p_sys->ps_pieces[i_pce].i_actual_angle;
ps_save_game->ps_pieces[i_pce].i_actual_mirror = p_sys->ps_pieces[i_pce].i_actual_mirror;
}
return ps_save_game;
}
void puzzle_load( filter_t *p_filter, save_game_t *ps_save_game)
{
filter_sys_t *p_sys = p_filter->p_sys;
if (p_sys->s_current_param.i_cols != ps_save_game->i_cols
|| p_sys->s_allocated.i_rows != ps_save_game->i_rows
|| p_sys->s_allocated.i_rotate != ps_save_game->i_rotate)
return;
int32_t i_border_width = p_sys->ps_desk_planes[0].i_border_width;
int32_t i_border_lines = p_sys->ps_desk_planes[0].i_border_lines;
for (uint32_t i_pce=0; i_pce < p_sys->s_allocated.i_pieces_nbr; i_pce++) {
for (uint32_t i=0; i < p_sys->s_allocated.i_pieces_nbr; i++)
if ( p_sys->ps_pieces[i].i_original_row == ps_save_game->ps_pieces[i_pce].i_original_row
&& p_sys->ps_pieces[i].i_original_col == ps_save_game->ps_pieces[i_pce].i_original_col )
{
p_sys->ps_pieces[i].ps_piece_in_plane[0].i_actual_x = i_border_width
+ ((float)p_sys->ps_desk_planes[0].i_width - 2 * i_border_width)
* ps_save_game->ps_pieces[i_pce].f_pos_x;
p_sys->ps_pieces[i].ps_piece_in_plane[0].i_actual_y = i_border_lines
+ ((float)p_sys->ps_desk_planes[0].i_lines - 2 * i_border_lines)
* ps_save_game->ps_pieces[i_pce].f_pos_y;
p_sys->ps_pieces[i].i_top_shape = ps_save_game->ps_pieces[i_pce].i_top_shape;
p_sys->ps_pieces[i].i_btm_shape = ps_save_game->ps_pieces[i_pce].i_btm_shape;
p_sys->ps_pieces[i].i_right_shape = ps_save_game->ps_pieces[i_pce].i_right_shape;
p_sys->ps_pieces[i].i_left_shape = ps_save_game->ps_pieces[i_pce].i_left_shape;
p_sys->ps_pieces[i].i_actual_angle = ps_save_game->ps_pieces[i_pce].i_actual_angle;
p_sys->ps_pieces[i].i_actual_mirror = ps_save_game->ps_pieces[i_pce].i_actual_mirror;
p_sys->ps_pieces[i].i_group_ID = i_pce;
p_sys->ps_pieces[i].b_finished = false;
p_sys->ps_pieces[i].ps_piece_in_plane[0].i_actual_x = i_border_width + ((float)p_sys->ps_desk_planes[0].i_width
- 2*i_border_width) * ps_save_game->ps_pieces[i_pce].f_pos_x;
p_sys->ps_pieces[i].ps_piece_in_plane[0].i_actual_y = i_border_lines + ((float)p_sys->ps_desk_planes[0].i_lines
- 2*i_border_lines) * ps_save_game->ps_pieces[i_pce].f_pos_y;
puzzle_calculate_corners( p_filter, i );
break;
}
}
for (uint32_t i_pce=0; i_pce < p_sys->s_allocated.i_pieces_nbr; i_pce++) {
uint32_t i_left_pce = 0;
uint32_t i_right_pce = 6;
uint32_t i_top_pce = 2;
uint32_t i_btm_pce = 4;
uint32_t i_pce_pair = 0;
for (int32_t i_row = 0; i_row < p_sys->s_allocated.i_rows; i_row++)
for (int32_t i_col = 0; i_col < p_sys->s_allocated.i_cols; i_col++) {
if (p_sys->ps_pieces[i_pce].i_original_row == p_sys->ps_pieces[i_pce_pair].i_original_row) {
if (p_sys->ps_pieces[i_pce].i_original_col == p_sys->ps_pieces[i_pce_pair].i_original_col - 1)
i_right_pce = i_pce_pair;
else if (p_sys->ps_pieces[i_pce].i_original_col == p_sys->ps_pieces[i_pce_pair].i_original_col + 1)
i_left_pce = i_pce_pair;
}
else if (p_sys->ps_pieces[i_pce].i_original_col == p_sys->ps_pieces[i_pce_pair].i_original_col) {
if (p_sys->ps_pieces[i_pce].i_original_row == p_sys->ps_pieces[i_pce_pair].i_original_row - 1)
i_btm_pce = i_pce_pair;
else if (p_sys->ps_pieces[i_pce].i_original_row == p_sys->ps_pieces[i_pce_pair].i_original_row + 1)
i_top_pce = i_pce_pair;
}
i_pce_pair++;
}
if ((p_sys->ps_pieces[i_pce].i_left_shape == 0) && (p_sys->ps_pieces[i_pce].i_original_col != 0)) {
p_sys->ps_pieces[i_left_pce].i_right_shape = 6 + 8 + 8*(( (unsigned) vlc_mrand48()) % ( SHAPES_QTY ) ) + (vlc_mrand48() & 0x01);
p_sys->ps_pieces[i_pce].i_left_shape = (p_sys->ps_pieces[i_left_pce].i_right_shape - 6 ) ^ 0x01;
}
if ((p_sys->ps_pieces[i_pce].i_right_shape == 6) && (p_sys->ps_pieces[i_pce].i_original_col != p_sys->s_allocated.i_cols-1)) {
p_sys->ps_pieces[i_pce].i_right_shape = 6 + 8 + 8*(( (unsigned) vlc_mrand48()) % ( SHAPES_QTY ) ) + (vlc_mrand48() & 0x01);
p_sys->ps_pieces[i_right_pce].i_left_shape = (p_sys->ps_pieces[i_pce].i_right_shape - 6 ) ^ 0x01;
}
if ((p_sys->ps_pieces[i_pce].i_top_shape == 2) && (p_sys->ps_pieces[i_pce].i_original_row != 0)) {
p_sys->ps_pieces[i_top_pce].i_btm_shape = 4 + 8 + 8*(( (unsigned) vlc_mrand48()) % ( SHAPES_QTY ) ) + (vlc_mrand48() & 0x01);
p_sys->ps_pieces[i_pce].i_top_shape = (p_sys->ps_pieces[i_top_pce].i_btm_shape - 2 ) ^ 0x01;
}
if ((p_sys->ps_pieces[i_pce].i_btm_shape == 4) && (p_sys->ps_pieces[i_pce].i_original_row != p_sys->s_allocated.i_rows-1)) {
p_sys->ps_pieces[i_pce].i_btm_shape = 4 + 8 + 8*(( (unsigned) vlc_mrand48()) % ( SHAPES_QTY ) ) + (vlc_mrand48() & 0x01);
p_sys->ps_pieces[i_btm_pce].i_top_shape = (p_sys->ps_pieces[i_pce].i_btm_shape - 2 ) ^ 0x01;
}
}
}
