typedef struct
{
uint32_t begin;
uint32_t size;
uint32_t capacity;
vlc_tick_t *buffer;
} timestamp_fifo_t;
static inline timestamp_fifo_t *timestamp_FifoNew(uint32_t capacity)
{
timestamp_fifo_t *fifo = calloc(1, sizeof(*fifo));
if (!fifo)
return NULL;
fifo->buffer = vlc_alloc(capacity, sizeof(*fifo->buffer));
if (!fifo->buffer) {
free(fifo);
return NULL;
}
fifo->capacity = capacity;
return fifo;
}
static inline void timestamp_FifoRelease(timestamp_fifo_t *fifo)
{
free(fifo->buffer);
free(fifo);
}
static inline bool timestamp_FifoIsEmpty(timestamp_fifo_t *fifo)
{
return fifo->size == 0;
}
static inline bool timestamp_FifoIsFull(timestamp_fifo_t *fifo)
{
return fifo->size == fifo->capacity;
}
static inline void timestamp_FifoEmpty(timestamp_fifo_t *fifo)
{
fifo->size = 0;
}
static inline void timestamp_FifoPut(timestamp_fifo_t *fifo, vlc_tick_t ts)
{
uint32_t end = (fifo->begin + fifo->size) % fifo->capacity;
fifo->buffer[end] = ts;
if (!timestamp_FifoIsFull(fifo))
fifo->size += 1;
else
fifo->begin = (fifo->begin + 1) % fifo->capacity;
}
static inline vlc_tick_t timestamp_FifoGet(timestamp_fifo_t *fifo)
{
if (timestamp_FifoIsEmpty(fifo))
return VLC_TICK_INVALID;
vlc_tick_t result = fifo->buffer[fifo->begin];
fifo->begin = (fifo->begin + 1) % fifo->capacity;
fifo->size -= 1;
return result;
}
