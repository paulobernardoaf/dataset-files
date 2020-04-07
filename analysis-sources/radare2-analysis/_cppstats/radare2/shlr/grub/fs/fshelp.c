#include <grub/err.h>
#include <grub/mm.h>
#include <grub/misc.h>
#include <grub/disk.h>
#include <grub/fshelp.h>
#include <stdlib.h>
GRUB_EXPORT(grub_fshelp_view);
GRUB_EXPORT(grub_fshelp_find_file);
GRUB_EXPORT(grub_fshelp_log2blksize);
GRUB_EXPORT(grub_fshelp_read_file);
int grub_fshelp_view = 0;
struct grub_fshelp_find_file_closure
{
grub_fshelp_node_t rootnode;
int (*iterate_dir) (grub_fshelp_node_t dir,
int (*hook)
(const char *filename,
enum grub_fshelp_filetype filetype,
grub_fshelp_node_t node, void *closure),
void *closure);
void *closure;
char *(*read_symlink) (grub_fshelp_node_t node);
int symlinknest;
enum grub_fshelp_filetype foundtype;
grub_fshelp_node_t currroot;
};
static void
free_node (grub_fshelp_node_t node, struct grub_fshelp_find_file_closure *c)
{
if (node != c->rootnode && node != c->currroot)
grub_free (node);
}
struct find_file_closure
{
char *name;
enum grub_fshelp_filetype *type;
grub_fshelp_node_t *oldnode;
grub_fshelp_node_t *currnode;
};
static int
iterate (const char *filename,
enum grub_fshelp_filetype filetype,
grub_fshelp_node_t node,
void *closure)
{
struct find_file_closure *c = closure;
if (filetype == GRUB_FSHELP_UNKNOWN ||
(grub_strcmp (c->name, filename) &&
(! (filetype & GRUB_FSHELP_CASE_INSENSITIVE) ||
grub_strncasecmp (c->name, filename, GRUB_LONG_MAX))))
{
grub_free (node);
return 0;
}
*(c->type) = filetype & ~GRUB_FSHELP_CASE_INSENSITIVE;
*(c->oldnode) = *(c->currnode);
*(c->currnode) = node;
return 1;
}
static grub_err_t
find_file (const char *currpath, grub_fshelp_node_t currroot,
grub_fshelp_node_t *currfound,
struct grub_fshelp_find_file_closure *c)
{
char *fpath = grub_malloc (grub_strlen (currpath) + 1);
char *name = fpath;
char *next;
enum grub_fshelp_filetype type = GRUB_FSHELP_DIR;
grub_fshelp_node_t currnode = currroot;
grub_fshelp_node_t oldnode = currroot;
c->currroot = currroot;
grub_strncpy (fpath, currpath, grub_strlen (currpath) + 1);
while (*name == '/')
name++;
if (! *name)
{
*currfound = currnode;
grub_free (fpath);
return 0;
}
for (;;)
{
int found;
struct find_file_closure cc;
next = grub_strchr (name, '/');
if (next)
{
while (*next == '/')
*(next++) = '\0';
}
if (type != GRUB_FSHELP_DIR)
{
free_node (currnode, c);
grub_free (fpath);
return grub_error (GRUB_ERR_BAD_FILE_TYPE, "not a directory");
}
cc.name = name;
cc.type = &type;
cc.oldnode = &oldnode;
cc.currnode = &currnode;
found = c->iterate_dir (currnode, iterate, &cc);
if (! found)
{
if (grub_errno) {
grub_free (fpath);
return grub_errno;
}
break;
}
if (type == GRUB_FSHELP_SYMLINK)
{
char *symlink;
if (++(c->symlinknest) == 8)
{
free_node (currnode, c);
free_node (oldnode, c);
grub_free (fpath);
return grub_error (GRUB_ERR_SYMLINK_LOOP,
"too deep nesting of symlinks");
}
symlink = c->read_symlink (currnode);
free_node (currnode, c);
if (!symlink)
{
free_node (oldnode, c);
grub_free (fpath);
return grub_errno;
}
if (symlink[0] == '/')
{
free_node (oldnode, c);
oldnode = c->rootnode;
}
find_file (symlink, oldnode, &currnode, c);
type = c->foundtype;
grub_free (symlink);
if (grub_errno)
{
free_node (oldnode, c);
grub_free (fpath);
return grub_errno;
}
}
free_node (oldnode, c);
if (! next || *next == '\0')
{
*currfound = currnode;
c->foundtype = type;
grub_free (fpath);
return 0;
}
name = next;
}
grub_free (fpath);
return grub_error (GRUB_ERR_FILE_NOT_FOUND, "file not found");
}
grub_err_t
grub_fshelp_find_file (const char *path, grub_fshelp_node_t rootnode,
grub_fshelp_node_t *foundnode,
int (*iterate_dir) (grub_fshelp_node_t dir,
int (*hook)
(const char *filename,
enum grub_fshelp_filetype filetype,
grub_fshelp_node_t node,
void *closure),
void *closure),
void *closure,
char *(*read_symlink) (grub_fshelp_node_t node),
enum grub_fshelp_filetype expecttype)
{
grub_err_t err;
struct grub_fshelp_find_file_closure c;
c.rootnode = rootnode;
c.iterate_dir = iterate_dir;
c.closure = closure;
c.read_symlink = read_symlink;
c.symlinknest = 0;
c.foundtype = GRUB_FSHELP_DIR;
if (!path || path[0] != '/')
{
grub_error (GRUB_ERR_BAD_FILENAME, "bad filename");
return grub_errno;
}
err = find_file (path, rootnode, foundnode, &c);
if (err)
return err;
if (expecttype == GRUB_FSHELP_REG && c.foundtype != expecttype)
return grub_error (GRUB_ERR_BAD_FILE_TYPE, "not a regular file");
else if (expecttype == GRUB_FSHELP_DIR && c.foundtype != expecttype)
return grub_error (GRUB_ERR_BAD_FILE_TYPE, "not a directory");
return 0;
}
unsigned long long grub_hack_lastoff = 0;
grub_ssize_t
grub_fshelp_read_file (grub_disk_t disk, grub_fshelp_node_t node,
void (*read_hook) (grub_disk_addr_t sector,
unsigned offset,
unsigned length,
void *closure),
void *closure, int flags,
grub_off_t pos, grub_size_t len, char *buf,
grub_disk_addr_t (*get_block) (grub_fshelp_node_t node,
grub_disk_addr_t block),
grub_off_t filesize, int log2blocksize)
{
grub_disk_addr_t i, blockcnt;
int blocksize = 1 << (log2blocksize + GRUB_DISK_SECTOR_BITS);
if (pos + len > filesize)
len = filesize - pos;
if (len < 1 || len == 0xffffffff) {
return -1;
}
blockcnt = ((len + pos) + blocksize - 1) >>
(log2blocksize + GRUB_DISK_SECTOR_BITS);
for (i = pos >> (log2blocksize + GRUB_DISK_SECTOR_BITS); i < blockcnt; i++)
{
grub_disk_addr_t blknr;
int blockoff = pos & (blocksize - 1);
int blockend = blocksize;
int skipfirst = 0;
blknr = get_block (node, i);
if (grub_errno)
return -1;
blknr = blknr << log2blocksize;
if (i == blockcnt - 1)
{
blockend = (len + pos) & (blocksize - 1);
if (! blockend)
blockend = blocksize;
}
if (i == (pos >> (log2blocksize + GRUB_DISK_SECTOR_BITS)))
{
skipfirst = blockoff;
blockend -= skipfirst;
}
if (blknr)
{
disk->read_hook = read_hook;
disk->closure = closure;
grub_hack_lastoff = blknr * 512;
grub_disk_read_ex (disk, blknr, skipfirst, blockend, buf, flags);
disk->read_hook = 0;
if (grub_errno)
return -1;
}
else if (buf)
grub_memset (buf, 0, blockend);
if (buf)
buf += blocksize - skipfirst;
}
return len;
}
unsigned int
grub_fshelp_log2blksize (unsigned int blksize, unsigned int *pow)
{
int mod;
*pow = 0;
while (blksize > 1)
{
mod = blksize - ((blksize >> 1) << 1);
blksize >>= 1;
if (mod)
return grub_error (GRUB_ERR_BAD_NUMBER,
"the blocksize is not a power of two");
(*pow)++;
}
return GRUB_ERR_NONE;
}
