






























































































#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <vlc_codec.h>
#include "synchro.h"









#define synchro_msg_Dbg(p_this, ...) do { if( !p_this->b_quiet ) msg_Generic(p_this->p_dec, VLC_MSG_DBG, __VA_ARGS__); } while (0)








#define synchro_msg_Warn(p_this, ...) do { if( !p_this->b_quiet ) msg_Generic(p_this->p_dec, VLC_MSG_WARN, __VA_ARGS__); } while (0)








#define MAX_PIC_AVERAGE 8

struct decoder_synchro_t
{

decoder_t *p_dec;


int i_frame_rate;
bool b_no_skip;
bool b_quiet;


vlc_tick_t decoding_start;


unsigned int i_n_p, i_n_b;


vlc_tick_t p_tau[4]; 
unsigned int pi_meaningful[4]; 


vlc_tick_t i_render_time;


int i_nb_ref; 
int i_dec_nb_ref; 

int i_trash_nb_ref; 

unsigned int i_eta_p, i_eta_b;
vlc_tick_t backward_pts, current_pts;
int i_current_period; 
int i_backward_period; 




unsigned int i_trashed_pic, i_not_chosen_pic, i_pic;
};


#define DELTA VLC_TICK_FROM_MS(75) 
#define MAX_VALID_TAU VLC_TICK_FROM_MS(300)

#define DEFAULT_NB_P 5
#define DEFAULT_NB_B 1




decoder_synchro_t * decoder_SynchroInit( decoder_t *p_dec, int i_frame_rate )
{
decoder_synchro_t * p_synchro = calloc( 1, sizeof(*p_synchro) );
if( !p_synchro )
return NULL;

p_synchro->p_dec = p_dec;
p_synchro->b_no_skip = !var_InheritBool( p_dec, "skip-frames" );
p_synchro->b_quiet = var_InheritBool( p_dec, "quiet-synchro" );


p_synchro->i_n_p = p_synchro->i_eta_p = DEFAULT_NB_P;
p_synchro->i_n_b = p_synchro->i_eta_b = DEFAULT_NB_B;
memset( p_synchro->p_tau, 0, 4 * sizeof(vlc_tick_t) );
memset( p_synchro->pi_meaningful, 0, 4 * sizeof(unsigned int) );
p_synchro->i_nb_ref = 0;
p_synchro->i_trash_nb_ref = p_synchro->i_dec_nb_ref = 0;
p_synchro->current_pts = 1,
p_synchro->backward_pts = VLC_TICK_INVALID;
p_synchro->i_current_period = p_synchro->i_backward_period = 0;
p_synchro->i_trashed_pic = p_synchro->i_not_chosen_pic =
p_synchro->i_pic = 0;

p_synchro->i_frame_rate = i_frame_rate;

return p_synchro;
}




void decoder_SynchroRelease( decoder_synchro_t * p_synchro )
{
free( p_synchro );
}




void decoder_SynchroReset( decoder_synchro_t * p_synchro )
{
p_synchro->i_nb_ref = 0;
p_synchro->i_trash_nb_ref = p_synchro->i_dec_nb_ref = 0;
}




bool decoder_SynchroChoose( decoder_synchro_t * p_synchro, int i_coding_type,
vlc_tick_t i_render_time, bool b_low_delay )
{
#define TAU_PRIME( coding_type ) (p_synchro->p_tau[(coding_type)] + (p_synchro->p_tau[(coding_type)] >> 1) + p_synchro->i_render_time)


#define S (*p_synchro)
vlc_tick_t now, period;
vlc_tick_t pts;
bool b_decode = 0;
float i_current_rate;

if ( p_synchro->b_no_skip )
return 1;

i_current_rate = decoder_GetDisplayRate( p_synchro->p_dec );

now = vlc_tick_now();
period = vlc_tick_from_samples(1001, p_synchro->i_frame_rate) * i_current_rate;

p_synchro->i_render_time = i_render_time;

switch( i_coding_type )
{
case I_CODING_TYPE:
if( b_low_delay )
{
pts = decoder_GetDisplayDate( p_synchro->p_dec, now, S.current_pts );
}
else if( S.backward_pts != VLC_TICK_INVALID )
{
pts = decoder_GetDisplayDate( p_synchro->p_dec, now, S.backward_pts );
}
else
{





pts = decoder_GetDisplayDate( p_synchro->p_dec, now, S.current_pts ) + period * (S.i_n_b + 2);
}

if( (1 + S.i_n_p * (S.i_n_b + 1)) * period > S.p_tau[I_CODING_TYPE] )
{
b_decode = 1;
}
else
{
b_decode = (pts - now) > (TAU_PRIME(I_CODING_TYPE) + DELTA);
}
if( pts == VLC_TICK_INVALID )
b_decode = 1;

if( !b_decode )
{
synchro_msg_Warn( p_synchro,
"synchro trashing I (%"PRId64")", pts - now );
}
break;

case P_CODING_TYPE:
if( b_low_delay )
{
pts = decoder_GetDisplayDate( p_synchro->p_dec, now, S.current_pts );
}
else if( S.backward_pts != VLC_TICK_INVALID )
{
pts = decoder_GetDisplayDate( p_synchro->p_dec, now, S.backward_pts );
}
else
{
pts = decoder_GetDisplayDate( p_synchro->p_dec, now, S.current_pts + period * (S.i_n_b + 1) );
}

if( p_synchro->i_nb_ref < 1 )
{
b_decode = 0;
}
else if( (1 + S.i_n_p * (S.i_n_b + 1)) * period >
S.p_tau[I_CODING_TYPE] )
{
if( (S.i_n_b + 1) * period > S.p_tau[P_CODING_TYPE] )
{

b_decode = (pts - now > 0);
}
else
{
b_decode = (pts - now) > (TAU_PRIME(P_CODING_TYPE) + DELTA);

b_decode &= (pts - now
+ period
* ( (S.i_n_p - S.i_eta_p) * (1 + S.i_n_b) - 1 ))
> (TAU_PRIME(P_CODING_TYPE)
+ TAU_PRIME(I_CODING_TYPE) + DELTA);
}
}
else
{
b_decode = 0;
}
if( p_synchro->i_nb_ref >= 1 && pts == VLC_TICK_INVALID )
b_decode = 1;
break;

case B_CODING_TYPE:
pts = decoder_GetDisplayDate( p_synchro->p_dec, now, S.current_pts );

if( p_synchro->i_nb_ref < 2 )
{
b_decode = 0;
}
else if( (S.i_n_b + 1) * period > S.p_tau[P_CODING_TYPE] )
{
b_decode = (pts - now) > (TAU_PRIME(B_CODING_TYPE) + DELTA);
}
else
{
b_decode = 0;
}
if( p_synchro->i_nb_ref >= 2 && pts == VLC_TICK_INVALID )
b_decode = 1;
break;
}

if( !b_decode )
{
S.i_not_chosen_pic++;
}
return( b_decode );
#undef S
#undef TAU_PRIME
}




void decoder_SynchroTrash( decoder_synchro_t * p_synchro )
{
p_synchro->i_trashed_pic++;
p_synchro->i_nb_ref = p_synchro->i_trash_nb_ref;
}




void decoder_SynchroDecode( decoder_synchro_t * p_synchro )
{
p_synchro->decoding_start = vlc_tick_now();
p_synchro->i_nb_ref = p_synchro->i_dec_nb_ref;
}




void decoder_SynchroEnd( decoder_synchro_t * p_synchro, int i_coding_type,
bool b_garbage )
{
vlc_tick_t tau;

if( b_garbage )
return;

tau = vlc_tick_now() - p_synchro->decoding_start;



if( tau < 3 * p_synchro->p_tau[i_coding_type] ||
( !p_synchro->pi_meaningful[i_coding_type] && tau < MAX_VALID_TAU ) )
{

p_synchro->p_tau[i_coding_type] =
(p_synchro->pi_meaningful[i_coding_type]
* p_synchro->p_tau[i_coding_type] + tau)
/ (p_synchro->pi_meaningful[i_coding_type] + 1);
if( p_synchro->pi_meaningful[i_coding_type] < MAX_PIC_AVERAGE )
{
p_synchro->pi_meaningful[i_coding_type]++;
}
}
}




vlc_tick_t decoder_SynchroDate( decoder_synchro_t * p_synchro )
{

return p_synchro->current_pts;
}




void decoder_SynchroNewPicture( decoder_synchro_t * p_synchro, int i_coding_type,
int i_repeat_field, vlc_tick_t next_pts,
vlc_tick_t next_dts, bool b_low_delay )
{
vlc_tick_t period = vlc_tick_from_samples(1001, p_synchro->i_frame_rate);
#if 0
vlc_tick_t now = vlc_tick_now();
#endif

switch( i_coding_type )
{
case I_CODING_TYPE:
if( p_synchro->i_eta_p && p_synchro->i_eta_p != p_synchro->i_n_p )
{
#if 0
synchro_msg_Dbg( p_synchro,
"stream periodicity changed from P[%d] to P[%d]",
p_synchro->i_n_p, p_synchro->i_eta_p );
#endif
p_synchro->i_n_p = p_synchro->i_eta_p;
}
p_synchro->i_eta_p = p_synchro->i_eta_b = 0;
p_synchro->i_trash_nb_ref = 0;
if( p_synchro->i_nb_ref < 2 )
p_synchro->i_dec_nb_ref = p_synchro->i_nb_ref + 1;
else
p_synchro->i_dec_nb_ref = p_synchro->i_nb_ref;

#if 0
synchro_msg_Dbg( p_synchro, "I(%"PRId64") P(%"PRId64")[%d] B(%"PRId64")"
"[%d] YUV(%"PRId64") : trashed %d:%d/%d",
p_synchro->p_tau[I_CODING_TYPE],
p_synchro->p_tau[P_CODING_TYPE],
p_synchro->i_n_p,
p_synchro->p_tau[B_CODING_TYPE],
p_synchro->i_n_b,
p_synchro->i_render_time,
p_synchro->i_not_chosen_pic,
p_synchro->i_trashed_pic -
p_synchro->i_not_chosen_pic,
p_synchro->i_pic );
p_synchro->i_trashed_pic = p_synchro->i_not_chosen_pic
= p_synchro->i_pic = 0;
#else
if( p_synchro->i_pic >= 100 )
{
if( p_synchro->i_trashed_pic != 0 )
synchro_msg_Dbg( p_synchro, "decoded %d/%d pictures",
p_synchro->i_pic
- p_synchro->i_trashed_pic,
p_synchro->i_pic );
p_synchro->i_trashed_pic = p_synchro->i_not_chosen_pic
= p_synchro->i_pic = 0;
}
#endif
break;

case P_CODING_TYPE:
p_synchro->i_eta_p++;
if( p_synchro->i_eta_b
&& p_synchro->i_eta_b != p_synchro->i_n_b )
{
#if 0
synchro_msg_Dbg( p_synchro,
"stream periodicity changed from B[%d] to B[%d]",
p_synchro->i_n_b, p_synchro->i_eta_b );
#endif
p_synchro->i_n_b = p_synchro->i_eta_b;
}
p_synchro->i_eta_b = 0;
p_synchro->i_dec_nb_ref = 2;
p_synchro->i_trash_nb_ref = 0;
break;

case B_CODING_TYPE:
p_synchro->i_eta_b++;
p_synchro->i_dec_nb_ref = p_synchro->i_trash_nb_ref
= p_synchro->i_nb_ref;
break;
}

p_synchro->current_pts += p_synchro->i_current_period
* (period >> 1);

#define PTS_THRESHOLD (period >> 2)
if( i_coding_type == B_CODING_TYPE || b_low_delay )
{



p_synchro->i_current_period = i_repeat_field;

if( next_pts != VLC_TICK_INVALID )
{
if( next_pts - p_synchro->current_pts
> PTS_THRESHOLD
|| p_synchro->current_pts - next_pts
> PTS_THRESHOLD )
{
synchro_msg_Warn( p_synchro, "decoder synchro warning: pts != "
"current_date (%"PRId64")",
p_synchro->current_pts
- next_pts );
}
p_synchro->current_pts = next_pts;
}
}
else
{
p_synchro->i_current_period = p_synchro->i_backward_period;
p_synchro->i_backward_period = i_repeat_field;

if( p_synchro->backward_pts != VLC_TICK_INVALID )
{
if( next_dts != VLC_TICK_INVALID &&
(next_dts - p_synchro->backward_pts
> PTS_THRESHOLD
|| p_synchro->backward_pts - next_dts
> PTS_THRESHOLD) )
{
synchro_msg_Warn( p_synchro, "backward_pts != dts (%"PRId64")",
next_dts
- p_synchro->backward_pts );
}
if( p_synchro->backward_pts - p_synchro->current_pts
> PTS_THRESHOLD
|| p_synchro->current_pts - p_synchro->backward_pts
> PTS_THRESHOLD )
{
synchro_msg_Warn( p_synchro,
"backward_pts != current_pts (%"PRId64")",
p_synchro->current_pts
- p_synchro->backward_pts );
}
p_synchro->current_pts = p_synchro->backward_pts;
p_synchro->backward_pts = VLC_TICK_INVALID;
}
else if( next_dts != VLC_TICK_INVALID )
{
if( next_dts - p_synchro->current_pts
> PTS_THRESHOLD
|| p_synchro->current_pts - next_dts
> PTS_THRESHOLD )
{
synchro_msg_Warn( p_synchro, "dts != current_pts (%"PRId64")",
p_synchro->current_pts
- next_dts );
}

p_synchro->current_pts = next_dts;
}

if( next_pts != VLC_TICK_INVALID )
{

p_synchro->backward_pts = next_pts;
}
}
#undef PTS_THRESHOLD

#if 0

if( p_synchro->current_pts + DEFAULT_PTS_DELAY < now )
{


synchro_msg_Warn( p_synchro, "PTS << now (%"PRId64"), resetting",
now - p_synchro->current_pts - DEFAULT_PTS_DELAY );
p_synchro->current_pts = now + DEFAULT_PTS_DELAY;
}
if( p_synchro->backward_pts != VLC_TICK_INVALID
&& p_synchro->backward_pts + DEFAULT_PTS_DELAY < now )
{

p_synchro->backward_pts = VLC_TICK_INVALID;
}
#endif

p_synchro->i_pic++;
}
