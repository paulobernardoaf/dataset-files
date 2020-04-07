





















#if !defined(VLC_FIFO_HELPER_H)
#define VLC_FIFO_HELPER_H 1

typedef struct FIFO_ITEM {
struct FIFO_ITEM *p_next;
} fifo_item_t;

typedef struct
{
fifo_item_t *p_first;
fifo_item_t **pp_last;
size_t i_depth;
} fifo_t;

static inline void fifo_Init(fifo_t *p_fifo)
{
p_fifo->p_first = NULL;
p_fifo->pp_last = &p_fifo->p_first;
p_fifo->i_depth = 0;
}

static inline void fifo_Release(fifo_t *fifo)
{
assert(fifo->i_depth == 0); 
}

static inline size_t fifo_GetCount(fifo_t *fifo)
{
return fifo->i_depth;
}

static inline fifo_item_t *fifo_Show(fifo_t *p_fifo)
{
fifo_item_t *b;

assert(p_fifo->p_first != NULL);
b = p_fifo->p_first;

return b;
}

static inline fifo_item_t *fifo_Get(fifo_t *fifo)
{
fifo_item_t *block = fifo->p_first;

if (block == NULL)
return NULL; 

fifo->p_first = block->p_next;
if (block->p_next == NULL)
fifo->pp_last = &fifo->p_first;
block->p_next = NULL;

assert(fifo->i_depth > 0);
fifo->i_depth--;

return block;
}

static inline void fifo_Put(fifo_t *fifo, fifo_item_t *p_item)
{
*(fifo->pp_last) = p_item;

while (p_item != NULL)
{
fifo->pp_last = &p_item->p_next;
fifo->i_depth++;

p_item = p_item->p_next;
}
}




#define TYPED_FIFO(type, prefix) static inline void prefix ##_fifo_Init(fifo_t *p_fifo) { fifo_Init(p_fifo); } static inline void prefix ##_fifo_Release(fifo_t *p_fifo) { fifo_Release(p_fifo); } static inline void prefix ##_fifo_Put(fifo_t *p_fifo, type *p_item) { fifo_Put(p_fifo, &p_item->fifo); } static inline type *prefix ##_fifo_Get(fifo_t *p_fifo) { return (type*)fifo_Get(p_fifo); } static inline type *prefix ##_fifo_Show(fifo_t *p_fifo) { return (type*)fifo_Show(p_fifo); } static inline size_t prefix ##_fifo_GetCount(fifo_t *fifo) { return fifo->i_depth; }


























#endif 
