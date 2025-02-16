#include "pthread.h"
#include "implement.h"
void
ptw32_tkAssocDestroy (ThreadKeyAssoc * assoc)
{
if (assoc != NULL)
{
ThreadKeyAssoc * prev, * next;
prev = assoc->prevKey;
next = assoc->nextKey;
if (prev != NULL)
{
prev->nextKey = next;
}
if (next != NULL)
{
next->prevKey = prev;
}
if (assoc->thread->keys == assoc)
{
assoc->thread->keys = next;
}
if (assoc->thread->nextAssoc == assoc)
{
assoc->thread->nextAssoc = next;
}
prev = assoc->prevThread;
next = assoc->nextThread;
if (prev != NULL)
{
prev->nextThread = next;
}
if (next != NULL)
{
next->prevThread = prev;
}
if (assoc->key->threads == assoc)
{
assoc->key->threads = next;
}
free (assoc);
}
} 
