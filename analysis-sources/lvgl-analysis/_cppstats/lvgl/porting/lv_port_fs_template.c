#if 0
#include "lv_port_fs_template.h"
typedef struct {
uint32_t dummy1;
uint32_t dummy2;
}file_t;
typedef struct {
uint32_t dummy1;
uint32_t dummy2;
}dir_t;
static void fs_init(void);
static lv_fs_res_t fs_open (lv_fs_drv_t * drv, void * file_p, const char * path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close (lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t fs_read (lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
static lv_fs_res_t fs_seek (lv_fs_drv_t * drv, void * file_p, uint32_t pos);
static lv_fs_res_t fs_size (lv_fs_drv_t * drv, void * file_p, uint32_t * size_p);
static lv_fs_res_t fs_tell (lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);
static lv_fs_res_t fs_remove (lv_fs_drv_t * drv, const char *path);
static lv_fs_res_t fs_trunc (lv_fs_drv_t * drv, void * file_p);
static lv_fs_res_t fs_rename (lv_fs_drv_t * drv, const char * oldname, const char * newname);
static lv_fs_res_t fs_free (lv_fs_drv_t * drv, uint32_t * total_p, uint32_t * free_p);
static lv_fs_res_t fs_dir_open (lv_fs_drv_t * drv, void * rddir_p, const char *path);
static lv_fs_res_t fs_dir_read (lv_fs_drv_t * drv, void * rddir_p, char *fn);
static lv_fs_res_t fs_dir_close (lv_fs_drv_t * drv, void * rddir_p);
void lv_port_fs_init(void)
{
fs_init();
lv_fs_drv_t fs_drv;
lv_fs_drv_init(&fs_drv);
fs_drv.file_size = sizeof(file_t);
fs_drv.letter = 'P';
fs_drv.open_cb = fs_open;
fs_drv.close_cb = fs_close;
fs_drv.read_cb = fs_read;
fs_drv.write_cb = fs_write;
fs_drv.seek_cb = fs_seek;
fs_drv.tell_cb = fs_tell;
fs_drv.free_space_cb = fs_free;
fs_drv.size_cb = fs_size;
fs_drv.remove_cb = fs_remove;
fs_drv.rename_cb = fs_rename;
fs_drv.trunc_cb = fs_trunc;
fs_drv.rddir_size = sizeof(dir_t);
fs_drv.dir_close_cb = fs_dir_close;
fs_drv.dir_open_cb = fs_dir_open;
fs_drv.dir_read_cb = fs_dir_read;
lv_fs_drv_register(&fs_drv);
}
static void fs_init(void)
{
}
static lv_fs_res_t fs_open (lv_fs_drv_t * drv, void * file_p, const char * path, lv_fs_mode_t mode)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
if(mode == LV_FS_MODE_WR)
{
}
else if(mode == LV_FS_MODE_RD)
{
}
else if(mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
{
}
return res;
}
static lv_fs_res_t fs_close (lv_fs_drv_t * drv, void * file_p)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_read (lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_seek (lv_fs_drv_t * drv, void * file_p, uint32_t pos)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_size (lv_fs_drv_t * drv, void * file_p, uint32_t * size_p)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_tell (lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_remove (lv_fs_drv_t * drv, const char *path)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_trunc (lv_fs_drv_t * drv, void * file_p)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_rename (lv_fs_drv_t * drv, const char * oldname, const char * newname)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_free (lv_fs_drv_t * drv, uint32_t * total_p, uint32_t * free_p)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_dir_open (lv_fs_drv_t * drv, void * rddir_p, const char *path)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_dir_read (lv_fs_drv_t * drv, void * rddir_p, char *fn)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
static lv_fs_res_t fs_dir_close (lv_fs_drv_t * drv, void * rddir_p)
{
lv_fs_res_t res = LV_FS_RES_NOT_IMP;
return res;
}
#else 
typedef int keep_pedantic_happy;
#endif
