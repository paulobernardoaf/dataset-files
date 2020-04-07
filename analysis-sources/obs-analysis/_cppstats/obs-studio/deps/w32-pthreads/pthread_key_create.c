#include "pthread.h"
#include "implement.h"
#if !defined(TLS_OUT_OF_INDEXES)
#define TLS_OUT_OF_INDEXES 0xffffffff
#endif
int
pthread_key_create (pthread_key_t * key, void (PTW32_CDECL *destructor) (void *))
{
int result = 0;
pthread_key_t newkey;
if ((newkey = (pthread_key_t) calloc (1, sizeof (*newkey))) == NULL)
{
result = ENOMEM;
}
else if ((newkey->key = TlsAlloc ()) == TLS_OUT_OF_INDEXES)
{
result = EAGAIN;
free (newkey);
newkey = NULL;
}
else if (destructor != NULL)
{
newkey->keyLock = 0;
newkey->destructor = destructor;
}
*key = newkey;
return (result);
}
