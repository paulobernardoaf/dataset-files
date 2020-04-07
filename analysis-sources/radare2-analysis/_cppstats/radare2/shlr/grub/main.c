#include <grub/file.h>
#include <grub/disk.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#define IMGFILE "/tmp/test.fs.img"
extern struct grub_fs grub_ext2_fs;
grub_err_t read_foo (struct grub_disk *disk, grub_disk_addr_t sector, grub_size_t size, char *buf) {
size=512;
{
FILE *fd = fopen (IMGFILE, "rb");
if (fd) {
fseek (fd, (512*sector), SEEK_SET);
fread (buf, 1, size, fd);
fclose (fd);
} else printf ("Cannot open "IMGFILE"\n");
}
return 0;
}
void read_hook (grub_disk_addr_t sector, unsigned long offset, unsigned long length, unsigned char *buf) {
{
int size=length;
FILE *fd = fopen(IMGFILE, "rb");
if (fd) {
fseek (fd, (512*sector)+offset, SEEK_SET);
fread (buf, 1, size, fd);
fclose (fd);
} else printf ("Cannot open "IMGFILE"\n");
}
}
grub_file_t openimage(grub_fs_t fs, const char *str) {
grub_file_t file = malloc (1024);
file->device = malloc (1024);
memset (file->device, 0, 1024);
file->device->disk = malloc (1024);
memset (file->device->disk, 0, 1024);
file->device->disk->dev = (grub_disk_dev_t)file->device;
file->device->disk->dev->read = read_foo; 
file->fs = fs;
#if 0
file->offset = 0;
file->size = 12190208;
file->data = malloc (file->size);
{
FILE *fd = fopen(IMGFILE, "rb");
if (fd == NULL) {
printf ("Cannot open fs image\n");
return NULL;
}
fread (file->data, file->size, 1, fd);
fclose (fd);
}
#endif
return file;
}
int dirhook (const char *filename, const struct grub_dirhook_info *info, void *closure) {
printf ("DIRFILE: %c (%d) %s\n", info->dir?'d':'f', 
info->mtime, filename);
return 0;
}
int do_main() {
struct grub_file *file;
struct grub_fs *e2;
grub_err_t err;
e2 = &grub_ext2_fs;
file = openimage (e2, IMGFILE);
if (file == NULL) {
printf ("oops\n");
return 0;
}
err = e2->open (file, "/test");
if (err == 0) {
char buf[1024];
err = e2->read (file, buf, file->size);
write (1, buf, file->size);
e2->close (file);
err = e2->dir (file->device, "/", dirhook, 0);
if (err != 0)
grub_print_error ();
} else {
grub_print_error ();
printf ("error is : %d\n", err);
return 0;
}
return 1;
}
#include "grubfs.h"
int foo_main() {
char buf[1024];
GrubFS *gfs = grubfs_new (&grub_ext2_fs, NULL);
gfs->file->fs->open (gfs->file, "/test");
gfs->file->fs->read (gfs->file, buf, gfs->file->size);
printf ("fs = %d\n", (int)gfs->file->size);
write (1, buf, gfs->file->size);
gfs->file->fs->close (gfs->file);
gfs->file->fs->dir (gfs->file->device, "/", dirhook, 0);
grubfs_free (gfs);
return 0;
}
int main(int argc, char **argv) {
if (argc>1) {
printf ("grubfs api\n");
return foo_main ();
}
printf ("grub internal api\n");
if (do_main()) {
printf ("\n** worked!\n");
} else {
printf ("\n** failed!\n");
}
return 0;
}
