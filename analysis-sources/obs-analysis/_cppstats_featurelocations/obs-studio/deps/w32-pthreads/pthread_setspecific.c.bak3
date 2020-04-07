



































#include "pthread.h"
#include "implement.h"


int
pthread_setspecific (pthread_key_t key, const void *value)
























{
pthread_t self;
int result = 0;

if (key != ptw32_selfThreadKey)
{





self = pthread_self ();
if (self.p == NULL)
{
return ENOENT;
}
}
else
{




ptw32_thread_t * sp = (ptw32_thread_t *) pthread_getspecific (ptw32_selfThreadKey);

if (sp == NULL)
{
if (value == NULL)
{
return ENOENT;
}
self = *((pthread_t *) value);
}
else
{
self = sp->ptHandle;
}
}

result = 0;

if (key != NULL)
{
if (self.p != NULL && key->destructor != NULL && value != NULL)
{
ptw32_mcs_local_node_t keyLock;
ptw32_mcs_local_node_t threadLock;
ptw32_thread_t * sp = (ptw32_thread_t *) self.p;









ThreadKeyAssoc *assoc;

ptw32_mcs_lock_acquire(&(key->keyLock), &keyLock);
ptw32_mcs_lock_acquire(&(sp->threadLock), &threadLock);

assoc = (ThreadKeyAssoc *) sp->keys;



while (assoc != NULL)
{
if (assoc->key == key)
{



break;
}
assoc = assoc->nextKey;
}




if (assoc == NULL)
{
result = ptw32_tkAssocCreate (sp, key);
}

ptw32_mcs_lock_release(&threadLock);
ptw32_mcs_lock_release(&keyLock);
}

if (result == 0)
{
if (!TlsSetValue (key->key, (LPVOID) value))
{
result = EAGAIN;
}
}
}

return (result);
} 
