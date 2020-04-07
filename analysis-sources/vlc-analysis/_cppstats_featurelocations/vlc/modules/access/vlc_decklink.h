





















#if !defined(VLC_DECKLINK_H)
#define VLC_DECKLINK_H 1






#include <DeckLinkAPI.h>



#if defined(_WIN32)
#error FIXME: Win32 is known to not work for decklink.
#elif defined(__APPLE__)
#include <vlc_common.h>
#include <vlc_charset.h>
typedef CFStringRef decklink_str_t;
#define DECKLINK_STRDUP(s) FromCFString(s, kCFStringEncodingUTF8)
#define DECKLINK_FREE(s) CFRelease(s)
#else
typedef const char* decklink_str_t;
#define DECKLINK_STRDUP strdup
#define DECKLINK_FREE(s) free((void *) s)
#endif

#endif 

