





















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <stdarg.h>
#include <assert.h>

#include <vlc_common.h>
#include <vlc_demux.h>
#include <vlc_network.h>
#include <vlc_plugin.h>

#include "../../demux/xiph.h"

#include "rtp.h"

typedef struct rtp_xiph_t
{
es_out_id_t *id;
block_t *block;
uint32_t ident;
bool vorbis;
} rtp_xiph_t;

static void *xiph_init (bool vorbis)
{
rtp_xiph_t *self = malloc (sizeof (*self));

if (self == NULL)
return NULL;

self->id = NULL;
self->block = NULL;
self->ident = 0xffffffff; 
self->vorbis = vorbis;
return self;
}

#if 0



static void *vorbis_init (demux_t *demux)
{
(void)demux;
return xiph_init (true);
}
#endif




void *theora_init (demux_t *demux)
{
(void)demux;
return xiph_init (false);
}

void xiph_destroy (demux_t *demux, void *data)
{
rtp_xiph_t *self = data;

if (!data)
return;
if (self->block)
{
self->block->i_flags |= BLOCK_FLAG_CORRUPTED;
codec_decode (demux, self->id, self->block);
}
codec_destroy (demux, self->id);
free (self);
}


static ssize_t xiph_header (void **pextra, const uint8_t *buf, size_t len)
{

if (len == 0)
return -1; 
unsigned hcount = 1 + *buf++;
len--;
if (hcount != 3)
return -1; 


uint16_t idlen = 0, cmtlen = 0, setuplen = 0;
do
{
if (len == 0)
return -1;
idlen = (idlen << 7) | (*buf & 0x7f);
len--;
}
while (*buf++ & 0x80);
do
{
if (len == 0)
return -1;
cmtlen = (cmtlen << 7) | (*buf & 0x7f);
len--;
}
while (*buf++ & 0x80);
if (len < idlen + cmtlen)
return -1;
setuplen = len - (idlen + cmtlen);


unsigned sizes[3] = {
idlen, cmtlen, setuplen
};
const void *payloads[3] = {
buf + 0,
buf + idlen,
buf + idlen + cmtlen
};
void *extra;
int extra_size;
if (xiph_PackHeaders (&extra_size, &extra, sizes, payloads, 3))
return -1;;
*pextra = extra;
return extra_size;
}


void xiph_decode (demux_t *demux, void *data, block_t *block)
{
rtp_xiph_t *self = data;

if (!data || block->i_buffer < 4)
goto drop;


uint32_t ident = GetDWBE (block->p_buffer);
block->i_buffer -= 4;
block->p_buffer += 4;

unsigned fragtype = (ident >> 6) & 3;
unsigned datatype = (ident >> 4) & 3;
unsigned pkts = (ident) & 15;
ident >>= 8;


if (self->block && (block->i_flags & BLOCK_FLAG_DISCONTINUITY))
{ 
msg_Warn (demux, self->vorbis ?
"discontinuity in fragmented Vorbis packet" :
"discontinuity in fragmented Theora packet");
block_Release (self->block);
self->block = NULL;
}

if (fragtype <= 1)
{
if (self->block) 
{
block_Release (self->block);
self->block = NULL;
}
}
else
{
if (!self->block)
goto drop; 
}

if (fragtype > 0)
{ 
if (pkts > 0 || block->i_buffer < 2)
goto drop;

size_t fraglen = GetWBE (block->p_buffer);
if (block->i_buffer < (fraglen + 2))
goto drop; 
block->i_buffer = fraglen;
if (fragtype == 1)
{
block->i_buffer += 2;
self->block = block;
}
else
{ 
size_t len = self->block->i_buffer;
self->block = block_Realloc (self->block, 0, len + fraglen);
if (!self->block)
{
block_Release (block);
return;
}
memcpy (self->block->p_buffer + len, block->p_buffer + 2,
fraglen);
block_Release (block);
}
if (fragtype < 3)
return; 


block = self->block;
self->block = NULL;
SetWBE (block->p_buffer, block->i_buffer - 2);
pkts = 1;
}


while (pkts > 0)
{
if (block->i_buffer < 2)
goto drop;

size_t len = GetWBE (block->p_buffer);
block->i_buffer -= 2;
block->p_buffer += 2;
if (block->i_buffer < len)
goto drop;

switch (datatype)
{
case 0: 
{
if (self->ident != ident)
{
msg_Warn (demux, self->vorbis ?
"ignoring raw Vorbis payload without configuration" :
"ignoring raw Theora payload without configuration");
break;
}
block_t *raw = block_Alloc (len);
memcpy (raw->p_buffer, block->p_buffer, len);
raw->i_pts = block->i_pts; 
codec_decode (demux, self->id, raw);
break;
}

case 1: 
{
if (self->ident == ident)
break; 

void *extv;
ssize_t extc = xiph_header (&extv, block->p_buffer, len);
if (extc < 0)
break;

es_format_t fmt;
es_format_Init (&fmt, self->vorbis ? AUDIO_ES : VIDEO_ES,
self->vorbis ? VLC_CODEC_VORBIS
: VLC_CODEC_THEORA);
fmt.p_extra = extv;
fmt.i_extra = extc;
codec_destroy (demux, self->id);
msg_Dbg (demux, self->vorbis ?
"Vorbis packed configuration received (%06"PRIx32")" :
"Theora packed configuration received (%06"PRIx32")",
ident);
self->ident = ident;
self->id = codec_init (demux, &fmt);
break;
}
}

block->i_buffer -= len;
block->p_buffer += len;
pkts--;
}

drop:
block_Release (block);
}
