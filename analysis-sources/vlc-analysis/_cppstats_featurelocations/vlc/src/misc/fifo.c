






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>

#include <vlc_common.h>
#include <vlc_block.h>
#include "libvlc.h"




struct block_fifo_t
{
vlc_mutex_t lock; 
vlc_cond_t wait; 

block_t *p_first;
block_t **pp_last;
size_t i_depth;
size_t i_size;
};

void vlc_fifo_Lock(vlc_fifo_t *fifo)
{
vlc_mutex_lock(&fifo->lock);
}

void vlc_fifo_Unlock(vlc_fifo_t *fifo)
{
vlc_mutex_unlock(&fifo->lock);
}

void vlc_fifo_Signal(vlc_fifo_t *fifo)
{
vlc_cond_signal(&fifo->wait);
}

void vlc_fifo_Wait(vlc_fifo_t *fifo)
{
vlc_fifo_WaitCond(fifo, &fifo->wait);
}

void vlc_fifo_WaitCond(vlc_fifo_t *fifo, vlc_cond_t *condvar)
{
vlc_cond_wait(condvar, &fifo->lock);
}

int vlc_fifo_TimedWaitCond(vlc_fifo_t *fifo, vlc_cond_t *condvar, vlc_tick_t deadline)
{
return vlc_cond_timedwait(condvar, &fifo->lock, deadline);
}

size_t vlc_fifo_GetCount(const vlc_fifo_t *fifo)
{
vlc_mutex_assert(&fifo->lock);
return fifo->i_depth;
}

size_t vlc_fifo_GetBytes(const vlc_fifo_t *fifo)
{
vlc_mutex_assert(&fifo->lock);
return fifo->i_size;
}

void vlc_fifo_QueueUnlocked(block_fifo_t *fifo, block_t *block)
{
vlc_mutex_assert(&fifo->lock);
assert(*(fifo->pp_last) == NULL);

*(fifo->pp_last) = block;

while (block != NULL)
{
fifo->pp_last = &block->p_next;
fifo->i_depth++;
fifo->i_size += block->i_buffer;

block = block->p_next;
}

vlc_fifo_Signal(fifo);
}

block_t *vlc_fifo_DequeueUnlocked(block_fifo_t *fifo)
{
vlc_mutex_assert(&fifo->lock);

block_t *block = fifo->p_first;

if (block == NULL)
return NULL; 

fifo->p_first = block->p_next;
if (block->p_next == NULL)
fifo->pp_last = &fifo->p_first;
block->p_next = NULL;

assert(fifo->i_depth > 0);
fifo->i_depth--;
assert(fifo->i_size >= block->i_buffer);
fifo->i_size -= block->i_buffer;

return block;
}

block_t *vlc_fifo_DequeueAllUnlocked(block_fifo_t *fifo)
{
vlc_mutex_assert(&fifo->lock);

block_t *block = fifo->p_first;

fifo->p_first = NULL;
fifo->pp_last = &fifo->p_first;
fifo->i_depth = 0;
fifo->i_size = 0;

return block;
}

block_fifo_t *block_FifoNew( void )
{
block_fifo_t *p_fifo = malloc( sizeof( block_fifo_t ) );
if( !p_fifo )
return NULL;

vlc_mutex_init( &p_fifo->lock );
vlc_cond_init( &p_fifo->wait );
p_fifo->p_first = NULL;
p_fifo->pp_last = &p_fifo->p_first;
p_fifo->i_depth = p_fifo->i_size = 0;

return p_fifo;
}

void block_FifoRelease( block_fifo_t *p_fifo )
{
block_ChainRelease( p_fifo->p_first );
free( p_fifo );
}

void block_FifoEmpty(block_fifo_t *fifo)
{
block_t *block;

vlc_fifo_Lock(fifo);
block = vlc_fifo_DequeueAllUnlocked(fifo);
vlc_fifo_Unlock(fifo);
block_ChainRelease(block);
}

void block_FifoPut(block_fifo_t *fifo, block_t *block)
{
vlc_fifo_Lock(fifo);
vlc_fifo_QueueUnlocked(fifo, block);
vlc_fifo_Unlock(fifo);
}

block_t *block_FifoGet(block_fifo_t *fifo)
{
block_t *block;

vlc_testcancel();

vlc_fifo_Lock(fifo);
while (vlc_fifo_IsEmpty(fifo))
{
vlc_fifo_CleanupPush(fifo);
vlc_fifo_Wait(fifo);
vlc_cleanup_pop();
}
block = vlc_fifo_DequeueUnlocked(fifo);
vlc_fifo_Unlock(fifo);

return block;
}

block_t *block_FifoShow( block_fifo_t *p_fifo )
{
block_t *b;

vlc_mutex_lock( &p_fifo->lock );
assert(p_fifo->p_first != NULL);
b = p_fifo->p_first;
vlc_mutex_unlock( &p_fifo->lock );

return b;
}


size_t block_FifoSize (block_fifo_t *fifo)
{
size_t size;

vlc_mutex_lock (&fifo->lock);
size = fifo->i_size;
vlc_mutex_unlock (&fifo->lock);
return size;
}


size_t block_FifoCount (block_fifo_t *fifo)
{
size_t depth;

vlc_mutex_lock (&fifo->lock);
depth = fifo->i_depth;
vlc_mutex_unlock (&fifo->lock);
return depth;
}
