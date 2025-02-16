#include "qnx_specs.h"
#include <r_util.h>
#include <r_lib.h>
#include <r_types.h>
#include <r_bin.h>
enum {
LMF_HEADER_REC = 0,
LMF_COMMENT_REC,
LMF_LOAD_REC,
LMF_FIXUP_REC,
LMF_8087_FIXUP_REC,
LMF_IMAGE_END_REC,
LMF_RESOURCE_REC,
LMF_RW_END_REC,
LMF_LINEAR_FIXUP_REC
};
R_PACKED (
typedef struct lmf_record {
ut8 rec_type;
ut8 reserved; 
ut16 data_nbytes; 
ut16 spare; 
}) lmf_record;
R_PACKED (
typedef struct lmf_data {
ut16 segment;
ut32 offset;
}) lmf_data;
R_PACKED (
typedef struct lmf_header {
ut16 version;
ut16 cflags;
ut16 cpu; 
ut16 fpu; 
ut16 code_index; 
ut16 stack_index; 
ut16 heap_index; 
ut16 argv_index; 
ut16 spare2[4]; 
ut32 code_offset; 
ut32 stack_nbytes; 
ut32 heap_nbytes; 
ut32 image_base; 
ut32 spare3[2];
}) lmf_header;
R_PACKED (
typedef struct lmf_eof {
ut8 spare[6];
}) lmf_eof;
enum {
RES_USAGE = 0
};
R_PACKED (
typedef struct lmf_resource {
ut16 res_type;
ut16 spare[3];
}) lmf_resource;
R_PACKED (
typedef struct lmf_rw_end {
ut16 verify;
ut32 signature;
}) lmf_rw_end;
R_PACKED (
typedef struct {
Sdb *kv;
lmf_header lmfh;
RList* fixups;
RList* sections;
lmf_rw_end rwend;
}) QnxObj;
