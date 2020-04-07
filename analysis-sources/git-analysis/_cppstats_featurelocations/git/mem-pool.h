#if !defined(MEM_POOL_H)
#define MEM_POOL_H

struct mp_block {
struct mp_block *next_block;
char *next_free;
char *end;
uintmax_t space[FLEX_ARRAY]; 
};

struct mem_pool {
struct mp_block *mp_block;





size_t block_alloc;


size_t pool_alloc;
};




void mem_pool_init(struct mem_pool **mem_pool, size_t initial_size);




void mem_pool_discard(struct mem_pool *mem_pool, int invalidate_memory);




void *mem_pool_alloc(struct mem_pool *pool, size_t len);




void *mem_pool_calloc(struct mem_pool *pool, size_t count, size_t size);






void mem_pool_combine(struct mem_pool *dst, struct mem_pool *src);





int mem_pool_contains(struct mem_pool *mem_pool, void *mem);

#endif
