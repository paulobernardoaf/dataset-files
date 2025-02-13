#include <stdint.h>
#if defined(__cplusplus)
extern "C" {
#endif
#define ASN1_JSON_NULL "null"
#define ASN1_JSON_EMPTY "{}"
#define ASN1_OID_LEN 64
#define ASN1_CLASS 0xC0 
#define ASN1_FORM 0x20 
#define ASN1_TAG 0x1F 
#define ASN1_LENLONG 0x80 
#define ASN1_LENSHORT 0x7F 
#define CLASS_UNIVERSAL 0x00 
#define CLASS_APPLICATION 0x40 
#define CLASS_CONTEXT 0x80 
#define CLASS_PRIVATE 0xC0 
#define FORM_PRIMITIVE 0x00 
#define FORM_CONSTRUCTED 0x20 
#define TAG_EOC 0x00 
#define TAG_BOOLEAN 0x01 
#define TAG_INTEGER 0x02 
#define TAG_BITSTRING 0x03 
#define TAG_OCTETSTRING 0x04 
#define TAG_NULL 0x05 
#define TAG_OID 0x06 
#define TAG_OBJDESCRIPTOR 0x07 
#define TAG_EXTERNAL 0x08 
#define TAG_REAL 0x09 
#define TAG_ENUMERATED 0x0A 
#define TAG_EMBEDDED_PDV 0x0B 
#define TAG_UTF8STRING 0x0C 
#define TAG_SEQUENCE 0x10 
#define TAG_SET 0x11 
#define TAG_NUMERICSTRING 0x12 
#define TAG_PRINTABLESTRING 0x13 
#define TAG_T61STRING 0x14 
#define TAG_VIDEOTEXSTRING 0x15 
#define TAG_IA5STRING 0x16 
#define TAG_UTCTIME 0x17 
#define TAG_GENERALIZEDTIME 0x18 
#define TAG_GRAPHICSTRING 0x19 
#define TAG_VISIBLESTRING 0x1A 
#define TAG_GENERALSTRING 0x1B 
#define TAG_UNIVERSALSTRING 0x1C 
#define TAG_BMPSTRING 0x1E 
typedef struct r_asn1_string_t {
ut32 length;
const char *string;
bool allocated;
} RASN1String;
typedef struct r_asn1_list_t {
ut32 length;
struct r_asn1_object_t **objects;
} ASN1List;
typedef struct r_asn1_bin_t {
ut32 length;
ut8 *binary;
} RASN1Binary;
typedef struct r_asn1_object_t {
ut8 klass; 
ut8 form; 
ut8 tag; 
const ut8 *sector; 
ut32 length; 
ut64 offset; 
ASN1List list; 
} RASN1Object;
R_API RASN1Object *r_asn1_create_object (const ut8 *buffer, ut32 length, const ut8 *start_pointer);
R_API RASN1Binary *r_asn1_create_binary (const ut8 *buffer, ut32 length);
R_API RASN1String *r_asn1_create_string (const char *string, bool allocated, ut32 length);
R_API RASN1String *r_asn1_stringify_bits (const ut8 *buffer, ut32 length);
R_API RASN1String *r_asn1_stringify_utctime (const ut8 *buffer, ut32 length);
R_API RASN1String *r_asn1_stringify_time (const ut8 *buffer, ut32 length);
R_API RASN1String *r_asn1_stringify_integer (const ut8 *buffer, ut32 length);
R_API RASN1String *r_asn1_stringify_string (const ut8 *buffer, ut32 length);
R_API RASN1String *r_asn1_stringify_bytes (const ut8 *buffer, ut32 length);
R_API RASN1String *r_asn1_stringify_boolean (const ut8 *buffer, ut32 length);
R_API RASN1String *r_asn1_stringify_oid (const ut8* buffer, ut32 length);
R_API void r_asn1_free_object (RASN1Object *object);
R_API char *r_asn1_to_string (RASN1Object *object, ut32 depth, RStrBuf *sb);
R_API void r_asn1_free_string (RASN1String *string);
R_API void r_asn1_free_binary (RASN1Binary *string);
R_API void asn1_setformat (int fmt);
#if defined(__cplusplus)
}
#endif
