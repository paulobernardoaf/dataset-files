#include <r_bin.h>
#include "le_specs.h"
typedef struct r_bin_le_obj_s {
LE_image_header *header;
bool is_le; 
char *filename;
const char *type;
const char *cpu;
const char *os;
const char *arch;
ut32 headerOff; 
LE_object_entry *objtbl;
void *buf; 
} r_bin_le_obj_t;
r_bin_le_obj_t *r_bin_le_new_buf(RBuffer *buf);
void r_bin_le_free(r_bin_le_obj_t *bin);
RList *r_bin_le_get_entrypoints(r_bin_le_obj_t *bin);
RList *r_bin_le_get_sections(r_bin_le_obj_t *bin);
RList *r_bin_le_get_symbols(r_bin_le_obj_t *bin);
RList *r_bin_le_get_imports(r_bin_le_obj_t *bin);
RList *r_bin_le_get_libs(r_bin_le_obj_t *bin);
RList *r_bin_le_get_relocs(r_bin_le_obj_t *bin);
