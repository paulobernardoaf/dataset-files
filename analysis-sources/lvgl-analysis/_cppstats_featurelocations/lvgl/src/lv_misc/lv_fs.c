







#include "lv_fs.h"
#if LV_USE_FILESYSTEM

#include "../lv_core/lv_debug.h"
#include "lv_ll.h"
#include <string.h>
#include "lv_gc.h"

#if defined(LV_GC_INCLUDE)
#include LV_GC_INCLUDE
#endif 









#if defined(free)
#undef free
#endif








static const char * lv_fs_get_real_path(const char * path);
















void lv_fs_init(void)
{
lv_ll_init(&LV_GC_ROOT(_lv_drv_ll), sizeof(lv_fs_drv_t));
}







bool lv_fs_is_ready(char letter)
{
lv_fs_drv_t * drv = lv_fs_get_drv(letter);

if(drv == NULL) return false; 

if(drv->ready_cb == NULL) return true; 

return drv->ready_cb(drv);
}








lv_fs_res_t lv_fs_open(lv_fs_file_t * file_p, const char * path, lv_fs_mode_t mode)
{
file_p->drv = NULL;
file_p->file_d = NULL;

if(path == NULL) return LV_FS_RES_INV_PARAM;

char letter = path[0];

file_p->drv = lv_fs_get_drv(letter);

if(file_p->drv == NULL) {
file_p->file_d = NULL;
return LV_FS_RES_NOT_EX;
}

if(file_p->drv->ready_cb != NULL) {
if(file_p->drv->ready_cb(file_p->drv) == false) {
file_p->drv = NULL;
file_p->file_d = NULL;
return LV_FS_RES_HW_ERR;
}
}

file_p->file_d = lv_mem_alloc(file_p->drv->file_size);
LV_ASSERT_MEM(file_p->file_d);
if(file_p->file_d == NULL) {
file_p->drv = NULL;
return LV_FS_RES_OUT_OF_MEM; 
}

if(file_p->drv->open_cb == NULL) {
return LV_FS_RES_NOT_IMP;
}

const char * real_path = lv_fs_get_real_path(path);
lv_fs_res_t res = file_p->drv->open_cb(file_p->drv, file_p->file_d, real_path, mode);

if(res != LV_FS_RES_OK) {
lv_mem_free(file_p->file_d);
file_p->file_d = NULL;
file_p->drv = NULL;
}

return res;
}






lv_fs_res_t lv_fs_close(lv_fs_file_t * file_p)
{
if(file_p->drv == NULL) {
return LV_FS_RES_INV_PARAM;
}

if(file_p->drv->close_cb == NULL) {
return LV_FS_RES_NOT_IMP;
}

lv_fs_res_t res = file_p->drv->close_cb(file_p->drv, file_p->file_d);

lv_mem_free(file_p->file_d); 
file_p->file_d = NULL;
file_p->drv = NULL;
file_p->file_d = NULL;

return res;
}






lv_fs_res_t lv_fs_remove(const char * path)
{
if(path == NULL) return LV_FS_RES_INV_PARAM;
lv_fs_drv_t * drv = NULL;

char letter = path[0];

drv = lv_fs_get_drv(letter);
if(drv == NULL) return LV_FS_RES_NOT_EX;
if(drv->ready_cb != NULL) {
if(drv->ready_cb(drv) == false) return LV_FS_RES_HW_ERR;
}

if(drv->remove_cb == NULL) return LV_FS_RES_NOT_IMP;

const char * real_path = lv_fs_get_real_path(path);
lv_fs_res_t res = drv->remove_cb(drv, real_path);

return res;
}









lv_fs_res_t lv_fs_read(lv_fs_file_t * file_p, void * buf, uint32_t btr, uint32_t * br)
{
if(br != NULL) *br = 0;
if(file_p->drv == NULL) return LV_FS_RES_INV_PARAM;
if(file_p->drv->read_cb == NULL) return LV_FS_RES_NOT_IMP;

uint32_t br_tmp = 0;
lv_fs_res_t res = file_p->drv->read_cb(file_p->drv, file_p->file_d, buf, btr, &br_tmp);
if(br != NULL) *br = br_tmp;

return res;
}









lv_fs_res_t lv_fs_write(lv_fs_file_t * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
if(bw != NULL) *bw = 0;

if(file_p->drv == NULL) {
return LV_FS_RES_INV_PARAM;
}

if(file_p->drv->write_cb == NULL) {
return LV_FS_RES_NOT_IMP;
}

uint32_t bw_tmp = 0;
lv_fs_res_t res = file_p->drv->write_cb(file_p->drv, file_p->file_d, buf, btw, &bw_tmp);
if(bw != NULL) *bw = bw_tmp;

return res;
}







lv_fs_res_t lv_fs_seek(lv_fs_file_t * file_p, uint32_t pos)
{
if(file_p->drv == NULL) {
return LV_FS_RES_INV_PARAM;
}

if(file_p->drv->seek_cb == NULL) {
return LV_FS_RES_NOT_IMP;
}

lv_fs_res_t res = file_p->drv->seek_cb(file_p->drv, file_p->file_d, pos);

return res;
}







lv_fs_res_t lv_fs_tell(lv_fs_file_t * file_p, uint32_t * pos)
{
if(file_p->drv == NULL) {
pos = 0;
return LV_FS_RES_INV_PARAM;
}

if(file_p->drv->tell_cb == NULL) {
pos = 0;
return LV_FS_RES_NOT_IMP;
}

lv_fs_res_t res = file_p->drv->tell_cb(file_p->drv, file_p->file_d, pos);

return res;
}







lv_fs_res_t lv_fs_trunc(lv_fs_file_t * file_p)
{
if(file_p->drv == NULL) {
return LV_FS_RES_INV_PARAM;
}

if(file_p->drv->tell_cb == NULL) {
return LV_FS_RES_NOT_IMP;
}

lv_fs_res_t res = file_p->drv->trunc_cb(file_p->drv, file_p->file_d);

return res;
}






lv_fs_res_t lv_fs_size(lv_fs_file_t * file_p, uint32_t * size)
{
if(file_p->drv == NULL) {
return LV_FS_RES_INV_PARAM;
}

if(file_p->drv->size_cb == NULL) return LV_FS_RES_NOT_IMP;

if(size == NULL) return LV_FS_RES_INV_PARAM;

lv_fs_res_t res = file_p->drv->size_cb(file_p->drv, file_p->file_d, size);

return res;
}







lv_fs_res_t lv_fs_rename(const char * oldname, const char * newname)
{
if(!oldname || !newname) return LV_FS_RES_INV_PARAM;

char letter = oldname[0];

lv_fs_drv_t * drv = lv_fs_get_drv(letter);

if(!drv) {
return LV_FS_RES_NOT_EX;
}

if(drv->ready_cb != NULL) {
if(drv->ready_cb(drv) == false) {
return LV_FS_RES_HW_ERR;
}
}

if(drv->rename_cb == NULL) return LV_FS_RES_NOT_IMP;

const char * old_real = lv_fs_get_real_path(oldname);
const char * new_real = lv_fs_get_real_path(newname);

lv_fs_res_t res = drv->rename_cb(drv, old_real, new_real);

return res;
}







lv_fs_res_t lv_fs_dir_open(lv_fs_dir_t * rddir_p, const char * path)
{
if(path == NULL) return LV_FS_RES_INV_PARAM;

char letter = path[0];

rddir_p->drv = lv_fs_get_drv(letter);

if(rddir_p->drv == NULL) {
rddir_p->dir_d = NULL;
return LV_FS_RES_NOT_EX;
}

rddir_p->dir_d = lv_mem_alloc(rddir_p->drv->rddir_size);
LV_ASSERT_MEM(rddir_p->dir_d);
if(rddir_p->dir_d == NULL) {
rddir_p->dir_d = NULL;
return LV_FS_RES_OUT_OF_MEM; 
}

if(rddir_p->drv->dir_open_cb == NULL) {
return LV_FS_RES_NOT_IMP;
}

const char * real_path = lv_fs_get_real_path(path);

lv_fs_res_t res = rddir_p->drv->dir_open_cb(rddir_p->drv, rddir_p->dir_d, real_path);

return res;
}








lv_fs_res_t lv_fs_dir_read(lv_fs_dir_t * rddir_p, char * fn)
{
if(rddir_p->drv == NULL || rddir_p->dir_d == NULL) {
fn[0] = '\0';
return LV_FS_RES_INV_PARAM;
}

if(rddir_p->drv->dir_read_cb == NULL) {
return LV_FS_RES_NOT_IMP;
}

lv_fs_res_t res = rddir_p->drv->dir_read_cb(rddir_p->drv, rddir_p->dir_d, fn);

return res;
}






lv_fs_res_t lv_fs_dir_close(lv_fs_dir_t * rddir_p)
{
if(rddir_p->drv == NULL || rddir_p->dir_d == NULL) {
return LV_FS_RES_INV_PARAM;
}

lv_fs_res_t res;

if(rddir_p->drv->dir_close_cb == NULL) {
res = LV_FS_RES_NOT_IMP;
} else {
res = rddir_p->drv->dir_close_cb(rddir_p->drv, rddir_p->dir_d);
}

lv_mem_free(rddir_p->dir_d); 
rddir_p->dir_d = NULL;
rddir_p->drv = NULL;
rddir_p->dir_d = NULL;

return res;
}








lv_fs_res_t lv_fs_free_space(char letter, uint32_t * total_p, uint32_t * free_p)
{
lv_fs_drv_t * drv = lv_fs_get_drv(letter);

if(drv == NULL) {
return LV_FS_RES_INV_PARAM;
}

lv_fs_res_t res;

if(drv->free_space_cb == NULL) {
res = LV_FS_RES_NOT_IMP;
} else {
uint32_t total_tmp = 0;
uint32_t free_tmp = 0;
res = drv->free_space_cb(drv, &total_tmp, &free_tmp);

if(total_p != NULL) *total_p = total_tmp;
if(free_p != NULL) *free_p = free_tmp;
}

return res;
}







void lv_fs_drv_init(lv_fs_drv_t * drv)
{
memset(drv, 0, sizeof(lv_fs_drv_t));
}






void lv_fs_drv_register(lv_fs_drv_t * drv_p)
{

lv_fs_drv_t * new_drv;
new_drv = lv_ll_ins_head(&LV_GC_ROOT(_lv_drv_ll));
LV_ASSERT_MEM(new_drv);
if(new_drv == NULL) return;

memcpy(new_drv, drv_p, sizeof(lv_fs_drv_t));
}






lv_fs_drv_t * lv_fs_get_drv(char letter)
{
lv_fs_drv_t * drv;

LV_LL_READ(LV_GC_ROOT(_lv_drv_ll), drv)
{
if(drv->letter == letter) {
return drv;
}
}

return NULL;
}





char * lv_fs_get_letters(char * buf)
{
lv_fs_drv_t * drv;
uint8_t i = 0;

LV_LL_READ(LV_GC_ROOT(_lv_drv_ll), drv)
{
buf[i] = drv->letter;
i++;
}

buf[i] = '\0';

return buf;
}






const char * lv_fs_get_ext(const char * fn)
{
size_t i;
for(i = strlen(fn); i > 0; i--) {
if(fn[i] == '.') {
return &fn[i + 1];
} else if(fn[i] == '/' || fn[i] == '\\') {
return ""; 
}
}

return ""; 
}






char * lv_fs_up(char * path)
{
size_t len = strlen(path);
if(len == 0) return path;

len--; 


while(path[len] == '/' || path[len] == '\\') {
path[len] = '\0';
if(len > 0)
len--;
else
return path;
}

size_t i;
for(i = len; i > 0; i--) {
if(path[i] == '/' || path[i] == '\\') break;
}

if(i > 0) path[i] = '\0';

return path;
}






const char * lv_fs_get_last(const char * path)
{
size_t len = strlen(path);
if(len == 0) return path;

len--; 


while(path[len] == '/' || path[len] == '\\') {
if(len > 0)
len--;
else
return path;
}

size_t i;
for(i = len; i > 0; i--) {
if(path[i] == '/' || path[i] == '\\') break;
}


if(i == 0) return path;

return &path[i + 1];
}









static const char * lv_fs_get_real_path(const char * path)
{



path++; 

while(*path != '\0') {
if(*path == ':' || *path == '\\' || *path == '/') {
path++;
} else {
break;
}
}

return path;
}

#endif 
