#include "pthread.h"
#include "implement.h"
void *
pthread_getspecific (pthread_key_t key)
{
void * ptr;
if (key == NULL)
{
ptr = NULL;
}
else
{
int lasterror = GetLastError ();
#if defined(RETAIN_WSALASTERROR)
int lastWSAerror = WSAGetLastError ();
#endif
ptr = TlsGetValue (key->key);
SetLastError (lasterror);
#if defined(RETAIN_WSALASTERROR)
WSASetLastError (lastWSAerror);
#endif
}
return ptr;
}
