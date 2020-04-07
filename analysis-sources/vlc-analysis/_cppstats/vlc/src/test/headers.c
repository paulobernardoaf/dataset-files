#define PACKAGE "vlc"
#if defined(__LIBVLC__)
#undef __LIBVLC__
#endif
#include <vlc/vlc.h>
#include <vlc/deprecated.h>
#include <vlc/libvlc.h>
#include <vlc/libvlc_events.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_media_discoverer.h>
#include <vlc/libvlc_media_list.h>
#include <vlc/libvlc_media_list_player.h>
#include <vlc/libvlc_media_player.h>
#include <stdio.h>
int main (void)
{
puts ("Public headers can be used for external compilation.");
return 0;
}
