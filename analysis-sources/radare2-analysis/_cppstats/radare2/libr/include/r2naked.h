#if defined(__cplusplus)
extern "C" {
#endif
void *r_core_new(void);
char *r_core_cmd_str(void *p, const char *cmd);
void r_core_free(void* core);
void free(void*);
#if defined(__cplusplus)
}
#endif
