#if !defined(HEADER_CURL_GSSAPI_STUBS_H)
#define HEADER_CURL_GSSAPI_STUBS_H
























#include <stdint.h>
#include <stddef.h>

#define GSS_ERROR(status) (status & 0x80000000)

#define GSS_S_COMPLETE 0
#define GSS_S_FAILURE (0x80000000)
#define GSS_S_CONTINUE_NEEDED (1ul)

#define GSS_C_QOP_DEFAULT 0
#define GSS_C_NO_OID ((gss_OID) 0)
#define GSS_C_NO_NAME ((gss_name_t) 0)
#define GSS_C_NO_BUFFER ((gss_buffer_t) 0)
#define GSS_C_NO_CONTEXT ((gss_ctx_id_t) 0)
#define GSS_C_NO_CREDENTIAL ((gss_cred_id_t) 0)
#define GSS_C_NO_CHANNEL_BINDINGS ((gss_channel_bindings_t) 0)

#define GSS_C_NULL_OID GSS_C_NO_OID

#define GSS_C_EMPTY_BUFFER {0, NULL}

#define GSS_C_AF_INET 2

#define GSS_C_GSS_CODE 1
#define GSS_C_MECH_CODE 2

#define GSS_C_DELEG_FLAG 1
#define GSS_C_MUTUAL_FLAG 2
#define GSS_C_REPLAY_FLAG 4
#define GSS_C_CONF_FLAG 16
#define GSS_C_INTEG_FLAG 32





#define GSS_C_INDEFINITE 0xfffffffful

#define GSS_C_NT_HOSTBASED_SERVICE NULL

typedef uint32_t OM_uint32;

typedef OM_uint32 gss_qop_t;

typedef struct gss_buffer_desc_struct {
size_t length;
void *value;
} gss_buffer_desc, *gss_buffer_t;

struct gss_cred_id_t_desc_struct;
typedef struct gss_cred_id_t_desc_struct *gss_cred_id_t;
typedef const struct gss_cred_id_t_desc_struct *gss_const_cred_id_t;

struct gss_ctx_id_t_desc_struct;
typedef struct gss_ctx_id_t_desc_struct *gss_ctx_id_t;
typedef const struct gss_ctx_id_t_desc_struct *gss_const_ctx_id_t;

struct gss_name_t_desc_struct;
typedef struct gss_name_t_desc_struct *gss_name_t;
typedef const struct gss_name_t_desc_struct *gss_const_name_t;

typedef struct gss_OID_desc_struct {
OM_uint32 length;
void *elements;
} gss_OID_desc, *gss_OID;

typedef struct gss_channel_bindings_struct {
OM_uint32 initiator_addrtype;
gss_buffer_desc initiator_address;
OM_uint32 acceptor_addrtype;
gss_buffer_desc acceptor_address;
gss_buffer_desc application_data;
} *gss_channel_bindings_t;

OM_uint32 gss_release_buffer(OM_uint32 * ,
gss_buffer_t );

OM_uint32 gss_init_sec_context(OM_uint32 * ,
gss_const_cred_id_t ,
gss_ctx_id_t * ,
gss_const_name_t ,
const gss_OID ,
OM_uint32 ,
OM_uint32 ,
const gss_channel_bindings_t ,
const gss_buffer_t ,
gss_OID * ,
gss_buffer_t ,
OM_uint32 * ,
OM_uint32 * );

OM_uint32 gss_delete_sec_context(OM_uint32 * ,
gss_ctx_id_t * ,
gss_buffer_t );

OM_uint32 gss_inquire_context(OM_uint32 * ,
gss_const_ctx_id_t ,
gss_name_t * ,
gss_name_t * ,
OM_uint32 * ,
gss_OID * ,
OM_uint32 * ,
int * ,
int * );

OM_uint32 gss_wrap(OM_uint32 * ,
gss_const_ctx_id_t ,
int ,
gss_qop_t ,
const gss_buffer_t ,
int * ,
gss_buffer_t );

OM_uint32 gss_unwrap(OM_uint32 * ,
gss_const_ctx_id_t ,
const gss_buffer_t ,
gss_buffer_t ,
int * ,
gss_qop_t * );

OM_uint32 gss_seal(OM_uint32 * ,
gss_ctx_id_t ,
int ,
int ,
gss_buffer_t ,
int * ,
gss_buffer_t );

OM_uint32 gss_unseal(OM_uint32 * ,
gss_ctx_id_t ,
gss_buffer_t ,
gss_buffer_t ,
int * ,
int * );

OM_uint32 gss_import_name(OM_uint32 * ,
const gss_buffer_t ,
const gss_OID ,
gss_name_t * );

OM_uint32 gss_release_name(OM_uint32 * ,
gss_name_t * );

OM_uint32 gss_display_name(OM_uint32 * ,
gss_const_name_t ,
gss_buffer_t ,
gss_OID * );

OM_uint32 gss_display_status(OM_uint32 * ,
OM_uint32 ,
int ,
const gss_OID ,
OM_uint32 * ,
gss_buffer_t );

#endif 
