#include "cache.h"

const signed char hexval_table[256] = {
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
0, 1, 2, 3, 4, 5, 6, 7, 
8, 9, -1, -1, -1, -1, -1, -1, 
-1, 10, 11, 12, 13, 14, 15, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, 10, 11, 12, 13, 14, 15, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, 
};

int hex_to_bytes(unsigned char *binary, const char *hex, size_t len)
{
for (; len; len--, hex += 2) {
unsigned int val = (hexval(hex[0]) << 4) | hexval(hex[1]);

if (val & ~0xff)
return -1;
*binary++ = val;
}
return 0;
}

static int get_hash_hex_algop(const char *hex, unsigned char *hash,
const struct git_hash_algo *algop)
{
int i;
for (i = 0; i < algop->rawsz; i++) {
int val = hex2chr(hex);
if (val < 0)
return -1;
*hash++ = val;
hex += 2;
}
return 0;
}

int get_sha1_hex(const char *hex, unsigned char *sha1)
{
return get_hash_hex_algop(hex, sha1, the_hash_algo);
}

int get_oid_hex_algop(const char *hex, struct object_id *oid,
const struct git_hash_algo *algop)
{
return get_hash_hex_algop(hex, oid->hash, algop);
}





int get_oid_hex_any(const char *hex, struct object_id *oid)
{
int i;
for (i = GIT_HASH_NALGOS - 1; i > 0; i--) {
if (!get_hash_hex_algop(hex, oid->hash, &hash_algos[i]))
return i;
}
return GIT_HASH_UNKNOWN;
}

int get_oid_hex(const char *hex, struct object_id *oid)
{
return get_oid_hex_algop(hex, oid, the_hash_algo);
}

int parse_oid_hex_algop(const char *hex, struct object_id *oid,
const char **end,
const struct git_hash_algo *algop)
{
int ret = get_hash_hex_algop(hex, oid->hash, algop);
if (!ret)
*end = hex + algop->hexsz;
return ret;
}

int parse_oid_hex_any(const char *hex, struct object_id *oid, const char **end)
{
int ret = get_oid_hex_any(hex, oid);
if (ret)
*end = hex + hash_algos[ret].hexsz;
return ret;
}

int parse_oid_hex(const char *hex, struct object_id *oid, const char **end)
{
return parse_oid_hex_algop(hex, oid, end, the_hash_algo);
}

char *hash_to_hex_algop_r(char *buffer, const unsigned char *hash,
const struct git_hash_algo *algop)
{
static const char hex[] = "0123456789abcdef";
char *buf = buffer;
int i;

for (i = 0; i < algop->rawsz; i++) {
unsigned int val = *hash++;
*buf++ = hex[val >> 4];
*buf++ = hex[val & 0xf];
}
*buf = '\0';

return buffer;
}

char *oid_to_hex_r(char *buffer, const struct object_id *oid)
{
return hash_to_hex_algop_r(buffer, oid->hash, the_hash_algo);
}

char *hash_to_hex_algop(const unsigned char *hash, const struct git_hash_algo *algop)
{
static int bufno;
static char hexbuffer[4][GIT_MAX_HEXSZ + 1];
bufno = (bufno + 1) % ARRAY_SIZE(hexbuffer);
return hash_to_hex_algop_r(hexbuffer[bufno], hash, algop);
}

char *hash_to_hex(const unsigned char *hash)
{
return hash_to_hex_algop(hash, the_hash_algo);
}

char *oid_to_hex(const struct object_id *oid)
{
return hash_to_hex_algop(oid->hash, the_hash_algo);
}
