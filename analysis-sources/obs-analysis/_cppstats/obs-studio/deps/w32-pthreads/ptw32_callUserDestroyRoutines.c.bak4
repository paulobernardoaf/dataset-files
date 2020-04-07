




































#include "pthread.h"
#include "implement.h"

#if defined(__CLEANUP_CXX)
#if defined(_MSC_VER)
#include <eh.h>
#elif defined(__WATCOMC__)
#include <eh.h>
#include <exceptio.h>
#else
#if defined(__GNUC__) && __GNUC__ < 3
#include <new.h>
#else
#include <new>
using
std::terminate;
#endif
#endif
#endif

void
ptw32_callUserDestroyRoutines (pthread_t thread)
















{
ThreadKeyAssoc * assoc;

if (thread.p != NULL)
{
ptw32_mcs_local_node_t threadLock;
ptw32_mcs_local_node_t keyLock;
int assocsRemaining;
int iterations = 0;
ptw32_thread_t * sp = (ptw32_thread_t *) thread.p;







do
{
assocsRemaining = 0;
iterations++;

ptw32_mcs_lock_acquire(&(sp->threadLock), &threadLock);








sp->nextAssoc = sp->keys;
ptw32_mcs_lock_release(&threadLock);

for (;;)
{
void * value;
pthread_key_t k;
void (*destructor) (void *);






ptw32_mcs_lock_acquire(&(sp->threadLock), &threadLock);

if ((assoc = (ThreadKeyAssoc *)sp->nextAssoc) == NULL)
{

ptw32_mcs_lock_release(&threadLock);
break;
}
else
{










if (ptw32_mcs_lock_try_acquire(&(assoc->key->keyLock), &keyLock) == EBUSY)
{
ptw32_mcs_lock_release(&threadLock);
Sleep(0);





continue;
}
}



sp->nextAssoc = assoc->nextKey;








k = assoc->key;
destructor = k->destructor;
value = TlsGetValue(k->key);
TlsSetValue (k->key, NULL);


if (value != NULL && iterations <= PTHREAD_DESTRUCTOR_ITERATIONS)
{







ptw32_mcs_lock_release(&threadLock);
ptw32_mcs_lock_release(&keyLock);

assocsRemaining++;

#if defined(__cplusplus)

try
{



destructor (value);
}
catch (...)
{









terminate ();
}

#else 




destructor (value);

#endif 

}
else
{




ptw32_tkAssocDestroy (assoc);
ptw32_mcs_lock_release(&threadLock);
ptw32_mcs_lock_release(&keyLock);
}
}
}
while (assocsRemaining);
}
} 
