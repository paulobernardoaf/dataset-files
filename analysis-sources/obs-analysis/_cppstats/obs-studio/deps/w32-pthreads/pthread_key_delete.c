#include "pthread.h"
#include "implement.h"
int
pthread_key_delete (pthread_key_t key)
{
ptw32_mcs_local_node_t keyLock;
int result = 0;
if (key != NULL)
{
if (key->threads != NULL && key->destructor != NULL)
{
ThreadKeyAssoc *assoc;
ptw32_mcs_lock_acquire (&(key->keyLock), &keyLock);
while ((assoc = (ThreadKeyAssoc *) key->threads) != NULL)
{
ptw32_mcs_local_node_t threadLock;
ptw32_thread_t * thread = assoc->thread;
if (assoc == NULL)
{
break;
}
ptw32_mcs_lock_acquire (&(thread->threadLock), &threadLock);
ptw32_tkAssocDestroy (assoc);
ptw32_mcs_lock_release (&threadLock);
ptw32_mcs_lock_release (&keyLock);
}
}
TlsFree (key->key);
if (key->destructor != NULL)
{
ptw32_mcs_lock_acquire (&(key->keyLock), &keyLock);
ptw32_mcs_lock_release (&keyLock);
}
#if defined( _DEBUG )
memset ((char *) key, 0, sizeof (*key));
#endif
free (key);
}
return (result);
}
