#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif
#include <stdatomic.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <interface/vcsm/user-vcsm.h>
#include <vlc_common.h>
#include <vlc_picture.h>
#include "mmal_cma.h"
#include "mmal_picture.h"
#include <assert.h>
struct cma_pool_fixed_s
{
atomic_int ref_count;
vlc_mutex_t lock;
unsigned int n_in;
unsigned int n_out;
unsigned int pool_size;
int flight_size;
size_t el_size;
cma_buf_t ** pool;
int in_flight;
vlc_cond_t flight_cond;
cma_buf_pool_t * alloc_v;
char * name;
};
static unsigned int inc_mod(const unsigned int n, const unsigned int m)
{
return n + 1 >= m ? 0 : n + 1;
}
static cma_buf_t * cma_pool_alloc_cb(cma_buf_pool_t * const, size_t);
static void cma_pool_delete(cma_buf_t * const);
static void cma_buf_pool_on_delete_cb(cma_buf_pool_t * const);
static void free_pool(cma_buf_t ** const pool, const unsigned int pool_size)
{
if (pool == NULL)
return;
for (unsigned int n = 0; n != pool_size; ++n)
if (pool[n] != NULL)
cma_pool_delete(pool[n]);
free(pool);
}
static void cma_pool_fixed_delete(cma_pool_fixed_t * const p)
{
cma_buf_pool_t * const v = p->alloc_v;
free_pool(p->pool, p->pool_size);
free(p->name);
free(p);
cma_buf_pool_on_delete_cb(v);
}
static void cma_pool_fixed_unref(cma_pool_fixed_t * const p)
{
if (atomic_fetch_sub(&p->ref_count, 1) <= 1)
cma_pool_fixed_delete(p);
}
static void cma_pool_fixed_ref(cma_pool_fixed_t * const p)
{
atomic_fetch_add(&p->ref_count, 1);
}
static void cma_pool_fixed_dec_in_flight(cma_pool_fixed_t * const p)
{
vlc_mutex_lock(&p->lock);
if (--p->in_flight == 0)
vlc_cond_signal(&p->flight_cond);
vlc_mutex_unlock(&p->lock);
}
static cma_buf_t * cma_pool_fixed_get(cma_pool_fixed_t * const p, const size_t req_el_size, const bool inc_flight, const bool no_pool)
{
cma_buf_t * v = NULL;
vlc_mutex_lock(&p->lock);
for (;;)
{
if (req_el_size != p->el_size)
{
cma_buf_t ** const deadpool = p->pool;
const unsigned int dead_n = p->pool_size;
p->pool = NULL;
p->n_in = 0;
p->n_out = 0;
p->el_size = req_el_size;
if (deadpool != NULL)
{
vlc_mutex_unlock(&p->lock);
free_pool(deadpool, dead_n);
vlc_mutex_lock(&p->lock);
continue;
}
}
if (req_el_size == 0)
{
vlc_mutex_unlock(&p->lock);
return NULL;
}
if (p->pool != NULL && !no_pool)
{
v = p->pool[p->n_in];
if (v != NULL)
{
p->pool[p->n_in] = NULL;
p->n_in = inc_mod(p->n_in, p->pool_size);
break;
}
}
if (p->in_flight <= 0)
break;
vlc_cond_wait(&p->flight_cond, &p->lock);
}
if (inc_flight)
++p->in_flight;
vlc_mutex_unlock(&p->lock);
if (v == NULL && req_el_size != 0)
v = cma_pool_alloc_cb(p->alloc_v, req_el_size);
if (v != NULL)
cma_pool_fixed_ref(p);
else if (inc_flight)
cma_pool_fixed_dec_in_flight(p);
return v;
}
static void cma_pool_fixed_put(cma_pool_fixed_t * const p, cma_buf_t * v, const size_t el_size, const bool was_in_flight)
{
vlc_mutex_lock(&p->lock);
if (el_size == p->el_size && (p->pool == NULL || p->pool[p->n_out] == NULL))
{
if (p->pool == NULL)
p->pool = calloc(p->pool_size, sizeof(void*));
p->pool[p->n_out] = v;
p->n_out = inc_mod(p->n_out, p->pool_size);
v = NULL;
}
if (was_in_flight)
--p->in_flight;
vlc_mutex_unlock(&p->lock);
vlc_cond_signal(&p->flight_cond);
if (v != NULL)
cma_pool_delete(v);
cma_pool_fixed_unref(p);
}
static void cma_pool_fixed_kill(cma_pool_fixed_t * const p)
{
if (p == NULL)
return;
cma_pool_fixed_get(p, 0, false, false);
cma_pool_fixed_unref(p);
}
static cma_pool_fixed_t*
cma_pool_fixed_new(const unsigned int pool_size,
const int flight_size,
cma_buf_pool_t * const alloc_v,
const char * const name)
{
cma_pool_fixed_t* const p = calloc(1, sizeof(cma_pool_fixed_t));
if (p == NULL)
return NULL;
atomic_store(&p->ref_count, 1);
vlc_mutex_init(&p->lock);
vlc_cond_init(&p->flight_cond);
p->pool_size = pool_size;
p->flight_size = flight_size;
p->in_flight = -flight_size;
p->alloc_v = alloc_v;
p->name = name == NULL ? NULL : strdup(name);
return p;
}
struct cma_buf_pool_s {
cma_pool_fixed_t * pool;
bool is_cma;
};
typedef struct cma_buf_s {
atomic_int ref_count;
cma_buf_pool_t * cbp;
bool in_flight;
size_t size;
unsigned int vcsm_h; 
unsigned int vc_h; 
int fd; 
void * mmap; 
} cma_buf_t;
static void cma_pool_delete(cma_buf_t * const cb)
{
assert(atomic_load(&cb->ref_count) == 0);
if (cb->mmap != MAP_FAILED)
{
if (cb->cbp->is_cma)
munmap(cb->mmap, cb->size);
else
vcsm_unlock_hdl(cb->vcsm_h);
}
if (cb->fd != -1)
close(cb->fd);
if (cb->vcsm_h != 0)
vcsm_free(cb->vcsm_h);
free(cb);
}
static cma_buf_t * cma_pool_alloc_cb(cma_buf_pool_t * const v, size_t size)
{
cma_buf_pool_t * const cbp = v;
cma_buf_t * const cb = malloc(sizeof(cma_buf_t));
if (cb == NULL)
return NULL;
*cb = (cma_buf_t){
.ref_count = ATOMIC_VAR_INIT(0),
.cbp = cbp,
.in_flight = 0,
.size = size,
.vcsm_h = 0,
.vc_h = 0,
.fd = -1,
.mmap = MAP_FAILED,
};
if ((cb->vcsm_h = vcsm_malloc_cache(size, VCSM_CACHE_TYPE_HOST | 0x80, "VLC frame")) == 0)
{
goto fail;
}
if ((cb->vc_h = vcsm_vc_hdl_from_hdl(cb->vcsm_h)) == 0)
{
goto fail;
}
if (cbp->is_cma)
{
if ((cb->fd = vcsm_export_dmabuf(cb->vcsm_h)) == -1)
{
goto fail;
}
if ((cb->mmap = mmap(NULL, cb->size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, cb->fd, 0)) == MAP_FAILED)
goto fail;
}
else
{
void * arm_addr;
if ((arm_addr = vcsm_lock(cb->vcsm_h)) == NULL)
{
goto fail;
}
cb->mmap = arm_addr;
}
return cb;
fail:
cma_pool_delete(cb);
return NULL;
}
static void cma_buf_pool_on_delete_cb(cma_buf_pool_t * const cbp)
{
free(cbp);
}
void cma_buf_pool_delete(cma_buf_pool_t * const cbp)
{
if (cbp == NULL)
return;
if (cbp->pool != NULL)
{
cma_pool_fixed_kill(cbp->pool);
}
else
{
cma_buf_pool_on_delete_cb(cbp);
}
}
cma_buf_pool_t * cma_buf_pool_new(const unsigned int pool_size, const unsigned int flight_size,
bool is_cma, const char * const name)
{
cma_buf_pool_t * const cbp = calloc(1, sizeof(cma_buf_pool_t));
if (cbp == NULL)
return NULL;
cbp->is_cma = is_cma;
if ((cbp->pool = cma_pool_fixed_new(pool_size, flight_size, cbp, name)) == NULL)
goto fail;
return cbp;
fail:
cma_buf_pool_delete(cbp);
return NULL;
}
size_t cma_buf_size(const cma_buf_t * const cb)
{
return cb->size;
}
unsigned int cma_buf_vc_handle(const cma_buf_t *const cb)
{
return cb->vc_h;
}
void * cma_buf_addr(const cma_buf_t *const cb)
{
return cb->mmap;
}
void cma_buf_unref(cma_buf_t * const cb)
{
if (cb == NULL)
return;
if (atomic_fetch_sub(&cb->ref_count, 1) <= 1)
{
const bool was_in_flight = cb->in_flight;
cb->in_flight = false;
cma_pool_fixed_put(cb->cbp->pool, cb, cb->size, was_in_flight);
}
}
cma_buf_t * cma_buf_ref(cma_buf_t * const cb)
{
if (cb == NULL)
return NULL;
atomic_fetch_add(&cb->ref_count, 1);
return cb;
}
cma_buf_t * cma_buf_pool_alloc_buf(cma_buf_pool_t * const cbp, const size_t size)
{
cma_buf_t *const cb = cma_pool_fixed_get(cbp->pool, size, true, false);
if (cb == NULL)
return NULL;
cb->in_flight = true;
return cma_buf_ref(cb);
}
