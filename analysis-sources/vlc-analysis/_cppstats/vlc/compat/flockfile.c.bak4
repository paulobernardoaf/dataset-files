



















#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <stdio.h>

#if defined(_WIN32)
#if !defined(HAVE__LOCK_FILE)
#warning Broken SDK: VLC logs will be garbage.
#define _lock_file(s) ((void)(s))
#define _unlock_file(s) ((void)(s))
#undef _getc_nolock
#define _getc_nolock(s) getc(s)
#undef _putc_nolock
#define _putc_nolock(s,c) putc(s,c)
#endif

void flockfile (FILE *stream)
{
_lock_file (stream);
}

void funlockfile (FILE *stream)
{
_unlock_file (stream);
}

int getc_unlocked (FILE *stream)
{
return _getc_nolock (stream);
}

int putc_unlocked (int c, FILE *stream)
{
return _putc_nolock (c, stream);
}

#elif defined __native_client__
void flockfile (FILE *stream)
{
_flockfile(stream);
}

void funlockfile (FILE *stream)
{
_funlockfile(stream);
}
#else
#error flockfile not implemented on your platform!
#endif

int getchar_unlocked (void)
{
return getc_unlocked (stdin);
}

int putchar_unlocked (int c)
{
return putc_unlocked (c, stdout);
}
