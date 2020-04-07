#include "pthread.h"
#include "implement.h"
int
ptw32_tkAssocCreate (ptw32_thread_t * sp, pthread_key_t key)
{
ThreadKeyAssoc *assoc;
assoc = (ThreadKeyAssoc *) calloc (1, sizeof (*assoc));
if (assoc == NULL)
{
return ENOMEM;
}
assoc->thread = sp;
assoc->key = key;
assoc->prevThread = NULL;
assoc->nextThread = (ThreadKeyAssoc *) key->threads;
if (assoc->nextThread != NULL)
{
assoc->nextThread->prevThread = assoc;
}
key->threads = (void *) assoc;
assoc->prevKey = NULL;
assoc->nextKey = (ThreadKeyAssoc *) sp->keys;
if (assoc->nextKey != NULL)
{
assoc->nextKey->prevKey = assoc;
}
sp->keys = (void *) assoc;
return (0);
} 
