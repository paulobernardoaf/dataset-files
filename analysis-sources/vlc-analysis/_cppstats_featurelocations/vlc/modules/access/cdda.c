
































#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <vlc_common.h>
#include <vlc_demux.h>
#include <vlc_plugin.h>
#include <vlc_input_item.h>
#include <vlc_access.h>
#include <vlc_meta.h>
#include <vlc_charset.h> 
#include <vlc_url.h>
#include <vlc_strings.h>
#include <vlc_memstream.h>
#if defined(HAVE_GCRYPT)
#include <gcrypt.h>
#include <vlc_gcrypt.h>
#endif

#include "vcd/cdrom.h" 
#include "../misc/webservices/musicbrainz.h"

#if defined(HAVE_LIBCDDB)
#include <cddb/cddb.h>
#include <errno.h>
#endif

static vcddev_t *DiscOpen(vlc_object_t *obj, const char *location,
const char *path, unsigned *restrict trackp)
{
char *devpath;

*trackp = var_InheritInteger(obj, "cdda-track");

if (path != NULL)
devpath = ToLocaleDup(path);
else if (location[0] != '\0')
{
#if (DIR_SEP_CHAR == '/')
char *dec = vlc_uri_decode_duplicate(location);
if (dec == NULL)
return NULL;


const char *sl = strrchr(dec, '/');
if (sl != NULL)
{
if (sscanf(sl, "/Track %2u", trackp) == 1)
dec[sl - dec] = '\0';
else
*trackp = 0;
}

if (unlikely(asprintf(&devpath, "/dev/%s", dec) == -1))
devpath = NULL;
free(dec);
#else
(void) location;
return NULL;
#endif
}
else
devpath = var_InheritString(obj, "cd-audio");

if (devpath == NULL)
return NULL;

#if defined (_WIN32) || defined (__OS2__)

if (devpath[0] != '\0' && !strcmp(&devpath[1], ":" DIR_SEP))
devpath[2] = '\0';
#endif


vcddev_t *dev = ioctl_Open(obj, devpath);
if (dev == NULL)
msg_Warn(obj, "cannot open disc %s", devpath);
free(devpath);

return dev;
}


#define CDDA_BLOCKS_ONCE 20

typedef struct
{
vcddev_t *vcddev; 
es_out_id_t *es;
date_t pts;

unsigned start; 
unsigned length; 
unsigned position; 
} demux_sys_t;

static int Demux(demux_t *demux)
{
demux_sys_t *sys = demux->p_sys;
unsigned count = CDDA_BLOCKS_ONCE;

if (sys->position >= sys->length)
return VLC_DEMUXER_EOF;

if (sys->position + count >= sys->length)
count = sys->length - sys->position;

block_t *block = block_Alloc(count * CDDA_DATA_SIZE);
if (unlikely(block == NULL))
return VLC_DEMUXER_EOF;

if (ioctl_ReadSectors(VLC_OBJECT(demux), sys->vcddev,
sys->start + sys->position,
block->p_buffer, count, CDDA_TYPE) < 0)
{
msg_Err(demux, "cannot read sector %u", sys->position);
block_Release(block);


sys->position++;
return VLC_DEMUXER_SUCCESS;
}

sys->position += count;

block->i_nb_samples = block->i_buffer / 4;
block->i_dts = block->i_pts = date_Get(&sys->pts);
date_Increment(&sys->pts, block->i_nb_samples);

es_out_Send(demux->out, sys->es, block);
es_out_SetPCR(demux->out, date_Get(&sys->pts));
return VLC_DEMUXER_SUCCESS;
}

static int DemuxControl(demux_t *demux, int query, va_list args)
{
demux_sys_t *sys = demux->p_sys;


static_assert (CDDA_DATA_SIZE * CLOCK_FREQ * 3 ==
4 * 44100 * INT64_C(40000), "Wrong time/sector ratio");

switch (query)
{
case DEMUX_CAN_SEEK:
case DEMUX_CAN_PAUSE:
case DEMUX_CAN_CONTROL_PACE:
*va_arg(args, bool*) = true;
break;
case DEMUX_GET_PTS_DELAY:
*va_arg(args, vlc_tick_t *) =
VLC_TICK_FROM_MS( var_InheritInteger(demux, "disc-caching") );
break;

case DEMUX_SET_PAUSE_STATE:
break;

case DEMUX_GET_POSITION:
*va_arg(args, double *) = (double)(sys->position)
/ (double)(sys->length);
break;

case DEMUX_SET_POSITION:
sys->position = lround(va_arg(args, double) * sys->length);
break;

case DEMUX_GET_LENGTH:
*va_arg(args, vlc_tick_t *) = (INT64_C(40000) * sys->length) / 3;
break;
case DEMUX_GET_TIME:
*va_arg(args, vlc_tick_t *) = (INT64_C(40000) * sys->position) / 3;
break;
case DEMUX_SET_TIME:
sys->position = (va_arg(args, vlc_tick_t) * 3) / INT64_C(40000);
break;

default:
return VLC_EGENERIC;
}
return VLC_SUCCESS;
}





















static int TOC_GetAudioRange(vcddev_toc_t *p_toc,
int *pi_first, int *pi_last)
{
if( p_toc->i_tracks < 1 )
return 0;

int i_first = p_toc->i_first_track;
int i_last = p_toc->i_last_track;
for(int i=i_first; i<p_toc->i_tracks; i++)
{
if((p_toc->p_sectors[i - 1].i_control & CD_ROM_DATA_FLAG) == 0)
break;
i_first++;
}
for(int i=i_last; i > 0; i--)
{
if((p_toc->p_sectors[i - 1].i_control & CD_ROM_DATA_FLAG) == 0)
break;
i_last--;
}



do
{
vcddev_sector_t *p_last = &p_toc->p_sectors[i_last - p_toc->i_first_track];
vcddev_sector_t *p_lout = &p_toc->p_sectors[p_toc->i_tracks];
if(p_lout->i_lba > p_last->i_lba || i_last <= i_first)
break;

i_last = i_last - 1;
p_toc->i_last_track = i_last;
p_last->i_lba -= CD_ROM_XA_INTERVAL;
p_toc->i_tracks--; 
} while( i_last > i_first );

*pi_first = i_first;
*pi_last = i_last;
return (i_last >= i_first) ? i_last - i_first + 1 : 0;
}

static int DemuxOpen(vlc_object_t *obj, vcddev_t *dev, unsigned track)
{
demux_t *demux = (demux_t *)obj;

if (demux->out == NULL)
goto error;

demux_sys_t *sys = vlc_obj_malloc(obj, sizeof (*sys));
if (unlikely(sys == NULL))
goto error;

demux->p_sys = sys;
sys->vcddev = dev;
sys->start = var_InheritInteger(obj, "cdda-first-sector");
sys->length = var_InheritInteger(obj, "cdda-last-sector") - sys->start;


if (sys->start == (unsigned)-1 || sys->length == (unsigned)-1)
{
vcddev_toc_t *p_toc = ioctl_GetTOC(obj, dev, true);
if(p_toc == NULL)
goto error;

int i_cdda_tracks, i_cdda_first, i_cdda_last;
i_cdda_tracks = TOC_GetAudioRange(p_toc, &i_cdda_first, &i_cdda_last);

if (track == 0 || track > (unsigned) i_cdda_tracks)
{
msg_Err(obj, "invalid track number: %u/%i", track, i_cdda_tracks);
vcddev_toc_Free(p_toc);
goto error;
}

track--;
int i_first_sector = p_toc->p_sectors[track].i_lba;
int i_last_sector = p_toc->p_sectors[track + 1].i_lba;
if(i_cdda_first + track == (unsigned) i_cdda_last && p_toc->i_last_track > i_cdda_last)
i_last_sector -= CD_ROM_XA_INTERVAL;

sys->start = i_first_sector;
sys->length = i_last_sector - i_first_sector;
vcddev_toc_Free(p_toc);
}

es_format_t fmt;

es_format_Init(&fmt, AUDIO_ES, VLC_CODEC_S16L);
fmt.audio.i_rate = 44100;
fmt.audio.i_channels = 2;
sys->es = es_out_Add(demux->out, &fmt);

date_Init(&sys->pts, 44100, 1);
date_Set(&sys->pts, VLC_TICK_0);

sys->position = 0;
demux->pf_demux = Demux;
demux->pf_control = DemuxControl;
return VLC_SUCCESS;

error:
ioctl_Close(obj, dev);
return VLC_EGENERIC;
}




typedef struct
{
vcddev_t *vcddev; 
vcddev_toc_t *p_toc; 
int i_cdda_tracks; 
int i_cdda_first; 
int i_cdda_last; 
int cdtextc;
vlc_meta_t **cdtextv;
#if defined(HAVE_LIBCDDB)
cddb_disc_t *cddb;
#endif
musicbrainz_recording_t *mbrecord;
} access_sys_t;

static inline int LBAPregap( int i_sector )
{


return i_sector + 150;
}

#if defined(HAVE_GCRYPT)
static char * BuildMusicbrainzDiscID( const vcddev_toc_t *p_toc,
int i_total, int i_first, int i_last )
{
gcry_md_hd_t hd;
gcry_error_t err = gcry_md_open( &hd, GCRY_MD_SHA1, 0 );
if(err)
return NULL;

if(gcry_md_enable( hd, GCRY_MD_SHA1 ))
{
gcry_md_close( hd );
return NULL;
}

char buffer[16];

sprintf( buffer, "%02X", i_first );
gcry_md_write( hd, buffer, 2 );
sprintf( buffer, "%02X", i_last );
gcry_md_write( hd, buffer, 2 );




int i_last_track_end;
if (p_toc->i_tracks > i_total)
i_last_track_end = LBAPregap(p_toc->p_sectors[i_total].i_lba) - CD_ROM_XA_INTERVAL;
else
i_last_track_end = LBAPregap(p_toc->p_sectors[p_toc->i_tracks].i_lba);

sprintf( buffer, "%08X", i_last_track_end );
gcry_md_write( hd, buffer, 8 );

for( int i = 0; i<i_total; i++ ) 
{
sprintf( buffer, "%08X", LBAPregap(p_toc->p_sectors[i].i_lba) );
gcry_md_write( hd, buffer, 8 );
}

for( int i = i_total; i<100; i++ )
{
if( i == p_toc->i_tracks )
continue;
gcry_md_write( hd, "00000000", 8 );
}

gcry_md_final( hd );

size_t i_len = gcry_md_get_algo_dlen( GCRY_MD_SHA1 );
unsigned char *output = gcry_md_read( hd, GCRY_MD_SHA1 );
char *out = vlc_b64_encode_binary( output, i_len );
if( out )
{
i_len = strlen( out );
for( size_t i=0; i<i_len; i++ )
{
if( !isalpha(out[i]) )
{
if( out[i] == '+' )
out[i] = '.';
else if( out[i] == '/' )
out[i] = '_';
else if( out[i] == '=' )
out[i] = '-';
}
}
}

gcry_md_close( hd );

return out;
}
#else
#define BuildMusicbrainzDiscID(a, b, c, d) (NULL)
#endif

static musicbrainz_recording_t * GetMusicbrainzInfo( vlc_object_t *obj,
const vcddev_toc_t *p_toc,
int i_total, int i_first, int i_last )
{
musicbrainz_recording_t *recording = NULL;

char *psz_mbserver = var_InheritString( obj, "musicbrainz-server" );
if( !psz_mbserver || !*psz_mbserver )
return NULL;

musicbrainz_config_t cfg = { .obj = obj,
.psz_mb_server = psz_mbserver,
.psz_coverart_server = NULL };


char *psz_disc_id = BuildMusicbrainzDiscID( p_toc,
i_total, i_first, i_last );
if( psz_disc_id )
{
recording = musicbrainz_lookup_recording_by_discid( &cfg, psz_disc_id );
}
else 
{
struct vlc_memstream ms;

vlc_memstream_open(&ms);
vlc_memstream_printf(&ms, "toc=%u+%u", i_first, i_last );



int i_last_track_end;
if (p_toc->i_tracks > i_total)
i_last_track_end = LBAPregap(p_toc->p_sectors[i_total].i_lba) - CD_ROM_XA_INTERVAL;
else
i_last_track_end = LBAPregap(p_toc->p_sectors[p_toc->i_tracks].i_lba);
vlc_memstream_printf(&ms, "+%u", i_last_track_end );
for( int i = 0; i<i_total; i++ ) 
vlc_memstream_printf(&ms, "+%u", LBAPregap(p_toc->p_sectors[i].i_lba) );
if( vlc_memstream_close(&ms) == 0 )
{
recording = musicbrainz_lookup_recording_by_toc( &cfg, ms.ptr );
free( ms.ptr );
}
}

free( psz_mbserver );
return recording;
}

#if defined(HAVE_LIBCDDB)
static cddb_disc_t *GetCDDBInfo( vlc_object_t *obj, const vcddev_toc_t *p_toc )
{
msg_Dbg( obj, "retrieving metadata with CDDB" );


cddb_conn_t *p_cddb = cddb_new();
if( !p_cddb )
{
msg_Warn( obj, "unable to use CDDB" );
return NULL;
}



cddb_http_enable( p_cddb );

char *psz_tmp = var_InheritString( obj, "cddb-server" );
if( psz_tmp )
{
cddb_set_server_name( p_cddb, psz_tmp );
free( psz_tmp );
}

cddb_set_server_port( p_cddb, var_InheritInteger( obj, "cddb-port" ) );

cddb_set_email_address( p_cddb, "vlc@videolan.org" );

cddb_set_http_path_query( p_cddb, "/~cddb/cddb.cgi" );
cddb_set_http_path_submit( p_cddb, "/~cddb/submit.cgi" );


char *psz_cachedir;
char *psz_temp = config_GetUserDir( VLC_CACHE_DIR );

if( asprintf( &psz_cachedir, "%s" DIR_SEP "cddb", psz_temp ) > 0 ) {
cddb_cache_enable( p_cddb );
cddb_cache_set_dir( p_cddb, psz_cachedir );
free( psz_cachedir );
}
free( psz_temp );

cddb_set_timeout( p_cddb, 10 );


cddb_disc_t *p_disc = cddb_disc_new();
if( !p_disc )
{
msg_Err( obj, "unable to create CDDB disc structure." );
goto error;
}

int64_t i_length = 2000000; 
for( int i = 0; i < p_toc->i_tracks; i++ )
{
cddb_track_t *t = cddb_track_new();
cddb_track_set_frame_offset( t, p_toc->p_sectors[i].i_lba + 150 ); 

cddb_disc_add_track( p_disc, t );
const int64_t i_size = ( p_toc->p_sectors[i+1].i_lba - p_toc->p_sectors[i].i_lba ) *
(int64_t)CDDA_DATA_SIZE;
i_length += INT64_C(1000000) * i_size / 44100 / 4 ;

msg_Dbg( obj, "Track %i offset: %i", i, p_toc->p_sectors[i].i_lba + 150 );
}

msg_Dbg( obj, "Total length: %i", (int)(i_length/1000000) );
cddb_disc_set_length( p_disc, (int)(i_length/1000000) );

if( !cddb_disc_calc_discid( p_disc ) )
{
msg_Err( obj, "CDDB disc ID calculation failed" );
goto error;
}

const int i_matches = cddb_query( p_cddb, p_disc );
if( i_matches < 0 )
{
msg_Warn( obj, "CDDB error: %s", cddb_error_str(errno) );
goto error;
}
else if( i_matches == 0 )
{
msg_Dbg( obj, "Couldn't find any matches in CDDB." );
goto error;
}
else if( i_matches > 1 )
msg_Warn( obj, "found %d matches in CDDB. Using first one.", i_matches );

cddb_read( p_cddb, p_disc );

msg_Dbg( obj, "disc ID: 0x%08x", cddb_disc_get_discid(p_disc) );

cddb_destroy( p_cddb);
return p_disc;

error:
if( p_disc )
cddb_disc_destroy( p_disc );
cddb_destroy( p_cddb );
msg_Dbg( obj, "CDDB failure" );
return NULL;
}
#endif 

static void AccessGetMeta(stream_t *access, vlc_meta_t *meta)
{
access_sys_t *sys = access->p_sys;

vlc_meta_SetTitle(meta, "Audio CD");


if (sys->cdtextc > 0 && sys->cdtextv[0] != NULL)
vlc_meta_Merge(meta, sys->cdtextv[0]);


#define NONEMPTY( psz ) ( (psz) && *(psz) )

#define ON_EMPTY( psz, code ) do { if( !NONEMPTY( psz) ) { (psz) = code; } } while(0)


#if defined(HAVE_LIBCDDB)
if (sys->cddb != NULL)
{
const char *str = cddb_disc_get_title(sys->cddb);
if (NONEMPTY(str))
vlc_meta_SetTitle(meta, str);

str = cddb_disc_get_genre(sys->cddb);
if (NONEMPTY(str))
vlc_meta_SetGenre(meta, str);

const unsigned year = cddb_disc_get_year(sys->cddb);
if (year != 0)
{
char yearbuf[5];

snprintf(yearbuf, sizeof (yearbuf), "%u", year);
vlc_meta_SetDate(meta, yearbuf);
}


str = cddb_disc_get_artist(sys->cddb);
if (NONEMPTY(str))
{
for (int i = 0; i < sys->p_toc->i_tracks; i++)
{
cddb_track_t *t = cddb_disc_get_track(sys->cddb, i);
if (t == NULL)
continue;

const char *track_artist = cddb_track_get_artist(t);
if (NONEMPTY(track_artist))
{
if (str == NULL)
str = track_artist;
else
if (strcmp(str, track_artist))
{
str = NULL;
break;
}
}
}
}
}
#endif
}

static int ReadDir(stream_t *access, input_item_node_t *node)
{
access_sys_t *sys = access->p_sys;
const vcddev_toc_t *p_toc = sys->p_toc;


const int i_start_track_offset = sys->i_cdda_first - sys->p_toc->i_first_track;
for (int i = 0; i < sys->i_cdda_tracks; i++)
{
if(i < i_start_track_offset)
continue;

msg_Dbg(access, "track[%d] start=%d", i, p_toc->p_sectors[i].i_lba);


char *name;

if (unlikely(asprintf(&name, _("Audio CD - Track %02i"),
i - i_start_track_offset + 1 ) == -1))
name = NULL;


int i_first_sector = p_toc->p_sectors[i].i_lba;
int i_last_sector = p_toc->p_sectors[i + 1].i_lba;
if(sys->i_cdda_first + i == sys->i_cdda_last &&
p_toc->i_last_track > sys->i_cdda_last)
i_last_sector -= CD_ROM_XA_INTERVAL;

const vlc_tick_t duration =
(vlc_tick_t)(i_last_sector - i_first_sector)
* CDDA_DATA_SIZE * CLOCK_FREQ / 44100 / 2 / 2;

input_item_t *item = input_item_NewDisc(access->psz_url,
(name != NULL) ? name :
access->psz_url, duration);
free(name);

if (unlikely(item == NULL))
continue;

char *opt;
if (likely(asprintf(&opt, "cdda-track=%i", i + 1) != -1))
{
input_item_AddOption(item, opt, VLC_INPUT_OPTION_TRUSTED);
free(opt);
}

if (likely(asprintf(&opt, "cdda-first-sector=%i",
p_toc->p_sectors[i].i_lba) != -1))
{
input_item_AddOption(item, opt, VLC_INPUT_OPTION_TRUSTED);
free(opt);
}

if (likely(asprintf(&opt, "cdda-last-sector=%i", i_last_sector) != -1))
{
input_item_AddOption(item, opt, VLC_INPUT_OPTION_TRUSTED);
free(opt);
}

const char *title = NULL;
const char *artist = NULL;
const char *album = NULL;
const char *genre = NULL;
const char *description = NULL;
int year = 0;

#if defined(HAVE_LIBCDDB)
if (sys->cddb != NULL)
{
cddb_track_t *t = cddb_disc_get_track(sys->cddb, i);
if (t != NULL)
{
title = cddb_track_get_title(t);
artist = cddb_track_get_artist(t);
}

ON_EMPTY(artist, cddb_disc_get_artist(sys->cddb));
album = cddb_disc_get_title(sys->cddb);
genre = cddb_disc_get_genre(sys->cddb);
year = cddb_disc_get_year(sys->cddb);
}
#endif


if(sys->cdtextc > 0)
{
const vlc_meta_t *m;
if (i + 1 < sys->cdtextc && (m = sys->cdtextv[i + 1]) != NULL)
{
ON_EMPTY(title, vlc_meta_Get(m, vlc_meta_Title));
ON_EMPTY(artist, vlc_meta_Get(m, vlc_meta_Artist));
ON_EMPTY(genre, vlc_meta_Get(m, vlc_meta_Genre));
ON_EMPTY(description, vlc_meta_Get(m, vlc_meta_Description));
}


if ((m = sys->cdtextv[0]) != NULL)
{
ON_EMPTY(artist, vlc_meta_Get(m, vlc_meta_Artist));
ON_EMPTY(album, vlc_meta_Get(m, vlc_meta_Album));
ON_EMPTY(genre, vlc_meta_Get(m, vlc_meta_Genre));
ON_EMPTY(description, vlc_meta_Get(m, vlc_meta_Description));
}
}

if(sys->mbrecord && sys->mbrecord->i_release)
{
musicbrainz_release_t *r = sys->mbrecord->p_releases;
for(size_t j=0; j<r->i_tracks; j++)
{
if(r->p_tracks[j].i_index == (unsigned)i + 1)
{
if (r->p_tracks[j].psz_title)
{
ON_EMPTY(title, r->p_tracks[j].psz_title);
ON_EMPTY(artist, r->p_tracks[j].psz_artist);
}
break;
}
}
ON_EMPTY(album, r->psz_title);
if(NONEMPTY(r->psz_artist))
{
artist = r->psz_artist;
input_item_SetAlbumArtist(item, r->psz_artist);
}
if(year == 0 && r->psz_date)
{
unsigned i_year;
if(sscanf(r->psz_date, "%4d", &i_year) == 1)
year = i_year;
}
if(NONEMPTY(r->psz_coverart_url))
input_item_SetArtworkURL(item, r->psz_coverart_url);
}

if (NONEMPTY(title))
{
input_item_SetName(item, title);
input_item_SetTitle(item, title);
}

if (NONEMPTY(artist))
input_item_SetArtist(item, artist);

if (NONEMPTY(genre))
input_item_SetGenre(item, genre);

if (NONEMPTY(description))
input_item_SetDescription(item, description);

if (NONEMPTY(album))
input_item_SetAlbum(item, album);

if (year != 0)
{
char yearbuf[5];

snprintf(yearbuf, sizeof (yearbuf), "%u", year);
input_item_SetDate(item, yearbuf);
}

char num[4];
if(snprintf(num, sizeof (num), "%u", i + 1) < 4)
input_item_SetTrackNum(item, num);
snprintf(num, sizeof (num), "%u", p_toc->i_tracks);
input_item_SetTrackTotal(item, num);

input_item_node_AppendItem(node, item);
input_item_Release(item);
}
#undef ON_EMPTY
#undef NONEMPTY
return VLC_SUCCESS;
}

static int AccessControl(stream_t *access, int query, va_list args)
{
if (query == STREAM_GET_META)
{
AccessGetMeta(access, va_arg(args, vlc_meta_t *));
return VLC_SUCCESS;
}
return access_vaDirectoryControlHelper(access, query, args);
}

static int AccessOpen(vlc_object_t *obj, vcddev_t *dev)
{
stream_t *access = (stream_t *)obj;

access_sys_t *sys = vlc_obj_malloc(obj, sizeof (*sys));
if (unlikely(sys == NULL))
{
ioctl_Close(obj, dev);
return VLC_ENOMEM;
}

sys->vcddev = dev;
sys->p_toc = ioctl_GetTOC(obj, dev, true);
if (sys->p_toc == NULL)
{
msg_Err(obj, "cannot count tracks");
goto error;
}

sys->i_cdda_tracks = TOC_GetAudioRange(sys->p_toc, &sys->i_cdda_first, &sys->i_cdda_last);
if (sys->i_cdda_tracks == 0)
{
msg_Err(obj, "no audio tracks found");
vcddev_toc_Free(sys->p_toc);
goto error;
}

if (ioctl_GetCdText(obj, dev, &sys->cdtextv, &sys->cdtextc))
{
msg_Dbg(obj, "CD-TEXT information missing");
sys->cdtextv = NULL;
sys->cdtextc = 0;
}

sys->mbrecord = NULL;
#if defined(HAVE_LIBCDDB)
sys->cddb = NULL;
#endif

if(var_InheritBool(obj, "metadata-network-access"))
{
sys->mbrecord = GetMusicbrainzInfo(obj, sys->p_toc, sys->i_cdda_tracks,
sys->i_cdda_first, sys->i_cdda_last );
#if defined(HAVE_LIBCDDB)
if(!sys->mbrecord)
sys->cddb = GetCDDBInfo(obj, sys->p_toc);
#endif
}
else msg_Dbg(obj, "album art policy set to manual: not fetching");

access->p_sys = sys;
access->pf_read = NULL;
access->pf_block = NULL;
access->pf_readdir = ReadDir;
access->pf_seek = NULL;
access->pf_control = AccessControl;
return VLC_SUCCESS;

error:
ioctl_Close(obj, dev);
return VLC_EGENERIC;
}

static void AccessClose(access_sys_t *sys)
{
for (int i = 0; i < sys->cdtextc; i++)
{
vlc_meta_t *meta = sys->cdtextv[i];
if (meta != NULL)
vlc_meta_Delete(meta);
}
free(sys->cdtextv);

#if defined(HAVE_LIBCDDB)
if (sys->cddb != NULL)
cddb_disc_destroy(sys->cddb);
#endif
if(sys->mbrecord)
musicbrainz_recording_release(sys->mbrecord);
vcddev_toc_Free(sys->p_toc);
}

static int Open(vlc_object_t *obj)
{
stream_t *stream = (stream_t *)obj;
unsigned track;

vcddev_t *dev = DiscOpen(obj, stream->psz_location, stream->psz_filepath,
&track);
if (dev == NULL)
return VLC_EGENERIC;

if (track == 0)
return AccessOpen(obj, dev);
else
return DemuxOpen(obj, dev, track);
}

static void Close(vlc_object_t *obj)
{
stream_t *stream = (stream_t *)obj;
void *sys = stream->p_sys;

if (stream->pf_readdir != NULL)
AccessClose(sys);

static_assert(offsetof(demux_sys_t, vcddev) == 0, "Invalid cast");
static_assert(offsetof(access_sys_t, vcddev) == 0, "Invalid cast");
ioctl_Close(obj, *(vcddev_t **)sys);
}




#define CDAUDIO_DEV_TEXT N_("Audio CD device")
#if defined( _WIN32 ) || defined( __OS2__ )
#define CDAUDIO_DEV_LONGTEXT N_( "This is the default Audio CD drive (or file) to use. Don't forget the " "colon after the drive letter (e.g. D:)")


#define CD_DEVICE "D:"
#else
#define CDAUDIO_DEV_LONGTEXT N_( "This is the default Audio CD device to use." )

#if defined(__OpenBSD__)
#define CD_DEVICE "/dev/cd0c"
#elif defined(__linux__)
#define CD_DEVICE "/dev/sr0"
#else
#define CD_DEVICE "/dev/cdrom"
#endif
#endif

vlc_module_begin ()
set_shortname( N_("Audio CD") )
set_description( N_("Audio CD input") )
set_capability( "access", 0 )
set_category( CAT_INPUT )
set_subcategory( SUBCAT_INPUT_ACCESS )
set_callbacks(Open, Close)

add_loadfile("cd-audio", CD_DEVICE, CDAUDIO_DEV_TEXT, CDAUDIO_DEV_LONGTEXT)

add_usage_hint( N_("[cdda:][device][@[track]]") )
add_integer( "cdda-track", 0 , NULL, NULL, true )
change_volatile ()
add_integer( "cdda-first-sector", -1, NULL, NULL, true )
change_volatile ()
add_integer( "cdda-last-sector", -1, NULL, NULL, true )
change_volatile ()

add_string( "musicbrainz-server", MUSICBRAINZ_DEFAULT_SERVER,
N_( "Musicbrainz Server" ),
N_( "Address of the musicbrainz server to use." ), true )
#if defined(HAVE_LIBCDDB)
add_string( "cddb-server", "freedb.videolan.org", N_( "CDDB Server" ),
N_( "Address of the CDDB server to use." ), true )
add_integer( "cddb-port", 80, N_( "CDDB port" ),
N_( "CDDB Server port to use." ), true )
change_integer_range( 1, 65535 )
#endif

add_shortcut( "cdda", "cddasimple" )
vlc_module_end ()
