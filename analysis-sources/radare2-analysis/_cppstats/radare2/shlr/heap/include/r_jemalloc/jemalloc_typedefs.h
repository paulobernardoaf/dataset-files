typedef void *(chunk_alloc_t)(void *, size_t, size_t, bool *, bool *, unsigned);
typedef bool (chunk_dalloc_t)(void *, size_t, bool, unsigned);
typedef bool (chunk_commit_t)(void *, size_t, size_t, size_t, unsigned);
typedef bool (chunk_decommit_t)(void *, size_t, size_t, size_t, unsigned);
typedef bool (chunk_purge_t)(void *, size_t, size_t, size_t, unsigned);
typedef bool (chunk_split_t)(void *, size_t, size_t, size_t, bool, unsigned);
typedef bool (chunk_merge_t)(void *, size_t, void *, size_t, bool, unsigned);
typedef struct {
chunk_alloc_t *alloc;
chunk_dalloc_t *dalloc;
chunk_commit_t *commit;
chunk_decommit_t *decommit;
chunk_purge_t *purge;
chunk_split_t *split;
chunk_merge_t *merge;
} chunk_hooks_t;
