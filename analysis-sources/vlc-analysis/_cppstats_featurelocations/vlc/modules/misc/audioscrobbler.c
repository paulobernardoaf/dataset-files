































#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <time.h>

#define VLC_MODULE_LICENSE VLC_LICENSE_GPL_2_PLUS
#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_interface.h>
#include <vlc_input_item.h>
#include <vlc_dialog.h>
#include <vlc_meta.h>
#include <vlc_md5.h>
#include <vlc_memstream.h>
#include <vlc_stream.h>
#include <vlc_url.h>
#include <vlc_tls.h>
#include <vlc_player.h>
#include <vlc_playlist.h>





#define QUEUE_MAX 50


typedef struct audioscrobbler_song_t
{
char *psz_a; 
char *psz_t; 
char *psz_b; 
char *psz_n; 
int i_l; 
char *psz_m; 
time_t date; 
vlc_tick_t i_start; 
} audioscrobbler_song_t;

struct intf_sys_t
{
audioscrobbler_song_t p_queue[QUEUE_MAX]; 
int i_songs; 

vlc_playlist_t *playlist;
struct vlc_playlist_listener_id *playlist_listener;
struct vlc_player_listener_id *player_listener;

vlc_mutex_t lock; 
vlc_cond_t wait; 
vlc_thread_t thread; 


vlc_url_t p_submit_url; 


vlc_url_t p_nowp_url; 

char psz_auth_token[33]; 


audioscrobbler_song_t p_current_song; 

vlc_tick_t time_pause; 
vlc_tick_t time_total_pauses; 

bool b_submit_nowp; 

bool b_meta_read; 

};

static int Open (vlc_object_t *);
static void Close (vlc_object_t *);
static void *Run (void *);





#define USERNAME_TEXT N_("Username")
#define USERNAME_LONGTEXT N_("The username of your last.fm account")
#define PASSWORD_TEXT N_("Password")
#define PASSWORD_LONGTEXT N_("The password of your last.fm account")
#define URL_TEXT N_("Scrobbler URL")
#define URL_LONGTEXT N_("The URL set for an alternative scrobbler engine")


#define VLC_AUDIOSCROBBLER_EFATAL -69


#define CLIENT_NAME PACKAGE
#define CLIENT_VERSION VERSION

vlc_module_begin ()
set_category(CAT_INTERFACE)
set_subcategory(SUBCAT_INTERFACE_CONTROL)
set_shortname(N_("Audioscrobbler"))
set_description(N_("Submission of played songs to last.fm"))
add_string("lastfm-username", "",
USERNAME_TEXT, USERNAME_LONGTEXT, false)
add_password("lastfm-password", "", PASSWORD_TEXT, PASSWORD_LONGTEXT)
add_string("scrobbler-url", "post.audioscrobbler.com",
URL_TEXT, URL_LONGTEXT, false)
set_capability("interface", 0)
set_callbacks(Open, Close)
vlc_module_end ()




static void DeleteSong(audioscrobbler_song_t* p_song)
{
FREENULL(p_song->psz_a);
FREENULL(p_song->psz_b);
FREENULL(p_song->psz_t);
FREENULL(p_song->psz_m);
FREENULL(p_song->psz_n);
}




static void ReadMetaData(intf_thread_t *p_this)
{
intf_sys_t *p_sys = p_this->p_sys;

vlc_player_t *player = vlc_playlist_GetPlayer(p_sys->playlist);
input_item_t *item = vlc_player_GetCurrentMedia(player);
if (item == NULL)
return;

#define ALLOC_ITEM_META(a, b) do { char *psz_meta = input_item_Get##b(item); if (psz_meta && *psz_meta) a = vlc_uri_encode(psz_meta); free(psz_meta); } while (0)






vlc_mutex_lock(&p_sys->lock);

p_sys->b_meta_read = true;

ALLOC_ITEM_META(p_sys->p_current_song.psz_a, Artist);
if (!p_sys->p_current_song.psz_a)
{
msg_Dbg(p_this, "No artist..");
DeleteSong(&p_sys->p_current_song);
goto end;
}

ALLOC_ITEM_META(p_sys->p_current_song.psz_t, Title);
if (!p_sys->p_current_song.psz_t)
{
msg_Dbg(p_this, "No track name..");
DeleteSong(&p_sys->p_current_song);
goto end;
}


p_sys->b_submit_nowp = true;

ALLOC_ITEM_META(p_sys->p_current_song.psz_b, Album);
ALLOC_ITEM_META(p_sys->p_current_song.psz_m, TrackID);
ALLOC_ITEM_META(p_sys->p_current_song.psz_n, TrackNum);

p_sys->p_current_song.i_l = SEC_FROM_VLC_TICK(input_item_GetDuration(item));

#undef ALLOC_ITEM_META

msg_Dbg(p_this, "Meta data registered");

vlc_cond_signal(&p_sys->wait);

end:
vlc_mutex_unlock(&p_sys->lock);
}




static void AddToQueue (intf_thread_t *p_this)
{
int64_t played_time;
intf_sys_t *p_sys = p_this->p_sys;

vlc_mutex_lock(&p_sys->lock);


if (!p_sys->p_current_song.psz_t || !p_sys->p_current_song.psz_a)
goto end;


played_time = SEC_FROM_VLC_TICK(vlc_tick_now() - p_sys->p_current_song.i_start -
p_sys->time_total_pauses);



if (p_sys->p_current_song.i_l == 0)
p_sys->p_current_song.i_l = played_time;


if (p_sys->p_current_song.i_l < 30)
{
msg_Dbg(p_this, "Song too short (< 30s), not submitting");
goto end;
}


if ((played_time < 240) &&
(played_time < (p_sys->p_current_song.i_l / 2)))
{
msg_Dbg(p_this, "Song not listened long enough, not submitting");
goto end;
}


if (!p_sys->p_current_song.psz_a || !*p_sys->p_current_song.psz_a ||
!p_sys->p_current_song.psz_t || !*p_sys->p_current_song.psz_t)
{
msg_Dbg(p_this, "Missing artist or title, not submitting");
goto end;
}

if (p_sys->i_songs >= QUEUE_MAX)
{
msg_Warn(p_this, "Submission queue is full, not submitting");
goto end;
}

msg_Dbg(p_this, "Song will be submitted.");

#define QUEUE_COPY(a) p_sys->p_queue[p_sys->i_songs].a = p_sys->p_current_song.a


#define QUEUE_COPY_NULL(a) QUEUE_COPY(a); p_sys->p_current_song.a = NULL



QUEUE_COPY(i_l);
QUEUE_COPY_NULL(psz_n);
QUEUE_COPY_NULL(psz_a);
QUEUE_COPY_NULL(psz_t);
QUEUE_COPY_NULL(psz_b);
QUEUE_COPY_NULL(psz_m);
QUEUE_COPY(date);
#undef QUEUE_COPY_NULL
#undef QUEUE_COPY

p_sys->i_songs++;


vlc_cond_signal(&p_sys->wait);

end:
DeleteSong(&p_sys->p_current_song);
vlc_mutex_unlock(&p_sys->lock);
}

static void player_on_state_changed(vlc_player_t *player,
enum vlc_player_state state, void *data)
{
intf_thread_t *intf = data;
intf_sys_t *sys = intf->p_sys;

if (vlc_player_GetVideoTrackCount(player))
{
msg_Dbg(intf, "Not an audio-only input, not submitting");
return;
}

if (!sys->b_meta_read && state >= VLC_PLAYER_STATE_PLAYING)
{
ReadMetaData(intf);
return;
}

switch (state)
{
case VLC_PLAYER_STATE_STOPPED:
AddToQueue(intf);
break;
case VLC_PLAYER_STATE_PAUSED:
sys->time_pause = vlc_tick_now();
break;
case VLC_PLAYER_STATE_PLAYING:
if (sys->time_pause > 0)
{
sys->time_total_pauses += vlc_tick_now() - sys->time_pause;
sys->time_pause = 0;
}
break;
default:
break;
}
}




static void playlist_on_current_index_changed(vlc_playlist_t *playlist,
ssize_t index, void *userdata)
{
VLC_UNUSED(index);

intf_thread_t *intf = userdata;
intf_sys_t *sys = intf->p_sys;

sys->b_meta_read = false;

vlc_player_t *player = vlc_playlist_GetPlayer(playlist);
input_item_t *item = vlc_player_GetCurrentMedia(player);
if (!item)
return;

if (vlc_player_GetVideoTrackCount(player))
{
msg_Dbg(intf, "Not an audio-only input, not submitting");
return;
}

sys->time_total_pauses = 0;
time(&sys->p_current_song.date); 
sys->p_current_song.i_start = vlc_tick_now(); 

if (input_item_IsPreparsed(item))
ReadMetaData(intf);


}




static int Open(vlc_object_t *p_this)
{
intf_thread_t *p_intf = (intf_thread_t*) p_this;
intf_sys_t *p_sys = calloc(1, sizeof(intf_sys_t));
int retval = VLC_EGENERIC;

if (!p_sys)
return VLC_ENOMEM;

p_intf->p_sys = p_sys;

static struct vlc_playlist_callbacks const playlist_cbs =
{
.on_current_index_changed = playlist_on_current_index_changed,
};
static struct vlc_player_cbs const player_cbs =
{
.on_state_changed = player_on_state_changed,
};

vlc_playlist_t *playlist = p_sys->playlist = vlc_intf_GetMainPlaylist(p_intf);
vlc_player_t *player = vlc_playlist_GetPlayer(playlist);

vlc_playlist_Lock(playlist);
p_sys->playlist_listener =
vlc_playlist_AddListener(playlist, &playlist_cbs, p_intf, false);
if (!p_sys->playlist_listener)
{
vlc_playlist_Unlock(playlist);
goto fail;
}

p_sys->player_listener =
vlc_player_AddListener(player, &player_cbs, p_intf);
vlc_playlist_Unlock(playlist);
if (!p_sys->player_listener)
goto fail;

vlc_mutex_init(&p_sys->lock);
vlc_cond_init(&p_sys->wait);

if (vlc_clone(&p_sys->thread, Run, p_intf, VLC_THREAD_PRIORITY_LOW))
{
retval = VLC_ENOMEM;
goto fail;
}

retval = VLC_SUCCESS;
goto ret;
fail:
if (p_sys->playlist_listener)
{
vlc_playlist_Lock(playlist);
if (p_sys->player_listener)
vlc_player_RemoveListener(player, p_sys->player_listener);
vlc_playlist_RemoveListener(playlist, p_sys->playlist_listener);
vlc_playlist_Unlock(playlist);
}
free(p_sys);
ret:
return retval;
}




static void Close(vlc_object_t *p_this)
{
intf_thread_t *p_intf = (intf_thread_t*) p_this;
intf_sys_t *p_sys = p_intf->p_sys;
vlc_playlist_t *playlist = p_sys->playlist;

vlc_cancel(p_sys->thread);
vlc_join(p_sys->thread, NULL);

int i;
for (i = 0; i < p_sys->i_songs; i++)
DeleteSong(&p_sys->p_queue[i]);
vlc_UrlClean(&p_sys->p_submit_url);
vlc_UrlClean(&p_sys->p_nowp_url);

vlc_playlist_Lock(playlist);
vlc_player_RemoveListener(
vlc_playlist_GetPlayer(playlist), p_sys->player_listener);
vlc_playlist_RemoveListener(playlist, p_sys->playlist_listener);
vlc_playlist_Unlock(playlist);

free(p_sys);
}




static int Handshake(intf_thread_t *p_this)
{
char *psz_username, *psz_password;
char *psz_scrobbler_url;
time_t timestamp;
char psz_timestamp[21];

struct md5_s p_struct_md5;

stream_t *p_stream;
char *psz_handshake_url;
uint8_t p_buffer[1024];
char *p_buffer_pos;

int i_ret;
char *psz_url;

intf_thread_t *p_intf = (intf_thread_t*) p_this;
intf_sys_t *p_sys = p_this->p_sys;

psz_username = var_InheritString(p_this, "lastfm-username");
psz_password = var_InheritString(p_this, "lastfm-password");


if (EMPTY_STR(psz_username) || EMPTY_STR(psz_password))
{
free(psz_username);
free(psz_password);
return VLC_EBADVAR;
}

time(&timestamp);


InitMD5(&p_struct_md5);
AddMD5(&p_struct_md5, (uint8_t*) psz_password, strlen(psz_password));
EndMD5(&p_struct_md5);

free(psz_password);

char *psz_password_md5 = psz_md5_hash(&p_struct_md5);
if (!psz_password_md5)
{
free(psz_username);
return VLC_ENOMEM;
}

snprintf(psz_timestamp, sizeof(psz_timestamp), "%"PRIu64,
(uint64_t)timestamp);





InitMD5(&p_struct_md5);
AddMD5(&p_struct_md5, (uint8_t*) psz_password_md5, 32);
AddMD5(&p_struct_md5, (uint8_t*) psz_timestamp, strlen(psz_timestamp));
EndMD5(&p_struct_md5);
free(psz_password_md5);

char *psz_auth_token = psz_md5_hash(&p_struct_md5);
if (!psz_auth_token)
{
free(psz_username);
return VLC_ENOMEM;
}

psz_scrobbler_url = var_InheritString(p_this, "scrobbler-url");
if (!psz_scrobbler_url)
{
free(psz_auth_token);
free(psz_username);
return VLC_ENOMEM;
}

i_ret = asprintf(&psz_handshake_url,
"http://%s/?hs=true&p=1.2&c="CLIENT_NAME"&v="CLIENT_VERSION"&u=%s&t=%s&a=%s"
, psz_scrobbler_url, psz_username, psz_timestamp, psz_auth_token);

free(psz_auth_token);
free(psz_scrobbler_url);
free(psz_username);
if (i_ret == -1)
return VLC_ENOMEM;


p_stream = vlc_stream_NewURL(p_intf, psz_handshake_url);
free(psz_handshake_url);

if (!p_stream)
return VLC_EGENERIC;


i_ret = vlc_stream_Read(p_stream, p_buffer, sizeof(p_buffer) - 1);
if (i_ret <= 0)
{
vlc_stream_Delete(p_stream);
return VLC_EGENERIC;
}
p_buffer[i_ret] = '\0';
vlc_stream_Delete(p_stream);

p_buffer_pos = strstr((char*) p_buffer, "FAILED ");
if (p_buffer_pos)
{

msg_Err(p_this, "last.fm handshake failed: %s", p_buffer_pos + 7);
return VLC_EGENERIC;
}

if (strstr((char*) p_buffer, "BADAUTH"))
{

vlc_dialog_display_error(p_this,
_("last.fm: Authentication failed"),
"%s", _("last.fm username or password is incorrect. "
"Please verify your settings and relaunch VLC."));
return VLC_AUDIOSCROBBLER_EFATAL;
}

if (strstr((char*) p_buffer, "BANNED"))
{

msg_Err(p_intf, "This version of VLC has been banned by last.fm. "
"You should upgrade VLC, or disable the last.fm plugin.");
return VLC_AUDIOSCROBBLER_EFATAL;
}

if (strstr((char*) p_buffer, "BADTIME"))
{

msg_Err(p_intf, "last.fm handshake failed because your clock is too "
"much shifted. Please correct it, and relaunch VLC.");
return VLC_AUDIOSCROBBLER_EFATAL;
}

p_buffer_pos = strstr((char*) p_buffer, "OK");
if (!p_buffer_pos)
goto proto;

p_buffer_pos = strstr(p_buffer_pos, "\n");
if (!p_buffer_pos || strlen(p_buffer_pos) < 33)
goto proto;
p_buffer_pos++; 


memcpy(p_sys->psz_auth_token, p_buffer_pos, 32);
p_sys->psz_auth_token[32] = '\0';

p_buffer_pos = strstr(p_buffer_pos, "http://");
if (!p_buffer_pos || strlen(p_buffer_pos) == 7)
goto proto;


psz_url = strndup(p_buffer_pos, strcspn(p_buffer_pos, "\n"));
if (!psz_url)
goto oom;

vlc_UrlParse(&p_sys->p_nowp_url, psz_url);
free(psz_url);
if (p_sys->p_nowp_url.psz_host == NULL ||
p_sys->p_nowp_url.i_port == 0)
{
vlc_UrlClean(&p_sys->p_nowp_url);
goto proto;
}
p_buffer_pos += strcspn(p_buffer_pos, "\n");

p_buffer_pos = strstr(p_buffer_pos, "http://");
if (!p_buffer_pos || strlen(p_buffer_pos) == 7)
goto proto;


psz_url = strndup(p_buffer_pos, strcspn(p_buffer_pos, "\n"));
if (!psz_url)
goto oom;


vlc_UrlParse(&p_sys->p_submit_url, psz_url);
free(psz_url);
if (p_sys->p_submit_url.psz_host == NULL ||
p_sys->p_submit_url.i_port == 0)
{
vlc_UrlClean(&p_sys->p_nowp_url);
vlc_UrlClean(&p_sys->p_submit_url);
goto proto;
}

return VLC_SUCCESS;

oom:
return VLC_ENOMEM;

proto:
msg_Err(p_intf, "Handshake: can't recognize server protocol");
return VLC_EGENERIC;
}

static void HandleInterval(vlc_tick_t *next, unsigned int *i_interval)
{
if (*i_interval == 0)
{

*i_interval = 1;
}
else
{

*i_interval <<= 1;
if (*i_interval > 120)
*i_interval = 120;
}
*next = vlc_tick_now() + (*i_interval * VLC_TICK_FROM_SEC(60));
}




static void *Run(void *data)
{
intf_thread_t *p_intf = data;
uint8_t p_buffer[1024];
int canc = vlc_savecancel();
bool b_handshaked = false;
bool b_nowp_submission_ongoing = false;


vlc_tick_t next_exchange = VLC_TICK_INVALID; 
unsigned int i_interval = 0; 

intf_sys_t *p_sys = p_intf->p_sys;


for (;;)
{
vlc_restorecancel(canc);
if (next_exchange != VLC_TICK_INVALID)
vlc_tick_wait(next_exchange);

vlc_mutex_lock(&p_sys->lock);
mutex_cleanup_push(&p_sys->lock);

while (p_sys->i_songs == 0 && p_sys->b_submit_nowp == false)
vlc_cond_wait(&p_sys->wait, &p_sys->lock);

vlc_cleanup_pop();
vlc_mutex_unlock(&p_sys->lock);
canc = vlc_savecancel();


if (!b_handshaked)
{
msg_Dbg(p_intf, "Handshaking with last.fm ...");

switch(Handshake(p_intf))
{
case VLC_ENOMEM:
goto out;

case VLC_EBADVAR:

vlc_dialog_display_error(p_intf,
_("Last.fm username not set"),
"%s", _("Please set a username or disable the "
"audioscrobbler plugin, and restart VLC.\n"
"Visit http://www.last.fm/join/ to get an account."));
goto out;

case VLC_SUCCESS:
msg_Dbg(p_intf, "Handshake successful :)");
b_handshaked = true;
i_interval = 0;
next_exchange = VLC_TICK_INVALID;
break;

case VLC_AUDIOSCROBBLER_EFATAL:
msg_Warn(p_intf, "Exiting...");
goto out;

case VLC_EGENERIC:
default:

HandleInterval(&next_exchange, &i_interval);
break;
}

if (!b_handshaked)
continue;
}

msg_Dbg(p_intf, "Going to submit some data...");
vlc_url_t *url;
struct vlc_memstream req, payload;

vlc_memstream_open(&payload);
vlc_memstream_printf(&payload, "s=%s", p_sys->psz_auth_token);


vlc_mutex_lock(&p_sys->lock);

if (p_sys->b_submit_nowp)
{
audioscrobbler_song_t *p_song = &p_sys->p_current_song;

b_nowp_submission_ongoing = true;
url = &p_sys->p_nowp_url;

vlc_memstream_printf(&payload, "&a=%s", p_song->psz_a);
vlc_memstream_printf(&payload, "&t=%s", p_song->psz_t);
vlc_memstream_puts(&payload, "&b=");
if (p_song->psz_b != NULL)
vlc_memstream_puts(&payload, p_song->psz_b);
vlc_memstream_printf(&payload, "&l=%d", p_song->i_l);
vlc_memstream_puts(&payload, "&n=");
if (p_song->psz_n != NULL)
vlc_memstream_puts(&payload, p_song->psz_n);
vlc_memstream_puts(&payload, "&m=");
if (p_song->psz_m != NULL)
vlc_memstream_puts(&payload, p_song->psz_m);
}
else
{
url = &p_sys->p_submit_url;

for (int i_song = 0 ; i_song < p_sys->i_songs ; i_song++)
{
audioscrobbler_song_t *p_song = &p_sys->p_queue[i_song];

vlc_memstream_printf(&payload, "&a%%5B%d%%5D=%s",
i_song, p_song->psz_a);
vlc_memstream_printf(&payload, "&t%%5B%d%%5D=%s",
i_song, p_song->psz_t);
vlc_memstream_printf(&payload, "&i%%5B%d%%5D=%"PRIu64,
i_song, (uint64_t)p_song->date);
vlc_memstream_printf(&payload, "&o%%5B%d%%5D=P", i_song);
vlc_memstream_printf(&payload, "&r%%5B%d%%5D=", i_song);
vlc_memstream_printf(&payload, "&l%%5B%d%%5D=%d",
i_song, p_song->i_l);
vlc_memstream_printf(&payload, "&b=%%5B%d%%5D=", i_song);
if (p_song->psz_b != NULL)
vlc_memstream_puts(&payload, p_song->psz_b);
vlc_memstream_printf(&payload, "&n=%%5B%d%%5D=", i_song);
if (p_song->psz_n != NULL)
vlc_memstream_puts(&payload, p_song->psz_n);
vlc_memstream_printf(&payload, "&m=%%5B%d%%5D=", i_song);
if (p_song->psz_m != NULL)
vlc_memstream_puts(&payload, p_song->psz_m);
}
}

vlc_mutex_unlock(&p_sys->lock);

if (vlc_memstream_close(&payload))
goto out;

vlc_memstream_open(&req);
vlc_memstream_printf(&req, "POST %s HTTP/1.1\r\n", url->psz_path);
vlc_memstream_printf(&req, "Host: %s\r\n", url->psz_host);
vlc_memstream_puts(&req, "User-Agent:"
" "PACKAGE_NAME"/"PACKAGE_VERSION"\r\n");
vlc_memstream_puts(&req, "Connection: close\r\n");
vlc_memstream_puts(&req, "Accept-Encoding: identity\r\n");
vlc_memstream_puts(&req, "Content-Type:"
" application/x-www-form-urlencoded\r\n");
vlc_memstream_printf(&req, "Content-Length: %zu\r\n", payload.length);
vlc_memstream_puts(&req, "\r\n");

vlc_memstream_write(&req, payload.ptr, payload.length);
vlc_memstream_puts(&req, "\r\n\r\n");
free(payload.ptr);

if (vlc_memstream_close(&req)) 
goto out;

vlc_tls_t *sock = vlc_tls_SocketOpenTCP(VLC_OBJECT(p_intf),
url->psz_host, url->i_port);
if (sock == NULL)
{

HandleInterval(&next_exchange, &i_interval);
b_handshaked = false;
free(req.ptr);
continue;
}


int i_net_ret = vlc_tls_Write(sock, req.ptr, req.length);
free(req.ptr);
if (i_net_ret == -1)
{

HandleInterval(&next_exchange, &i_interval);
b_handshaked = false;
vlc_tls_Close(sock);
continue;
}




i_net_ret = vlc_tls_Read(sock, p_buffer, sizeof(p_buffer) - 1, false);
vlc_tls_Close(sock);
if (i_net_ret <= 0)
{

continue;
}
p_buffer[i_net_ret] = '\0';

char *failed = strstr((char *) p_buffer, "FAILED");
if (failed)
{
msg_Warn(p_intf, "%s", failed);
HandleInterval(&next_exchange, &i_interval);
continue;
}

if (strstr((char *) p_buffer, "BADSESSION"))
{
msg_Err(p_intf, "Authentication failed (BADSESSION), are you connected to last.fm with another program ?");
b_handshaked = false;
HandleInterval(&next_exchange, &i_interval);
continue;
}

if (strstr((char *) p_buffer, "OK"))
{
if (b_nowp_submission_ongoing)
{
b_nowp_submission_ongoing = false;
p_sys->b_submit_nowp = false;
}
else
{
for (int i = 0; i < p_sys->i_songs; i++)
DeleteSong(&p_sys->p_queue[i]);
p_sys->i_songs = 0;
}

i_interval = 0;
next_exchange = VLC_TICK_INVALID;
msg_Dbg(p_intf, "Submission successful!");
}
else
{
msg_Err(p_intf, "Authentication failed, handshaking again (%s)",
p_buffer);
b_handshaked = false;
HandleInterval(&next_exchange, &i_interval);
}
}
out:
vlc_restorecancel(canc);
return NULL;
}
