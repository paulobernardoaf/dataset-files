






















#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <vlc_common.h>
#include <vlc_block.h>
#include <vlc_fs.h>

#if !defined(NDEBUG)
static void block_Check (block_t *block)
{
while (block != NULL)
{
unsigned char *start = block->p_start;
unsigned char *end = block->p_start + block->i_size;
unsigned char *bufstart = block->p_buffer;
unsigned char *bufend = block->p_buffer + block->i_buffer;

assert (start <= end);
assert (bufstart <= bufend);
assert (bufstart >= start);
assert (bufend <= end);

block = block->p_next;
}
}
#else
#define block_Check(b) ((void)(b))
#endif

block_t *block_Init(block_t *restrict b, const struct vlc_block_callbacks *cbs,
void *buf, size_t size)
{

b->p_next = NULL;
b->p_buffer = buf;
b->i_buffer = size;
b->p_start = buf;
b->i_size = size;
b->i_flags = 0;
b->i_nb_samples = 0;
b->i_pts =
b->i_dts = VLC_TICK_INVALID;
b->i_length = 0;
b->cbs = cbs;
return b;
}

static void block_generic_Release (block_t *block)
{

assert (block->p_start == (unsigned char *)(block + 1));
free (block);
}

static const struct vlc_block_callbacks block_generic_cbs =
{
block_generic_Release,
};

static void BlockMetaCopy( block_t *restrict out, const block_t *in )
{
out->p_next = in->p_next;
out->i_nb_samples = in->i_nb_samples;
out->i_dts = in->i_dts;
out->i_pts = in->i_pts;
out->i_flags = in->i_flags;
out->i_length = in->i_length;
}




#define BLOCK_ALIGN 32


#define BLOCK_PADDING 32

block_t *block_Alloc (size_t size)
{
if (unlikely(size >> 27))
{
errno = ENOBUFS;
return NULL;
}


const size_t alloc = sizeof (block_t) + BLOCK_ALIGN + (2 * BLOCK_PADDING)
+ size;
if (unlikely(alloc <= size))
return NULL;

block_t *b = malloc (alloc);
if (unlikely(b == NULL))
return NULL;

block_Init(b, &block_generic_cbs, b + 1, alloc - sizeof (*b));
static_assert ((BLOCK_PADDING % BLOCK_ALIGN) == 0,
"BLOCK_PADDING must be a multiple of BLOCK_ALIGN");
b->p_buffer += BLOCK_PADDING + BLOCK_ALIGN - 1;
b->p_buffer = (void *)(((uintptr_t)b->p_buffer) & ~(BLOCK_ALIGN - 1));
b->i_buffer = size;
return b;
}

void block_Release(block_t *block)
{
#if !defined(NDEBUG)
block->p_next = NULL;
block_Check (block);
#endif
block->cbs->free(block);
}

block_t *block_TryRealloc (block_t *p_block, ssize_t i_prebody, size_t i_body)
{
block_Check( p_block );


if( i_prebody <= 0 && i_body <= (size_t)(-i_prebody) )
i_prebody = i_body = 0;

assert( p_block->p_start <= p_block->p_buffer );
assert( p_block->p_start + p_block->i_size
>= p_block->p_buffer + p_block->i_buffer );




if( i_prebody < 0 )
{
if( p_block->i_buffer >= (size_t)-i_prebody )
{
p_block->p_buffer -= i_prebody;
p_block->i_buffer += i_prebody;
}
else 
p_block->i_buffer = 0;
i_body += i_prebody;
i_prebody = 0;
}


if( p_block->i_buffer > i_body )
p_block->i_buffer = i_body;

size_t requested = i_prebody + i_body;

if( p_block->i_buffer == 0 )
{ 
if( requested <= p_block->i_size )
{ 
size_t extra = p_block->i_size - requested;

p_block->p_buffer = p_block->p_start + (extra / 2);
p_block->i_buffer = requested;
return p_block;
}


block_t *p_rea = block_Alloc( requested );
if( p_rea == NULL )
return NULL;

BlockMetaCopy( p_rea, p_block );
block_Release( p_block );
return p_rea;
}

uint8_t *p_start = p_block->p_start;
uint8_t *p_end = p_start + p_block->i_size;


assert( i_prebody >= 0 );
if( (size_t)(p_block->p_buffer - p_start) < (size_t)i_prebody
|| (size_t)(p_end - p_block->p_buffer) < i_body )
{
block_t *p_rea = block_Alloc( requested );
if( p_rea == NULL )
return NULL;

memcpy( p_rea->p_buffer + i_prebody, p_block->p_buffer,
p_block->i_buffer );
BlockMetaCopy( p_rea, p_block );
block_Release( p_block );
return p_rea;
}




if( i_prebody > 0 )
{
p_block->p_buffer -= i_prebody;
p_block->i_buffer += i_prebody;
i_body += i_prebody;
i_prebody = 0;
}


p_block->i_buffer = i_body;

return p_block;
}

block_t *block_Realloc (block_t *block, ssize_t prebody, size_t body)
{
block_t *rea = block_TryRealloc (block, prebody, body);
if (rea == NULL)
block_Release(block);
return rea;
}

static void block_heap_Release (block_t *block)
{
free (block->p_start);
free (block);
}

static const struct vlc_block_callbacks block_heap_cbs =
{
block_heap_Release,
};

block_t *block_heap_Alloc (void *addr, size_t length)
{
block_t *block = malloc (sizeof (*block));
if (block == NULL)
{
free (addr);
return NULL;
}

return block_Init(block, &block_heap_cbs, addr, length);
}

#if defined(HAVE_MMAP)
#include <sys/mman.h>

static void block_mmap_Release (block_t *block)
{
munmap (block->p_start, block->i_size);
free (block);
}

static const struct vlc_block_callbacks block_mmap_cbs =
{
block_mmap_Release,
};

block_t *block_mmap_Alloc (void *addr, size_t length)
{
if (addr == MAP_FAILED)
return NULL;

long page_mask = sysconf(_SC_PAGESIZE) - 1;
size_t left = ((uintptr_t)addr) & page_mask;
size_t right = (-length) & page_mask;

block_t *block = malloc (sizeof (*block));
if (block == NULL)
{
munmap (addr, length);
return NULL;
}

block_Init(block, &block_mmap_cbs,
((char *)addr) - left, left + length + right);
block->p_buffer = addr;
block->i_buffer = length;
return block;
}
#else
block_t *block_mmap_Alloc (void *addr, size_t length)
{
(void)addr; (void)length; return NULL;
}
#endif

#if defined(HAVE_SYS_SHM_H)
#include <sys/shm.h>

static void block_shm_Release (block_t *block)
{
shmdt(block->p_start);
free(block);
}

static const struct vlc_block_callbacks block_shm_cbs =
{
block_shm_Release,
};

block_t *block_shm_Alloc (void *addr, size_t length)
{
block_t *block = malloc (sizeof (*block));
if (unlikely(block == NULL))
{
shmdt (addr);
return NULL;
}

return block_Init(block, &block_shm_cbs, (uint8_t *)addr, length);
}
#else
block_t *block_shm_Alloc (void *addr, size_t length)
{
(void) addr; (void) length;
abort ();
}
#endif


#if defined(_WIN32)
#include <io.h>

static
ssize_t pread (int fd, void *buf, size_t count, off_t offset)
{
HANDLE handle = (HANDLE)(intptr_t)_get_osfhandle (fd);
if (handle == INVALID_HANDLE_VALUE)
return -1;

OVERLAPPED olap = {.Offset = offset, .OffsetHigh = (offset >> 32)};
DWORD written;



if (ReadFile (handle, buf, count, &written, &olap))
return written;
return -1;
}
#endif

block_t *block_File(int fd, bool write)
{
size_t length;
struct stat st;


if (fstat (fd, &st))
return NULL;




#if !defined(S_TYPEISSHM)
#define S_TYPEISSHM( buf ) (0)
#endif
if (S_ISDIR (st.st_mode))
{
errno = EISDIR;
return NULL;
}
if (!S_ISREG (st.st_mode) && !S_TYPEISSHM (&st))
{
errno = ESPIPE;
return NULL;
}


if ((uintmax_t)st.st_size >= SIZE_MAX)
{
errno = ENOMEM;
return NULL;
}
length = (size_t)st.st_size;

#if defined(HAVE_MMAP)
if (length > 0)
{
int prot = PROT_READ | (write ? PROT_WRITE : 0);
int flags = write ? MAP_PRIVATE : MAP_SHARED;
void *addr = mmap(NULL, length, prot, flags, fd, 0);

if (addr != MAP_FAILED)
return block_mmap_Alloc (addr, length);
}
#else
(void) write;
#endif


block_t *block = block_Alloc (length);
if (block == NULL)
return NULL;
block_cleanup_push (block);

for (size_t i = 0; i < length;)
{
ssize_t len = pread (fd, block->p_buffer + i, length - i, i);
if (len == -1)
{
block_Release (block);
block = NULL;
break;
}
i += len;
}
vlc_cleanup_pop ();
return block;
}

block_t *block_FilePath(const char *path, bool write)
{


int fd = vlc_open (path, O_RDONLY);
if (fd == -1)
return NULL;

block_t *block = block_File(fd, write);
vlc_close (fd);
return block;
}
