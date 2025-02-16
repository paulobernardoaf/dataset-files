#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdarg.h>
#include <assert.h>
#include <vlc_common.h>
#include <vlc_demux.h>
#include <vlc_network.h>
#include <vlc_plugin.h>
#include <vlc_dialog.h>
#include <vlc_aout.h> 
#include "rtp.h"
#if defined(HAVE_SRTP)
#include "srtp.h"
#include <gcrypt.h>
#include <vlc_gcrypt.h>
#endif
#define RTCP_PORT_TEXT N_("RTCP (local) port")
#define RTCP_PORT_LONGTEXT N_( "RTCP packets will be received on this transport protocol port. " "If zero, multiplexed RTP/RTCP is used.")
#define SRTP_KEY_TEXT N_("SRTP key (hexadecimal)")
#define SRTP_KEY_LONGTEXT N_( "RTP packets will be authenticated and deciphered ""with this Secure RTP master shared secret key. ""This must be a 32-character-long hexadecimal string.")
#define SRTP_SALT_TEXT N_("SRTP salt (hexadecimal)")
#define SRTP_SALT_LONGTEXT N_( "Secure RTP requires a (non-secret) master salt value. " "This must be a 28-character-long hexadecimal string.")
#define RTP_MAX_SRC_TEXT N_("Maximum RTP sources")
#define RTP_MAX_SRC_LONGTEXT N_( "How many distinct active RTP sources are allowed at a time." )
#define RTP_TIMEOUT_TEXT N_("RTP source timeout (sec)")
#define RTP_TIMEOUT_LONGTEXT N_( "How long to wait for any packet before a source is expired.")
#define RTP_MAX_DROPOUT_TEXT N_("Maximum RTP sequence number dropout")
#define RTP_MAX_DROPOUT_LONGTEXT N_( "RTP packets will be discarded if they are too much ahead (i.e. in the " "future) by this many packets from the last received packet." )
#define RTP_MAX_MISORDER_TEXT N_("Maximum RTP sequence number misordering")
#define RTP_MAX_MISORDER_LONGTEXT N_( "RTP packets will be discarded if they are too far behind (i.e. in the " "past) by this many packets from the last received packet." )
#define RTP_DYNAMIC_PT_TEXT N_("RTP payload format assumed for dynamic " "payloads")
#define RTP_DYNAMIC_PT_LONGTEXT N_( "This payload format will be assumed for dynamic payload types " "(between 96 and 127) if it can't be determined otherwise with " "out-of-band mappings (SDP)" )
static const char *const dynamic_pt_list[] = { "theora" };
static const char *const dynamic_pt_list_text[] = { "Theora Encoded Video" };
static int Open (vlc_object_t *);
static void Close (vlc_object_t *);
vlc_module_begin ()
set_shortname (N_("RTP"))
set_description (N_("Real-Time Protocol (RTP) input"))
set_category (CAT_INPUT)
set_subcategory (SUBCAT_INPUT_DEMUX)
set_capability ("access", 0)
set_callbacks (Open, Close)
add_integer ("rtcp-port", 0, RTCP_PORT_TEXT,
RTCP_PORT_LONGTEXT, false)
change_integer_range (0, 65535)
change_safe ()
#if defined(HAVE_SRTP)
add_string ("srtp-key", "",
SRTP_KEY_TEXT, SRTP_KEY_LONGTEXT, false)
change_safe ()
add_string ("srtp-salt", "",
SRTP_SALT_TEXT, SRTP_SALT_LONGTEXT, false)
change_safe ()
#endif
add_integer ("rtp-max-src", 1, RTP_MAX_SRC_TEXT,
RTP_MAX_SRC_LONGTEXT, true)
change_integer_range (1, 255)
add_integer ("rtp-timeout", 5, RTP_TIMEOUT_TEXT,
RTP_TIMEOUT_LONGTEXT, true)
add_integer ("rtp-max-dropout", 3000, RTP_MAX_DROPOUT_TEXT,
RTP_MAX_DROPOUT_LONGTEXT, true)
change_integer_range (0, 32767)
add_integer ("rtp-max-misorder", 100, RTP_MAX_MISORDER_TEXT,
RTP_MAX_MISORDER_LONGTEXT, true)
change_integer_range (0, 32767)
add_string ("rtp-dynamic-pt", NULL, RTP_DYNAMIC_PT_TEXT,
RTP_DYNAMIC_PT_LONGTEXT, true)
change_string_list (dynamic_pt_list, dynamic_pt_list_text)
add_shortcut ("dccp", "rtptcp", 
"rtp", "udplite")
vlc_module_end ()
#if !defined(IPPROTO_DCCP)
#define IPPROTO_DCCP 33 
#endif
#if !defined(IPPROTO_UDPLITE)
#define IPPROTO_UDPLITE 136 
#endif
static int Control (demux_t *, int i_query, va_list args);
static int extract_port (char **phost);
static int Open (vlc_object_t *obj)
{
demux_t *demux = (demux_t *)obj;
int tp; 
if (demux->out == NULL)
return VLC_EGENERIC;
if (!strcasecmp(demux->psz_name, "dccp"))
tp = IPPROTO_DCCP;
else
if (!strcasecmp(demux->psz_name, "rtptcp"))
tp = IPPROTO_TCP;
else
if (!strcasecmp(demux->psz_name, "rtp"))
tp = IPPROTO_UDP;
else
if (!strcasecmp(demux->psz_name, "udplite"))
tp = IPPROTO_UDPLITE;
else
return VLC_EGENERIC;
char *tmp = strdup (demux->psz_location);
if (tmp == NULL)
return VLC_ENOMEM;
char *shost;
char *dhost = strchr (tmp, '@');
if (dhost != NULL)
{
*(dhost++) = '\0';
shost = tmp;
}
else
{
dhost = tmp;
shost = NULL;
}
int sport = 0, dport = 0;
if (shost != NULL)
sport = extract_port (&shost);
if (dhost != NULL)
dport = extract_port (&dhost);
if (dport == 0)
dport = 5004; 
int rtcp_dport = var_CreateGetInteger (obj, "rtcp-port");
int fd = -1, rtcp_fd = -1;
switch (tp)
{
case IPPROTO_UDP:
case IPPROTO_UDPLITE:
fd = net_OpenDgram (obj, dhost, dport, shost, sport, tp);
if (fd == -1)
break;
if (rtcp_dport > 0) 
rtcp_fd = net_OpenDgram (obj, dhost, rtcp_dport, shost, 0, tp);
break;
case IPPROTO_DCCP:
#if !defined(SOCK_DCCP)
#if defined(__linux__)
#define SOCK_DCCP 6
#endif
#endif
#if defined(SOCK_DCCP)
var_Create (obj, "dccp-service", VLC_VAR_STRING);
var_SetString (obj, "dccp-service", "RTPV"); 
fd = net_Connect (obj, dhost, dport, SOCK_DCCP, tp);
#else
msg_Err (obj, "DCCP support not included");
#endif
break;
case IPPROTO_TCP:
fd = net_Connect (obj, dhost, dport, SOCK_STREAM, tp);
break;
}
free (tmp);
if (fd == -1)
return VLC_EGENERIC;
net_SetCSCov (fd, -1, 12);
demux_sys_t *p_sys = malloc (sizeof (*p_sys));
if (p_sys == NULL)
{
net_Close (fd);
if (rtcp_fd != -1)
net_Close (rtcp_fd);
return VLC_EGENERIC;
}
p_sys->chained_demux = NULL;
#if defined(HAVE_SRTP)
p_sys->srtp = NULL;
#endif
p_sys->fd = fd;
p_sys->rtcp_fd = rtcp_fd;
p_sys->max_src = var_CreateGetInteger (obj, "rtp-max-src");
p_sys->timeout = vlc_tick_from_sec( var_CreateGetInteger (obj, "rtp-timeout") );
p_sys->max_dropout = var_CreateGetInteger (obj, "rtp-max-dropout");
p_sys->max_misorder = var_CreateGetInteger (obj, "rtp-max-misorder");
p_sys->thread_ready = false;
p_sys->autodetect = true;
demux->pf_demux = NULL;
demux->pf_control = Control;
demux->p_sys = p_sys;
p_sys->session = rtp_session_create (demux);
if (p_sys->session == NULL)
goto error;
#if defined(HAVE_SRTP)
char *key = var_CreateGetNonEmptyString (demux, "srtp-key");
if (key)
{
vlc_gcrypt_init ();
p_sys->srtp = srtp_create (SRTP_ENCR_AES_CM, SRTP_AUTH_HMAC_SHA1, 10,
SRTP_PRF_AES_CM, SRTP_RCC_MODE1);
if (p_sys->srtp == NULL)
{
free (key);
goto error;
}
char *salt = var_CreateGetNonEmptyString (demux, "srtp-salt");
int val = srtp_setkeystring (p_sys->srtp, key, salt ? salt : "");
free (salt);
free (key);
if (val)
{
msg_Err (obj, "bad SRTP key/salt combination (%s)",
vlc_strerror_c(val));
goto error;
}
}
#endif
if (vlc_clone (&p_sys->thread,
(tp != IPPROTO_TCP) ? rtp_dgram_thread : rtp_stream_thread,
demux, VLC_THREAD_PRIORITY_INPUT))
goto error;
p_sys->thread_ready = true;
return VLC_SUCCESS;
error:
Close (obj);
return VLC_EGENERIC;
}
static void Close (vlc_object_t *obj)
{
demux_t *demux = (demux_t *)obj;
demux_sys_t *p_sys = demux->p_sys;
if (p_sys->thread_ready)
{
vlc_cancel (p_sys->thread);
vlc_join (p_sys->thread, NULL);
}
#if defined(HAVE_SRTP)
if (p_sys->srtp)
srtp_destroy (p_sys->srtp);
#endif
if (p_sys->session)
rtp_session_destroy (demux, p_sys->session);
if (p_sys->rtcp_fd != -1)
net_Close (p_sys->rtcp_fd);
net_Close (p_sys->fd);
free (p_sys);
}
static int extract_port (char **phost)
{
char *host = *phost, *port;
if (host[0] == '[')
{
host = ++*phost; 
port = strchr (host, ']');
if (port)
*port++ = '\0'; 
}
else
port = strchr (host, ':');
if (port == NULL)
return 0;
*port++ = '\0'; 
return atoi (port);
}
static int Control (demux_t *demux, int query, va_list args)
{
demux_sys_t *sys = demux->p_sys;
switch (query)
{
case DEMUX_GET_PTS_DELAY:
{
*va_arg (args, vlc_tick_t *) =
VLC_TICK_FROM_MS( var_InheritInteger (demux, "network-caching") );
return VLC_SUCCESS;
}
case DEMUX_CAN_PAUSE:
case DEMUX_CAN_SEEK:
case DEMUX_CAN_CONTROL_PACE:
{
bool *v = va_arg( args, bool * );
*v = false;
return VLC_SUCCESS;
}
}
if (sys->chained_demux != NULL)
return vlc_demux_chained_ControlVa (sys->chained_demux, query, args);
switch (query)
{
case DEMUX_GET_POSITION:
{
float *v = va_arg (args, float *);
*v = 0.;
return VLC_SUCCESS;
}
case DEMUX_GET_LENGTH:
case DEMUX_GET_TIME:
{
*va_arg (args, vlc_tick_t *) = 0;
return VLC_SUCCESS;
}
}
return VLC_EGENERIC;
}
void *codec_init (demux_t *demux, es_format_t *fmt)
{
if (fmt->i_cat == AUDIO_ES)
aout_FormatPrepare (&fmt->audio);
return es_out_Add (demux->out, fmt);
}
void codec_destroy (demux_t *demux, void *data)
{
if (data)
es_out_Del (demux->out, (es_out_id_t *)data);
}
void codec_decode (demux_t *demux, void *data, block_t *block)
{
if (data)
{
block->i_dts = VLC_TICK_INVALID; 
es_out_SetPCR(demux->out, block->i_pts);
es_out_Send (demux->out, (es_out_id_t *)data, block);
}
else
block_Release (block);
}
static void *stream_init (demux_t *demux, const char *name)
{
demux_sys_t *p_sys = demux->p_sys;
if (p_sys->chained_demux != NULL)
return NULL;
p_sys->chained_demux = vlc_demux_chained_New(VLC_OBJECT(demux), name,
demux->out);
return p_sys->chained_demux;
}
static void stream_destroy (demux_t *demux, void *data)
{
demux_sys_t *p_sys = demux->p_sys;
if (data)
{
vlc_demux_chained_Delete(data);
p_sys->chained_demux = NULL;
}
}
static void stream_header (demux_t *demux, void *data, block_t *block)
{
VLC_UNUSED(demux);
VLC_UNUSED(data);
if(block->p_buffer[1] & 0x80) 
{
block->i_flags |= BLOCK_FLAG_DISCONTINUITY;
}
}
static void stream_decode (demux_t *demux, void *data, block_t *block)
{
if (data)
vlc_demux_chained_Send(data, block);
else
block_Release (block);
(void)demux;
}
static void *pcmu_init (demux_t *demux)
{
es_format_t fmt;
es_format_Init (&fmt, AUDIO_ES, VLC_CODEC_MULAW);
fmt.audio.i_rate = 8000;
fmt.audio.i_physical_channels = AOUT_CHAN_CENTER;
return codec_init (demux, &fmt);
}
static void *gsm_init (demux_t *demux)
{
es_format_t fmt;
es_format_Init (&fmt, AUDIO_ES, VLC_CODEC_GSM);
fmt.audio.i_rate = 8000;
fmt.audio.i_physical_channels = AOUT_CHAN_CENTER;
return codec_init (demux, &fmt);
}
static void *pcma_init (demux_t *demux)
{
es_format_t fmt;
es_format_Init (&fmt, AUDIO_ES, VLC_CODEC_ALAW);
fmt.audio.i_rate = 8000;
fmt.audio.i_physical_channels = AOUT_CHAN_CENTER;
return codec_init (demux, &fmt);
}
static void *l16s_init (demux_t *demux)
{
es_format_t fmt;
es_format_Init (&fmt, AUDIO_ES, VLC_CODEC_S16B);
fmt.audio.i_rate = 44100;
fmt.audio.i_physical_channels = AOUT_CHANS_STEREO;
return codec_init (demux, &fmt);
}
static void *l16m_init (demux_t *demux)
{
es_format_t fmt;
es_format_Init (&fmt, AUDIO_ES, VLC_CODEC_S16B);
fmt.audio.i_rate = 44100;
fmt.audio.i_physical_channels = AOUT_CHAN_CENTER;
return codec_init (demux, &fmt);
}
static void *qcelp_init (demux_t *demux)
{
es_format_t fmt;
es_format_Init (&fmt, AUDIO_ES, VLC_CODEC_QCELP);
fmt.audio.i_rate = 8000;
fmt.audio.i_physical_channels = AOUT_CHAN_CENTER;
return codec_init (demux, &fmt);
}
static void *mpa_init (demux_t *demux)
{
es_format_t fmt;
es_format_Init (&fmt, AUDIO_ES, VLC_CODEC_MPGA);
fmt.audio.i_physical_channels = AOUT_CHANS_STEREO;
fmt.b_packetized = false;
return codec_init (demux, &fmt);
}
static void mpa_decode (demux_t *demux, void *data, block_t *block)
{
if (block->i_buffer < 4)
{
block_Release (block);
return;
}
block->i_buffer -= 4; 
block->p_buffer += 4;
codec_decode (demux, data, block);
}
static void *mpv_init (demux_t *demux)
{
es_format_t fmt;
es_format_Init (&fmt, VIDEO_ES, VLC_CODEC_MPGV);
fmt.b_packetized = false;
return codec_init (demux, &fmt);
}
static void mpv_decode (demux_t *demux, void *data, block_t *block)
{
if (block->i_buffer < 4)
{
block_Release (block);
return;
}
block->i_buffer -= 4; 
block->p_buffer += 4;
#if 0
if (block->p_buffer[-3] & 0x4)
{
}
#endif
codec_decode (demux, data, block);
}
static void *ts_init (demux_t *demux)
{
return stream_init (demux, "ts");
}
void rtp_autodetect (demux_t *demux, rtp_session_t *session,
const block_t *block)
{
uint8_t ptype = rtp_ptype (block);
rtp_pt_t pt = {
.init = NULL,
.destroy = codec_destroy,
.header = NULL,
.decode = codec_decode,
.frequency = 0,
.number = ptype,
};
switch (ptype)
{
case 0:
msg_Dbg (demux, "detected G.711 mu-law");
pt.init = pcmu_init;
pt.frequency = 8000;
break;
case 3:
msg_Dbg (demux, "detected GSM");
pt.init = gsm_init;
pt.frequency = 8000;
break;
case 8:
msg_Dbg (demux, "detected G.711 A-law");
pt.init = pcma_init;
pt.frequency = 8000;
break;
case 10:
msg_Dbg (demux, "detected stereo PCM");
pt.init = l16s_init;
pt.frequency = 44100;
break;
case 11:
msg_Dbg (demux, "detected mono PCM");
pt.init = l16m_init;
pt.frequency = 44100;
break;
case 12:
msg_Dbg (demux, "detected QCELP");
pt.init = qcelp_init;
pt.frequency = 8000;
break;
case 14:
msg_Dbg (demux, "detected MPEG Audio");
pt.init = mpa_init;
pt.decode = mpa_decode;
pt.frequency = 90000;
break;
case 32:
msg_Dbg (demux, "detected MPEG Video");
pt.init = mpv_init;
pt.decode = mpv_decode;
pt.frequency = 90000;
break;
case 33:
msg_Dbg (demux, "detected MPEG2 TS");
pt.init = ts_init;
pt.destroy = stream_destroy;
pt.header = stream_header;
pt.decode = stream_decode;
pt.frequency = 90000;
break;
default:
if (ptype >= 96)
{
char *dynamic = var_InheritString(demux, "rtp-dynamic-pt");
if (dynamic == NULL)
;
else if (!strcmp(dynamic, "theora"))
{
msg_Dbg (demux, "assuming Theora Encoded Video");
pt.init = theora_init;
pt.destroy = xiph_destroy;
pt.decode = xiph_decode;
pt.frequency = 90000;
free (dynamic);
break;
}
else
msg_Err (demux, "unknown dynamic payload format `%s' "
"specified", dynamic);
free (dynamic);
}
msg_Err (demux, "unspecified payload format (type %"PRIu8")", ptype);
msg_Info (demux, "A valid SDP is needed to parse this RTP stream.");
vlc_dialog_display_error (demux, N_("SDP required"),
N_("A description in SDP format is required to receive the RTP "
"stream. Note that rtp:// URIs cannot work with dynamic "
"RTP payload format (%"PRIu8")."), ptype);
return;
}
rtp_add_type (demux, session, &pt);
}
