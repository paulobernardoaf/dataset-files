


















#if !defined(GRUB_FSHELP_HEADER)
#define GRUB_FSHELP_HEADER 1

#include <grub/types.h>
#include <grub/symbol.h>
#include <grub/err.h>
#include <grub/disk.h>

typedef struct grub_fshelp_node *grub_fshelp_node_t;

extern int grub_fshelp_view;
#define GRUB_FSHELP_CASE_INSENSITIVE 0x100
#define GRUB_FSHELP_TYPE_MASK 0xff
#define GRUB_FSHELP_FLAGS_MASK 0x100

enum grub_fshelp_filetype
{
GRUB_FSHELP_UNKNOWN,
GRUB_FSHELP_REG,
GRUB_FSHELP_DIR,
GRUB_FSHELP_SYMLINK
};










grub_err_t grub_fshelp_find_file (const char *path,
grub_fshelp_node_t rootnode,
grub_fshelp_node_t *foundnode,
int (*iterate_dir)
(grub_fshelp_node_t dir,
int (*hook)
(const char *filename,
enum grub_fshelp_filetype filetype,
grub_fshelp_node_t node,
void *closure),
void *closure),
void *closure,
char *(*read_symlink) (grub_fshelp_node_t node),
enum grub_fshelp_filetype expect);








grub_ssize_t grub_fshelp_read_file (grub_disk_t disk, grub_fshelp_node_t node,
void (*read_hook)
(grub_disk_addr_t sector,
unsigned offset,
unsigned length,
void *closure),
void *closure, int flags,
grub_off_t pos, grub_size_t len, char *buf,
grub_disk_addr_t (*get_block)
(grub_fshelp_node_t node,
grub_disk_addr_t block),
grub_off_t filesize, int log2blocksize);

unsigned int grub_fshelp_log2blksize (unsigned int blksize,
unsigned int *pow);

#endif 
