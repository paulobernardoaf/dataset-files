struct blob;
struct index_state;
void *merge_blobs(struct index_state *, const char *,
struct blob *, struct blob *,
struct blob *, unsigned long *);
