


#if !defined(STREAMING_H)
#define STREAMING_H 1
#include "cache.h"


struct git_istream;

struct git_istream *open_istream(struct repository *, const struct object_id *,
enum object_type *, unsigned long *,
struct stream_filter *);
int close_istream(struct git_istream *);
ssize_t read_istream(struct git_istream *, void *, size_t);

int stream_blob_to_fd(int fd, const struct object_id *, struct stream_filter *, int can_seek);

#endif 
