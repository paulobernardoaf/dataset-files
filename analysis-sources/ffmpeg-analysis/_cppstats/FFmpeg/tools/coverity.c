#define NULL (void *)0
void *av_malloc(size_t size) {
int has_memory;
__coverity_negative_sink__(size);
if (has_memory) {
void *ptr = __coverity_alloc__(size);
__coverity_mark_as_uninitialized_buffer__(ptr);
__coverity_mark_as_afm_allocated__(ptr, "av_free");
return ptr;
} else {
return 0;
}
}
void *av_mallocz(size_t size) {
int has_memory;
__coverity_negative_sink__(size);
if (has_memory) {
void *ptr = __coverity_alloc__(size);
__coverity_writeall0__(ptr);
__coverity_mark_as_afm_allocated__(ptr, "av_free");
return ptr;
} else {
return 0;
}
}
void *av_realloc(void *ptr, size_t size) {
int has_memory;
__coverity_negative_sink__(size);
if (has_memory) {
__coverity_escape__(ptr);
ptr = __coverity_alloc__(size);
__coverity_writeall__(ptr);
__coverity_mark_as_afm_allocated__(ptr, "av_free");
return ptr;
} else {
return 0;
}
}
void *av_free(void *ptr) {
__coverity_free__(ptr);
__coverity_mark_as_afm_freed__(ptr, "av_free");
}
