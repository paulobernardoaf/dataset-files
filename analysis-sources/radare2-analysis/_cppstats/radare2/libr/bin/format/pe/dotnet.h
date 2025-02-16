#include <r_types.h>
#include "pe_specs.h"
#pragma pack(push, 1)
#define fits_in_pe(pe, pointer, size) ((size_t) size <= pe->data_size && (uint8_t*) (pointer) >= pe->data && (uint8_t*) (pointer) <= pe->data + pe->data_size - size)
#define struct_fits_in_pe(pe, pointer, struct_type) fits_in_pe(pe, pointer, sizeof(struct_type))
typedef struct _CLI_HEADER {
DWORD Size; 
WORD MajorRuntimeVersion;
WORD MinorRuntimeVersion;
IMAGE_DATA_DIRECTORY MetaData;
DWORD Flags;
DWORD EntryPointToken;
IMAGE_DATA_DIRECTORY Resources;
IMAGE_DATA_DIRECTORY StrongNameSignature;
ULONGLONG CodeManagerTable;
IMAGE_DATA_DIRECTORY VTableFixups;
ULONGLONG ExportAddressTableJumps;
ULONGLONG ManagedNativeHeader;
} CLI_HEADER, *PCLI_HEADER;
#define NET_METADATA_MAGIC 0x424a5342
typedef struct _NET_METADATA {
DWORD Magic;
WORD MajorVersion;
WORD MinorVersion;
DWORD Reserved;
DWORD Length;
char Version[0];
} NET_METADATA, *PNET_METADATA;
#define DOTNET_STREAM_NAME_SIZE 32
typedef struct _STREAM_HEADER {
DWORD Offset;
DWORD Size;
char Name[0];
} STREAM_HEADER, *PSTREAM_HEADER;
typedef struct _TILDE_HEADER {
DWORD Reserved1;
BYTE MajorVersion;
BYTE MinorVersion;
BYTE HeapSizes;
BYTE Reserved2;
ULONGLONG Valid;
ULONGLONG Sorted;
} TILDE_HEADER, *PTILDE_HEADER;
#define BIT_MODULE 0x00
#define BIT_TYPEREF 0x01
#define BIT_TYPEDEF 0x02
#define BIT_FIELDPTR 0x03 
#define BIT_FIELD 0x04
#define BIT_METHODDEFPTR 0x05 
#define BIT_METHODDEF 0x06
#define BIT_PARAMPTR 0x07 
#define BIT_PARAM 0x08
#define BIT_INTERFACEIMPL 0x09
#define BIT_MEMBERREF 0x0A
#define BIT_CONSTANT 0x0B
#define BIT_CUSTOMATTRIBUTE 0x0C
#define BIT_FIELDMARSHAL 0x0D
#define BIT_DECLSECURITY 0x0E
#define BIT_CLASSLAYOUT 0x0F
#define BIT_FIELDLAYOUT 0x10
#define BIT_STANDALONESIG 0x11
#define BIT_EVENTMAP 0x12
#define BIT_EVENTPTR 0x13 
#define BIT_EVENT 0x14
#define BIT_PROPERTYMAP 0x15
#define BIT_PROPERTYPTR 0x16 
#define BIT_PROPERTY 0x17
#define BIT_METHODSEMANTICS 0x18
#define BIT_METHODIMPL 0x19
#define BIT_MODULEREF 0x1A
#define BIT_TYPESPEC 0x1B
#define BIT_IMPLMAP 0x1C
#define BIT_FIELDRVA 0x1D
#define BIT_ENCLOG 0x1E 
#define BIT_ENCMAP 0x1F 
#define BIT_ASSEMBLY 0x20
#define BIT_ASSEMBLYPROCESSOR 0x21
#define BIT_ASSEMBLYOS 0x22
#define BIT_ASSEMBLYREF 0x23
#define BIT_ASSEMBLYREFPROCESSOR 0x24
#define BIT_ASSEMBLYREFOS 0x25
#define BIT_FILE 0x26
#define BIT_EXPORTEDTYPE 0x27
#define BIT_MANIFESTRESOURCE 0x28
#define BIT_NESTEDCLASS 0x29
#define BIT_GENERICPARAM 0x2A
#define BIT_METHODSPEC 0x2B
#define BIT_GENERICPARAMCONSTRAINT 0x2C
#define ELEMENT_TYPE_STRING 0x0E
#define MAX_TYPELIB_SIZE 0xFF
typedef struct _MODULE_TABLE {
WORD Generation;
union {
WORD Name_Short;
DWORD Name_Long;
} Name;
union {
WORD Mvid_Short;
DWORD Mvid_Long;
} Mvid;
union {
WORD EncId_Short;
DWORD EncId_Long;
} EncId;
union {
WORD EncBaseId_Short;
DWORD EncBaseId_Long;
} EncBaseId;
} MODULE_TABLE, *PMODULE_TABLE;
typedef struct _ASSEMBLY_TABLE {
DWORD HashAlgId;
WORD MajorVersion;
WORD MinorVersion;
WORD BuildNumber;
WORD RevisionNumber;
DWORD Flags;
union {
WORD PublicKey_Short;
DWORD PublicKey_Long;
} PublicKey;
union {
WORD Name_Short;
DWORD Name_Long;
} Name;
} ASSEMBLY_TABLE, *PASSEMBLY_TABLE;
typedef struct _ASSEMBLYREF_TABLE {
WORD MajorVersion;
WORD MinorVersion;
WORD BuildNumber;
WORD RevisionNumber;
DWORD Flags;
union {
WORD PublicKeyOrToken_Short;
DWORD PublicKeyOrToken_Long;
} PublicKeyOrToken;
union {
WORD Name_Short;
DWORD Name_Long;
} Name;
} ASSEMBLYREF_TABLE, *PASSEMBLYREF_TABLE;
typedef struct _MANIFESTRESOURCE_TABLE {
DWORD Offset;
DWORD Flags;
union {
WORD Name_Short;
DWORD Name_Long;
} Name;
union {
WORD Implementation_Short;
DWORD Implementation_Long;
} Implementation;
} MANIFESTRESOURCE_TABLE, *PMANIFESTRESOURCE_TABLE;
typedef struct _MODULEREF_TABLE {
union {
WORD Name_Short;
DWORD Name_Long;
} Name;
} MODULEREF_TABLE, *PMODULEREF_TABLE;
typedef struct _CUSTOMATTRIBUTE_TABLE {
union {
WORD Parent_Short;
DWORD Parent_Long;
} Parent;
union {
WORD Type_Short;
DWORD Type_Long;
} Type;
union {
WORD Value_Short;
DWORD Value_Long;
} Value;
} CUSTOMATTRIBUTE_TABLE, *PCUSTOMATTRIBUTE_TABLE;
typedef struct _CONSTANT_TABLE {
WORD Type;
union {
WORD Parent_Short;
DWORD Parent_Long;
} Parent;
union {
WORD Value_Short;
DWORD Value_Long;
} Value;
} CONSTANT_TABLE, *PCONSTANT_TABLE;
typedef struct _STREAMS {
PSTREAM_HEADER guid;
PSTREAM_HEADER tilde;
PSTREAM_HEADER string;
PSTREAM_HEADER blob;
PSTREAM_HEADER us;
} STREAMS, *PSTREAMS;
typedef struct _BLOB_PARSE_RESULT {
uint8_t size; 
DWORD length; 
} BLOB_PARSE_RESULT, *PBLOB_PARSE_RESULT;
typedef struct _ROWS {
uint32_t module;
uint32_t moduleref;
uint32_t assemblyref;
uint32_t typeref;
uint32_t methoddef;
uint32_t memberref;
uint32_t typedef_;
uint32_t typespec;
uint32_t field;
uint32_t param;
uint32_t property;
uint32_t interfaceimpl;
uint32_t event;
uint32_t standalonesig;
uint32_t assembly;
uint32_t file;
uint32_t exportedtype;
uint32_t manifestresource;
uint32_t genericparam;
uint32_t genericparamconstraint;
uint32_t methodspec;
uint32_t assemblyrefprocessor;
} ROWS, *PROWS;
typedef struct _INDEX_SIZES {
uint8_t string;
uint8_t guid;
uint8_t blob;
uint8_t field;
uint8_t methoddef;
uint8_t memberref;
uint8_t param;
uint8_t event;
uint8_t typedef_;
uint8_t property;
uint8_t moduleref;
uint8_t assemblyrefprocessor;
uint8_t assemblyref;
uint8_t genericparam;
} INDEX_SIZES, *PINDEX_SIZES;
#pragma pack(pop)
