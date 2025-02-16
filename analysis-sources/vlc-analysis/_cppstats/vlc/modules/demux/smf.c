#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_demux.h>
#include <vlc_charset.h>
#include <limits.h>
#include <assert.h>
#define TEMPO_MIN 20
#define TEMPO_MAX 250 
static int32_t ReadVarInt (stream_t *s)
{
uint32_t val = 0;
uint8_t byte;
for (unsigned i = 0; i < 4; i++)
{
if (vlc_stream_Read (s, &byte, 1) < 1)
return -1;
val = (val << 7) | (byte & 0x7f);
if ((byte & 0x80) == 0)
return val;
}
return -1;
}
typedef struct smf_track_t
{
uint64_t next; 
uint64_t start; 
uint32_t length; 
uint32_t offset; 
uint8_t running_event; 
} mtrk_t;
static int ReadDeltaTime (stream_t *s, mtrk_t *track)
{
int32_t delta_time;
assert (vlc_stream_Tell (s) == track->start + track->offset);
if (track->offset >= track->length)
{
track->next = UINT64_MAX;
return 0;
}
delta_time = ReadVarInt (s);
if (delta_time < 0)
return -1;
track->next += delta_time;
track->offset = vlc_stream_Tell (s) - track->start;
return 0;
}
typedef struct
{
es_out_id_t *es;
date_t pts; 
uint64_t pulse; 
vlc_tick_t tick; 
vlc_tick_t duration; 
unsigned ppqn; 
unsigned trackc; 
mtrk_t trackv[]; 
} demux_sys_t;
static
int HandleMeta (demux_t *p_demux, mtrk_t *tr)
{
stream_t *s = p_demux->s;
demux_sys_t *p_sys = p_demux->p_sys;
uint8_t *payload;
uint8_t type;
int32_t length;
int ret = 0;
if (vlc_stream_Read (s, &type, 1) != 1)
return -1;
length = ReadVarInt (s);
if (length < 0)
return -1;
payload = malloc (length + 1);
if ((payload == NULL)
|| (vlc_stream_Read (s, payload, length) != length))
{
free (payload);
return -1;
}
payload[length] = '\0';
switch (type)
{
case 0x00: 
break;
case 0x01: 
EnsureUTF8 ((char *)payload);
msg_Info (p_demux, "Text : %s", (char *)payload);
break;
case 0x02: 
EnsureUTF8 ((char *)payload);
msg_Info (p_demux, "Copyright : %s", (char *)payload);
break;
case 0x03: 
EnsureUTF8 ((char *)payload);
msg_Info (p_demux, "Track name: %s", (char *)payload);
break;
case 0x04: 
EnsureUTF8 ((char *)payload);
msg_Info (p_demux, "Instrument: %s", (char *)payload);
break;
case 0x05: 
break;
case 0x06: 
EnsureUTF8 ((char *)payload);
msg_Info (p_demux, "Marker : %s", (char *)payload);
break;
case 0x07: 
EnsureUTF8 ((char *)payload);
msg_Info (p_demux, "Cue point : %s", (char *)payload);
break;
case 0x08: 
EnsureUTF8 ((char *)payload);
msg_Info (p_demux, "Patch name: %s", (char *)payload);
break;
case 0x09: 
EnsureUTF8 ((char *)payload);
msg_Dbg (p_demux, "MIDI port : %s", (char *)payload);
break;
case 0x2F: 
if (tr->start + tr->length != vlc_stream_Tell (s))
{
msg_Err (p_demux, "misplaced end of track");
ret = -1;
}
break;
case 0x51: 
if (length == 3)
{
uint32_t uspqn = (payload[0] << 16)
| (payload[1] << 8) | payload[2];
unsigned tempo = 60 * 1000000 / (uspqn ? uspqn : 1);
msg_Dbg (p_demux, "tempo: %uus/qn -> %u BPM",
(unsigned)uspqn, tempo);
if (tempo < TEMPO_MIN)
{
msg_Warn (p_demux, "tempo too slow -> %u BPM", TEMPO_MIN);
tempo = TEMPO_MIN;
}
else
if (tempo > TEMPO_MAX)
{
msg_Warn (p_demux, "tempo too fast -> %u BPM", TEMPO_MAX);
tempo = TEMPO_MAX;
}
date_Change (&p_sys->pts, p_sys->ppqn * tempo, 60);
}
else
ret = -1;
break;
case 0x54: 
if (length == 5)
msg_Warn (p_demux, "SMPTE offset not implemented");
else
ret = -1;
break;
case 0x58: 
if (length == 4)
;
else
ret = -1;
break;
case 0x59: 
if (length != 2)
msg_Warn(p_demux, "invalid key signature");
break;
case 0x7f: 
msg_Dbg (p_demux, "ignored proprietary SMF Meta Event (%d bytes)",
length);
break;
default:
msg_Warn (p_demux, "unknown SMF Meta Event type 0x%02X (%d bytes)",
type, length);
}
free (payload);
return ret;
}
static
int HandleMessage (demux_t *p_demux, mtrk_t *tr, es_out_t *out)
{
stream_t *s = p_demux->s;
demux_sys_t *sys = p_demux->p_sys;
block_t *block;
uint8_t first, event;
int datalen;
if (vlc_stream_Seek (s, tr->start + tr->offset)
|| (vlc_stream_Read (s, &first, 1) != 1))
return -1;
event = (first & 0x80) ? first : tr->running_event;
switch (event & 0xf0)
{
case 0xF0: 
switch (event)
{
case 0xF0: 
case 0xF7: 
{
int32_t len = ReadVarInt (s);
if (len == -1)
return -1;
block = vlc_stream_Block (s, len);
if (block == NULL)
return -1;
block = block_Realloc (block, 1, len);
if (block == NULL)
return -1;
block->p_buffer[0] = event;
goto send;
}
case 0xFF: 
if (HandleMeta (p_demux, tr))
return -1;
goto skip;
case 0xF1:
case 0xF3:
datalen = 1;
break;
case 0xF2:
datalen = 2;
break;
case 0xF4:
case 0xF5:
default:
datalen = 0;
break;
}
break;
case 0xC0:
case 0xD0:
datalen = 1;
break;
default:
datalen = 2;
break;
}
block = block_Alloc (1 + datalen);
if (block == NULL)
goto skip;
block->p_buffer[0] = event;
if (first & 0x80)
{
if (vlc_stream_Read(s, block->p_buffer + 1, datalen) < datalen)
goto error;
}
else
{
if (datalen == 0)
{ 
msg_Err (p_demux, "malformatted MIDI event");
goto error;
}
block->p_buffer[1] = first;
if (datalen > 1
&& vlc_stream_Read(s, block->p_buffer + 2, datalen - 1) < datalen - 1)
goto error;
}
send:
block->i_dts = block->i_pts = date_Get(&sys->pts);
if (out != NULL)
es_out_Send(out, sys->es, block);
else
block_Release (block);
skip:
if (event < 0xF8)
tr->running_event = event;
tr->offset = vlc_stream_Tell (s) - tr->start;
return 0;
error:
block_Release(block);
return -1;
}
static int SeekSet0 (demux_t *demux)
{
stream_t *stream = demux->s;
demux_sys_t *sys = demux->p_sys;
date_Init (&sys->pts, sys->ppqn * 2, 1);
date_Set (&sys->pts, VLC_TICK_0);
sys->pulse = 0;
sys->tick = VLC_TICK_0;
for (unsigned i = 0; i < sys->trackc; i++)
{
mtrk_t *tr = sys->trackv + i;
tr->offset = 0;
tr->next = 0;
tr->running_event = 0xF6;
if (vlc_stream_Seek (stream, tr->start)
|| ReadDeltaTime (stream, tr))
{
msg_Err (demux, "fatal parsing error");
return -1;
}
}
return 0;
}
static int ReadEvents (demux_t *demux, uint64_t *restrict pulse,
es_out_t *out)
{
uint64_t cur_pulse = *pulse, next_pulse = UINT64_MAX;
demux_sys_t *sys = demux->p_sys;
for (unsigned i = 0; i < sys->trackc; i++)
{
mtrk_t *track = sys->trackv + i;
while (track->next <= cur_pulse)
{
if (HandleMessage (demux, track, out)
|| ReadDeltaTime (demux->s, track))
{
msg_Err (demux, "fatal parsing error");
return -1;
}
}
if (next_pulse > track->next)
next_pulse = track->next;
}
if (next_pulse != UINT64_MAX)
date_Increment (&sys->pts, next_pulse - cur_pulse);
*pulse = next_pulse;
return 0;
}
#define TICK VLC_TICK_FROM_MS(10)
static int Demux (demux_t *demux)
{
demux_sys_t *sys = demux->p_sys;
if (sys->tick <= date_Get (&sys->pts))
{
block_t *tick = block_Alloc (1);
if (unlikely(tick == NULL))
return VLC_ENOMEM;
tick->p_buffer[0] = 0xF9;
tick->i_dts = tick->i_pts = sys->tick;
es_out_Send (demux->out, sys->es, tick);
es_out_SetPCR (demux->out, sys->tick);
sys->tick += TICK;
return VLC_DEMUXER_SUCCESS;
}
uint64_t pulse = sys->pulse;
if (ReadEvents (demux, &pulse, demux->out))
return VLC_DEMUXER_EGENERIC;
if (pulse == UINT64_MAX)
return VLC_DEMUXER_EOF; 
sys->pulse = pulse;
return VLC_DEMUXER_SUCCESS;
}
static int Seek (demux_t *demux, vlc_tick_t pts)
{
demux_sys_t *sys = demux->p_sys;
if (pts < date_Get (&sys->pts) && SeekSet0 (demux))
return VLC_EGENERIC;
uint64_t pulse = sys->pulse;
while (pts > date_Get (&sys->pts))
{
if (pulse == UINT64_MAX)
return VLC_SUCCESS; 
if (ReadEvents (demux, &pulse, NULL))
return VLC_EGENERIC;
}
sys->pulse = pulse;
sys->tick = ((date_Get (&sys->pts) - VLC_TICK_0) / TICK) * TICK + VLC_TICK_0;
return VLC_SUCCESS;
}
static int Control (demux_t *demux, int i_query, va_list args)
{
demux_sys_t *sys = demux->p_sys;
switch (i_query)
{
case DEMUX_CAN_SEEK:
*va_arg (args, bool *) = true;
break;
case DEMUX_GET_POSITION:
if (!sys->duration)
return VLC_EGENERIC;
*va_arg (args, double *) = (sys->tick - (double)VLC_TICK_0)
/ sys->duration;
break;
case DEMUX_SET_POSITION:
return Seek (demux, va_arg (args, double) * sys->duration);
case DEMUX_GET_LENGTH:
*va_arg (args, vlc_tick_t *) = sys->duration;
break;
case DEMUX_GET_TIME:
*va_arg (args, vlc_tick_t *) = sys->tick - VLC_TICK_0;
break;
case DEMUX_SET_TIME:
return Seek (demux, va_arg (args, vlc_tick_t));
case DEMUX_CAN_PAUSE:
case DEMUX_SET_PAUSE_STATE:
case DEMUX_CAN_CONTROL_PACE:
case DEMUX_GET_PTS_DELAY:
return demux_vaControlHelper( demux->s, 0, -1, 0, 1, i_query, args );
default:
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}
static int Open (vlc_object_t *obj)
{
demux_t *demux = (demux_t *)obj;
stream_t *stream = demux->s;
const uint8_t *peek;
bool multitrack;
if (vlc_stream_Peek (stream, &peek, 14) < 14)
return VLC_EGENERIC;
if (!memcmp (peek, "RIFF", 4) && !memcmp (peek + 8, "RMID", 4))
{
uint32_t riff_len = GetDWLE (peek + 4);
msg_Dbg (demux, "detected RIFF MIDI file (%"PRIu32" bytes)", riff_len);
if ((vlc_stream_Read (stream, NULL, 12) < 12))
return VLC_EGENERIC;
for (;;)
{
char chnk_hdr[8];
uint32_t chnk_len;
if ((riff_len < 8)
|| (vlc_stream_Read (stream, chnk_hdr, 8) < 8))
return VLC_EGENERIC;
riff_len -= 8;
chnk_len = GetDWLE (chnk_hdr + 4);
if (riff_len < chnk_len)
return VLC_EGENERIC;
riff_len -= chnk_len;
if (!memcmp (chnk_hdr, "data", 4))
break; 
if (vlc_stream_Read (stream, NULL, chnk_len) < (ssize_t)chnk_len)
return VLC_EGENERIC;
}
if (vlc_stream_Peek (stream, &peek, 14) < 14)
return VLC_EGENERIC;
}
if (memcmp (peek, "MThd\x00\x00\x00\x06", 8))
return VLC_EGENERIC;
peek += 8;
switch (GetWBE (peek))
{
case 0:
multitrack = false;
break;
case 1:
multitrack = true;
break;
default:
msg_Err (demux, "unsupported SMF file type %u", GetWBE (peek));
return VLC_EGENERIC;
}
peek += 2;
unsigned tracks = GetWBE (peek);
peek += 2;
if (!multitrack && (tracks != 1))
{
msg_Err (demux, "invalid SMF type 0 file");
return VLC_EGENERIC;
}
msg_Dbg (demux, "detected Standard MIDI File (type %u) with %u track(s)",
multitrack, tracks);
unsigned ppqn = GetWBE (peek);
if (ppqn & 0x8000)
{ 
msg_Err (demux, "SMPTE timestamps not implemented");
return VLC_EGENERIC;
}
else
{
if (ppqn == 0)
{
msg_Err(demux, "invalid SMF file PPQN: %u", ppqn);
return VLC_EGENERIC;
}
msg_Dbg (demux, " %u pulses per quarter note", ppqn);
}
demux_sys_t *sys = malloc (sizeof (*sys) + (sizeof (mtrk_t) * tracks));
if (unlikely(sys == NULL))
return VLC_ENOMEM;
if (vlc_stream_Read (stream, NULL, 14) < 14)
goto error;
demux->p_sys = sys;
sys->duration = 0;
sys->ppqn = ppqn;
sys->trackc = tracks;
for (unsigned i = 0; i < tracks; i++)
{
mtrk_t *tr = sys->trackv + i;
uint8_t head[8];
if (i > 0 && vlc_stream_Seek (stream, tr[-1].start + tr[-1].length))
{
msg_Err (demux, "cannot build SMF index (corrupted file?)");
goto error;
}
for (;;)
{
if (vlc_stream_Read (stream, head, 8) < 8)
{
msg_Err (demux, "incomplete SMF chunk, file is corrupted");
goto error;
}
if (memcmp (head, "MTrk", 4) == 0)
break;
uint_fast32_t chunk_len = GetDWBE(head + 4);
msg_Dbg(demux, "skipping unknown SMF chunk (%"PRIuFAST32" bytes)",
chunk_len);
if (vlc_stream_Seek(stream, vlc_stream_Tell(stream) + chunk_len))
goto error;
}
tr->start = vlc_stream_Tell (stream);
tr->length = GetDWBE (head + 4);
}
bool b;
if (vlc_stream_Control (stream, STREAM_CAN_FASTSEEK, &b) == 0 && b)
{
if (SeekSet0 (demux))
goto error;
for (uint64_t pulse = 0; pulse != UINT64_MAX;)
if (ReadEvents (demux, &pulse, NULL))
break;
sys->duration = date_Get (&sys->pts);
}
if (SeekSet0 (demux))
goto error;
es_format_t fmt;
es_format_Init (&fmt, AUDIO_ES, VLC_CODEC_MIDI);
fmt.audio.i_channels = 2;
fmt.audio.i_rate = 44100; 
fmt.i_id = 0;
sys->es = es_out_Add (demux->out, &fmt);
demux->pf_demux = Demux;
demux->pf_control = Control;
return VLC_SUCCESS;
error:
free (sys);
return VLC_EGENERIC;
}
static void Close (vlc_object_t * p_this)
{
demux_t *p_demux = (demux_t *)p_this;
demux_sys_t *p_sys = p_demux->p_sys;
free (p_sys);
}
vlc_module_begin ()
set_description (N_("SMF demuxer"))
set_category (CAT_INPUT)
set_subcategory (SUBCAT_INPUT_DEMUX)
set_capability ("demux", 20)
set_callbacks (Open, Close)
vlc_module_end ()
