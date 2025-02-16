



#include <r_util.h>


R_API RCache *r_cache_new() {
RCache *c = R_NEW0 (RCache);
if (!c) {
return NULL;
}
c->buf = NULL;
c->base = 0;
c->len = 0;
return c;
}

R_API void r_cache_free(RCache *c) {
if (c) {
free (c->buf);
}
free (c);
}

R_API const ut8 *r_cache_get(RCache *c, ut64 addr, int *len) {
if (!c->buf) {
return NULL;
}
if (len) {
*len = c->base - addr;
}
if (addr < c->base) {
return NULL;
}
if (addr > (c->base + c->len)) {
return NULL;
}
if (len) {
*len = c->len - (addr - c->base);
}

return c->buf + (addr - c->base);
}

R_API int r_cache_set(RCache *c, ut64 addr, const ut8 *buf, int len) {
if (!c) {
return 0;
}
if (!c->buf) {
c->buf = malloc (len);
if (!c->buf) {
return 0;
}
memcpy (c->buf, buf, len);
c->base = addr;
c->len = len;
} else if (addr < c->base) {
ut8 *b;
int baselen = (c->base - addr);
int newlen = baselen + ((len > c->len)? len: c->base);

b = malloc (newlen);
if (!b) {
return 0;
}
memset (b, 0xff, newlen);
memcpy (b + baselen, c->buf, c->len);
memcpy (b, buf, len);
free (c->buf);
c->buf = b;
c->base = addr;
c->len = newlen;
} else if ((addr + len) > (c->base + c->len)) {
ut8 *b;
int baselen = (addr - c->base);
int newlen = baselen + len;
b = realloc (c->buf, newlen);
if (!b) {
return 0;
}
memcpy (b + baselen, buf, len);
c->buf = b;
c->len = newlen;
} else {
memcpy (c->buf, buf, len);
}
return c->len;
}

R_API void r_cache_flush(RCache *c) {
if (c) {
c->base = 0;
c->len = 0;
R_FREE (c->buf);
}
}
