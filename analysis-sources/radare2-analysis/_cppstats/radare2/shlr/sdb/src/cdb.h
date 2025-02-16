#include <string.h>
#include "types.h"
#define KVLSZ 4
#define CDB_MAX_KEY 0xff
#define CDB_MAX_VALUE 0xffffff
#define CDB_HASHSTART 5381
struct cdb {
char *map; 
int fd; 
ut32 size; 
ut32 loop; 
ut32 khash; 
ut32 kpos; 
ut32 hpos; 
ut32 hslots; 
ut32 dpos; 
ut32 dlen; 
};
bool cdb_getkvlen(struct cdb *db, ut32 *klen, ut32 *vlen, ut32 pos);
void cdb_free(struct cdb *);
bool cdb_init(struct cdb *, int fd);
void cdb_findstart(struct cdb *);
bool cdb_read(struct cdb *, char *, unsigned int, ut32);
int cdb_findnext(struct cdb *, ut32 u, const char *, ut32);
#define cdb_datapos(c) ((c)->dpos)
#define cdb_datalen(c) ((c)->dlen)
