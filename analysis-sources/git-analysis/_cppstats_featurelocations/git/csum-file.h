#if !defined(CSUM_FILE_H)
#define CSUM_FILE_H

#include "hash.h"

struct progress;


struct hashfile {
int fd;
int check_fd;
unsigned int offset;
git_hash_ctx ctx;
off_t total;
struct progress *tp;
const char *name;
int do_crc;
uint32_t crc32;
unsigned char buffer[8192];
};


struct hashfile_checkpoint {
off_t offset;
git_hash_ctx ctx;
};

void hashfile_checkpoint(struct hashfile *, struct hashfile_checkpoint *);
int hashfile_truncate(struct hashfile *, struct hashfile_checkpoint *);


#define CSUM_CLOSE 1
#define CSUM_FSYNC 2
#define CSUM_HASH_IN_STREAM 4

struct hashfile *hashfd(int fd, const char *name);
struct hashfile *hashfd_check(const char *name);
struct hashfile *hashfd_throughput(int fd, const char *name, struct progress *tp);
int finalize_hashfile(struct hashfile *, unsigned char *, unsigned int);
void hashwrite(struct hashfile *, const void *, unsigned int);
void hashflush(struct hashfile *f);
void crc32_begin(struct hashfile *);
uint32_t crc32_end(struct hashfile *);





static inline off_t hashfile_total(struct hashfile *f)
{
return f->total + f->offset;
}

static inline void hashwrite_u8(struct hashfile *f, uint8_t data)
{
hashwrite(f, &data, sizeof(data));
}

static inline void hashwrite_be32(struct hashfile *f, uint32_t data)
{
data = htonl(data);
hashwrite(f, &data, sizeof(data));
}

#endif
