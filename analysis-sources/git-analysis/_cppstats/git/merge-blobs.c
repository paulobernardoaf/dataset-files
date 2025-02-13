#include "cache.h"
#include "run-command.h"
#include "xdiff-interface.h"
#include "ll-merge.h"
#include "blob.h"
#include "merge-blobs.h"
#include "object-store.h"
static int fill_mmfile_blob(mmfile_t *f, struct blob *obj)
{
void *buf;
unsigned long size;
enum object_type type;
buf = read_object_file(&obj->object.oid, &type, &size);
if (!buf)
return -1;
if (type != OBJ_BLOB) {
free(buf);
return -1;
}
f->ptr = buf;
f->size = size;
return 0;
}
static void free_mmfile(mmfile_t *f)
{
free(f->ptr);
}
static void *three_way_filemerge(struct index_state *istate,
const char *path,
mmfile_t *base,
mmfile_t *our,
mmfile_t *their,
unsigned long *size)
{
int merge_status;
mmbuffer_t res;
merge_status = ll_merge(&res, path, base, NULL,
our, ".our", their, ".their",
istate, NULL);
if (merge_status < 0)
return NULL;
*size = res.size;
return res.ptr;
}
void *merge_blobs(struct index_state *istate, const char *path,
struct blob *base, struct blob *our,
struct blob *their, unsigned long *size)
{
void *res = NULL;
mmfile_t f1, f2, common;
if (!our || !their) {
enum object_type type;
if (base)
return NULL;
if (!our)
our = their;
return read_object_file(&our->object.oid, &type, size);
}
if (fill_mmfile_blob(&f1, our) < 0)
goto out_no_mmfile;
if (fill_mmfile_blob(&f2, their) < 0)
goto out_free_f1;
if (base) {
if (fill_mmfile_blob(&common, base) < 0)
goto out_free_f2_f1;
} else {
common.ptr = xstrdup("");
common.size = 0;
}
res = three_way_filemerge(istate, path, &common, &f1, &f2, size);
free_mmfile(&common);
out_free_f2_f1:
free_mmfile(&f2);
out_free_f1:
free_mmfile(&f1);
out_no_mmfile:
return res;
}
