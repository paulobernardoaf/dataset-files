#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#define ULONGLONG ut64
#define DWORD uint32_t
#define WORD uint16_t
#define BYTE uint8_t
typedef struct _IMAGE_DATA_DIRECTORY {
DWORD VirtualAddress;
DWORD Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;
#include "r_types.h"
#include "pe_specs.h"
#include "dotnet.h"
static void set_string(const char *k, const char *f, const char *v, ...) {
eprintf ("-> %s = %s\n", k, v);
}
static void set_integer(int a, void *b, const char *name, ...) {
eprintf ("-> %s = %d\n", name, a);
}
static void set_sized_string(char *a, int len, void *p, const char *fmt, ...) {
eprintf ("-> %s = %d (%s)\n", fmt, len, (char *)a);
}
static ut64 pe_rva_to_offset(void *pe, ut64 addr) {
return addr;
}
typedef struct _PE {
const uint8_t* data;
size_t data_size;
void* object;
} PE;
PIMAGE_DATA_DIRECTORY pe_get_directory_entry( PE* pe, int entry) {
#if 0
PIMAGE_DATA_DIRECTORY result = IS_64BITS_PE(pe)
? &pe->header64->OptionalHeader.DataDirectory[entry]
: &pe->header->OptionalHeader.DataDirectory[entry];
#else
PIMAGE_DATA_DIRECTORY result = { 0 };
#endif
return result;
}
char* pe_get_dotnet_string( PE* pe, const uint8_t* string_offset, DWORD string_index) {
if (!(string_offset + string_index >= pe->data &&
string_offset + string_index < pe->data + pe->data_size)) {
return NULL;
}
size_t remaining = (pe->data + pe->data_size) - (string_offset + string_index);
char *start = (char*) (string_offset + string_index);
char *eos = (char*) memmem((void*) start, remaining, "\0", 1);
return eos? start: NULL;
}
uint32_t max_rows(int count, ...) {
va_list ap;
int i;
uint32_t biggest;
uint32_t x;
if (count == 0)
return 0;
va_start(ap, count);
biggest = va_arg(ap, uint32_t);
for (i = 1; i < count; i++) {
x = va_arg(ap, uint32_t);
biggest = (x > biggest) ? x : biggest;
}
va_end(ap);
return biggest;
}
void dotnet_parse_guid( PE* pe, ut64 metadata_root, PSTREAM_HEADER guid_header) {
char guid[37];
int i = 0;
const uint8_t* guid_offset = pe->data + metadata_root + guid_header->Offset;
DWORD guid_size = guid_header->Size;
while (guid_size >= 16 && fits_in_pe(pe, guid_offset, 16)) {
sprintf(guid, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
*(uint32_t*) guid_offset,
*(uint16_t*) (guid_offset + 4),
*(uint16_t*) (guid_offset + 6),
*(guid_offset + 8),
*(guid_offset + 9),
*(guid_offset + 10),
*(guid_offset + 11),
*(guid_offset + 12),
*(guid_offset + 13),
*(guid_offset + 14),
*(guid_offset + 15));
guid[(16 * 2) + 4] = '\0';
set_string (guid, pe->object, "guids[%i]", i);
i++;
guid_size -= 16;
}
set_integer(i, pe->object, "number_of_guids");
}
BLOB_PARSE_RESULT dotnet_parse_blob_entry( PE* pe, const uint8_t* offset) {
BLOB_PARSE_RESULT result;
if (!fits_in_pe(pe, offset, 1)) {
result.size = 0;
return result;
}
if ((*offset & 0x80) == 0x00) {
result.length = (DWORD) *offset;
result.size = 1;
} else if ((*offset & 0xC0) == 0x80) {
if (!fits_in_pe(pe, offset, 2)) {
result.size = 0;
return result;
}
result.length = ((*offset & 0x3F) << 8) | *(offset + 1);
result.size = 2;
} else if (offset + 4 < pe->data + pe->data_size && (*offset & 0xE0) == 0xC0) {
if (!fits_in_pe(pe, offset, 4)) {
result.size = 0;
return result;
}
result.length = ((*offset & 0x1F) << 24) |
(*(offset + 1) << 16) |
(*(offset + 2) << 8) |
*(offset + 3);
result.size = 4;
} else {
result.size = 0;
}
return result;
}
void dotnet_parse_us( PE* pe, ut64 metadata_root, PSTREAM_HEADER us_header) {
BLOB_PARSE_RESULT blob_result;
int i = 0;
const uint8_t* offset = pe->data + metadata_root + us_header->Offset;
const uint8_t* end_of_header = offset + us_header->Size;
if (!fits_in_pe(pe, offset, us_header->Size) || *offset != 0x00) {
return;
}
offset++;
while (offset < end_of_header) {
blob_result = dotnet_parse_blob_entry(pe, offset);
if (blob_result.size == 0 || !fits_in_pe(pe, offset, blob_result.length)) {
set_integer(i, pe->object, "number_of_user_strings");
return;
}
offset += blob_result.size;
if (blob_result.length > 0) {
set_sized_string(
(char*) offset,
blob_result.length,
pe->object,
"user_strings[%i]",
i);
offset += blob_result.length;
i++;
}
}
set_integer(i, pe->object, "number_of_user_strings");
}
STREAMS dotnet_parse_stream_headers(
PE* pe,
ut64 offset,
ut64 metadata_root,
DWORD num_streams)
{
PSTREAM_HEADER stream_header;
STREAMS headers;
char *start;
char *eos;
char stream_name[DOTNET_STREAM_NAME_SIZE + 1];
unsigned int i;
memset(&headers, '\0', sizeof(STREAMS));
stream_header = (PSTREAM_HEADER) (pe->data + offset);
for (i = 0; i < num_streams; i++)
{
if (!struct_fits_in_pe(pe, stream_header, STREAM_HEADER))
break;
start = (char*) stream_header->Name;
if (!fits_in_pe(pe, start, DOTNET_STREAM_NAME_SIZE))
break;
eos = (char*) memmem((void*) start, DOTNET_STREAM_NAME_SIZE, "\0", 1);
if (eos == NULL)
break;
strncpy(stream_name, stream_header->Name, DOTNET_STREAM_NAME_SIZE);
stream_name[DOTNET_STREAM_NAME_SIZE] = '\0';
set_string(stream_name,
pe->object, "streams[%i].name", i);
set_integer(metadata_root + stream_header->Offset,
pe->object, "streams[%i].offset", i);
set_integer(stream_header->Size,
pe->object, "streams[%i].size", i);
if (strncmp(stream_name, "#GUID", 5) == 0)
headers.guid = stream_header;
else if (strncmp(stream_name, "#~", 2) == 0 && headers.tilde == NULL)
headers.tilde = stream_header;
else if (strncmp(stream_name, "#Strings", 8) == 0 && headers.string == NULL)
headers.string = stream_header;
else if (strncmp(stream_name, "#Blob", 5) == 0)
headers.blob = stream_header;
else if (strncmp(stream_name, "#US", 3) == 0 && headers.us == NULL)
headers.us = stream_header;
stream_header = (PSTREAM_HEADER) ((uint8_t*) stream_header +
sizeof(STREAM_HEADER) +
strlen(stream_name) +
4 - (strlen(stream_name) % 4));
}
set_integer(i, pe->object, "number_of_streams");
return headers;
}
void dotnet_parse_tilde_2(
PE* pe,
PTILDE_HEADER tilde_header,
ut64 resource_base,
ut64 metadata_root,
ROWS rows,
INDEX_SIZES index_sizes,
PSTREAMS streams)
{
PMODULE_TABLE module_table;
PASSEMBLY_TABLE assembly_table;
PASSEMBLYREF_TABLE assemblyref_table;
PMANIFESTRESOURCE_TABLE manifestresource_table;
PMODULEREF_TABLE moduleref_table;
PCUSTOMATTRIBUTE_TABLE customattribute_table;
PCONSTANT_TABLE constant_table;
DWORD resource_size, implementation;
char *name;
char typelib[MAX_TYPELIB_SIZE + 1];
unsigned int i;
int bit_check;
int matched_bits = 0;
ut64 resource_offset;
uint32_t row_size, row_count, counter;
const uint8_t* string_offset;
const uint8_t* blob_offset;
uint32_t num_rows = 0;
uint32_t valid_rows = 0;
uint32_t* row_offset = NULL;
uint8_t* table_offset = NULL;
uint8_t* row_ptr = NULL;
uint8_t* typeref_ptr = NULL;
uint8_t* memberref_ptr = NULL;
uint32_t typeref_row_size = 0;
uint32_t memberref_row_size = 0;
uint8_t* typeref_row = NULL;
uint8_t* memberref_row = NULL;
DWORD type_index;
DWORD class_index;
BLOB_PARSE_RESULT blob_result;
DWORD blob_index;
DWORD blob_length;
uint8_t index_size, index_size2;
for (i = 0; i < 64; i++) {
valid_rows += ((tilde_header->Valid >> i) & 0x01);
}
row_offset = (uint32_t*) (tilde_header + 1);
table_offset = (uint8_t*) row_offset;
table_offset += sizeof(uint32_t) * valid_rows;
#define DOTNET_STRING_INDEX(Name) index_sizes.string == 2 ? Name.Name_Short : Name.Name_Long
string_offset = pe->data + metadata_root + streams->string->Offset;
for (bit_check = 0; bit_check < 64; bit_check++)
{
if (!((tilde_header->Valid >> bit_check) & 0x01))
continue;
if (!fits_in_pe (pe, table_offset, 1))
return;
num_rows = *(row_offset + matched_bits);
switch (bit_check)
{
case BIT_MODULE:
module_table = (PMODULE_TABLE) table_offset;
name = pe_get_dotnet_string(pe,
string_offset,
DOTNET_STRING_INDEX(module_table->Name));
if (name != NULL)
set_string(name, pe->object, "module_name");
table_offset += (
2 + index_sizes.string + (index_sizes.guid * 3)) * num_rows;
break;
case BIT_TYPEREF:
row_count = max_rows(4,
rows.module,
rows.moduleref,
rows.assemblyref,
rows.typeref);
if (row_count > (0xFFFF >> 0x02))
index_size = 4;
else
index_size = 2;
row_size = (index_size + (index_sizes.string * 2));
typeref_row_size = row_size;
typeref_ptr = table_offset;
table_offset += row_size * num_rows;
break;
case BIT_TYPEDEF:
row_count = max_rows(3,
rows.typedef_,
rows.typeref,
rows.typespec);
if (row_count > (0xFFFF >> 0x02))
index_size = 4;
else
index_size = 2;
table_offset += (
4 + (index_sizes.string * 2) + index_size +
index_sizes.field + index_sizes.methoddef) * num_rows;
break;
case BIT_FIELDPTR:
table_offset += (index_sizes.field) * num_rows;
break;
case BIT_FIELD:
table_offset += (
2 + (index_sizes.string) + index_sizes.blob) * num_rows;
break;
case BIT_METHODDEFPTR:
table_offset += (index_sizes.methoddef) * num_rows;
break;
case BIT_METHODDEF:
table_offset += (
4 + 2 + 2 +
index_sizes.string +
index_sizes.blob +
index_sizes.param) * num_rows;
break;
case BIT_PARAM:
table_offset += (2 + 2 + index_sizes.string) * num_rows;
break;
case BIT_INTERFACEIMPL:
row_count = max_rows(3,
rows.typedef_,
rows.typeref,
rows.typespec);
if (row_count > (0xFFFF >> 0x02))
index_size = 4;
else
index_size = 2;
table_offset += (index_sizes.typedef_ + index_size) * num_rows;
break;
case BIT_MEMBERREF:
row_count = max_rows(4,
rows.methoddef,
rows.moduleref,
rows.typeref,
rows.typespec);
if (row_count > (0xFFFF >> 0x03))
index_size = 4;
else
index_size = 2;
row_size = (index_size + index_sizes.string + index_sizes.blob);
memberref_row_size = row_size;
memberref_ptr = table_offset;
table_offset += row_size * num_rows;
break;
case BIT_CONSTANT:
row_count = max_rows(3, rows.param, rows.field, rows.property);
if (row_count > (0xFFFF >> 0x02))
index_size = 4;
else
index_size = 2;
counter = 0;
row_size = (1 + 1 + index_size + index_sizes.blob);
row_ptr = table_offset;
for (i = 0; i < num_rows; i++)
{
if (!fits_in_pe(pe, row_ptr, row_size))
break;
constant_table = (PCONSTANT_TABLE) row_ptr;
if (constant_table->Type != ELEMENT_TYPE_STRING)
{
row_ptr += row_size;
continue;
}
blob_offset = ((uint8_t*) constant_table) + 2 + index_size;
if (index_sizes.blob == 4)
blob_index = *(DWORD*) blob_offset;
else
blob_index = (DWORD) (*(WORD*) blob_offset);
blob_offset = \
pe->data + metadata_root +
streams->blob->Offset + blob_index;
blob_result = dotnet_parse_blob_entry(pe, blob_offset);
if (blob_result.size == 0)
{
row_ptr += row_size;
continue;
}
blob_length = blob_result.length;
blob_offset += blob_result.size;
if (blob_offset + blob_length >= pe->data + pe->data_size)
{
row_ptr += row_size;
continue;
}
set_sized_string(
(char*) blob_offset,
blob_result.length,
pe->object,
"constants[%i]",
counter);
counter++;
row_ptr += row_size;
}
set_integer(counter, pe->object, "number_of_constants");
table_offset += row_size * num_rows;
break;
case BIT_CUSTOMATTRIBUTE:
row_count = max_rows(21,
rows.methoddef,
rows.field,
rows.typeref,
rows.typedef_,
rows.param,
rows.interfaceimpl,
rows.memberref,
rows.module,
rows.property,
rows.event,
rows.standalonesig,
rows.moduleref,
rows.typespec,
rows.assembly,
rows.assemblyref,
rows.file,
rows.exportedtype,
rows.manifestresource,
rows.genericparam,
rows.genericparamconstraint,
rows.methodspec);
if (row_count > (0xFFFF >> 0x05))
index_size = 4;
else
index_size = 2;
row_count = max_rows(2,
rows.methoddef,
rows.memberref);
if (row_count > (0xFFFF >> 0x03))
index_size2 = 4;
else
index_size2 = 2;
row_size = (index_size + index_size2 + index_sizes.blob);
if (typeref_ptr != NULL && memberref_ptr != NULL)
{
row_ptr = table_offset;
for (i = 0; i < num_rows; i++)
{
if (!fits_in_pe(pe, row_ptr, row_size))
break;
customattribute_table = (PCUSTOMATTRIBUTE_TABLE) row_ptr;
if (index_size == 4)
{
if ((*(DWORD*) customattribute_table & 0x1F) != 0x0E)
{
row_ptr += row_size;
continue;
}
}
else
{
if ((*(WORD*) customattribute_table & 0x1F) != 0x0E)
{
row_ptr += row_size;
continue;
}
}
customattribute_table = (PCUSTOMATTRIBUTE_TABLE) \
(row_ptr + index_size);
if (index_size2 == 4)
{
if ((*(DWORD*) customattribute_table & 0x07) != 0x03)
{
row_ptr += row_size;
continue;
}
type_index = *(DWORD*) customattribute_table >> 3;
}
else
{
if ((*(WORD*) customattribute_table & 0x07) != 0x03)
{
row_ptr += row_size;
continue;
}
type_index = (DWORD) ((*(WORD*) customattribute_table >> 3));
}
if (type_index > 0)
type_index--;
memberref_row = memberref_ptr + (memberref_row_size * type_index);
if (index_sizes.memberref == 4)
{
if ((*(DWORD*) memberref_row & 0x07) != 0x01)
{
row_ptr += row_size;
continue;
}
class_index = *(DWORD*) memberref_row >> 3;
}
else
{
if ((*(WORD*) memberref_row & 0x07) != 0x01)
{
row_ptr += row_size;
continue;
}
class_index = (DWORD) (*(WORD*) memberref_row >> 3);
}
if (class_index > 0)
class_index--;
typeref_row = typeref_ptr + (typeref_row_size * class_index);
row_count = max_rows(4,
rows.module,
rows.moduleref,
rows.assemblyref,
rows.typeref);
if (row_count > (0xFFFF >> 0x02))
typeref_row += 4;
else
typeref_row += 2;
if (index_sizes.string == 4)
{
name = pe_get_dotnet_string(
pe, string_offset, *(DWORD*) typeref_row);
}
else
{
name = pe_get_dotnet_string(
pe, string_offset, *(WORD*) typeref_row);
}
if (name != NULL && strncmp(name, "GuidAttribute", 13) != 0)
{
row_ptr += row_size;
continue;
}
customattribute_table = (PCUSTOMATTRIBUTE_TABLE) \
(row_ptr + index_size + index_size2);
if (index_sizes.blob == 4)
blob_index = *(DWORD*) customattribute_table;
else
blob_index = (DWORD) (*(WORD*) customattribute_table);
blob_offset = \
pe->data + metadata_root + streams->blob->Offset + blob_index;
if (blob_index == 0x00 || blob_offset >= pe->data + pe->data_size)
{
row_ptr += row_size;
continue;
}
blob_result = dotnet_parse_blob_entry(pe, blob_offset);
if (blob_result.size == 0)
{
row_ptr += row_size;
continue;
}
blob_length = blob_result.length;
blob_offset += blob_result.size;
if (blob_offset + blob_length >= pe->data + pe->data_size)
{
row_ptr += row_size;
continue;
}
if (*(WORD*) blob_offset != 0x0001)
{
row_ptr += row_size;
continue;
}
blob_offset += 2;
if (blob_offset + *blob_offset >= pe->data + pe->data_size)
{
row_ptr += row_size;
continue;
}
blob_offset += 1;
if (*blob_offset == 0xFF || *blob_offset == 0x00)
{
typelib[0] = '\0';
}
else
{
strncpy(typelib, (char*) blob_offset, MAX_TYPELIB_SIZE);
typelib[MAX_TYPELIB_SIZE] = '\0';
}
set_string(typelib, pe->object, "typelib");
row_ptr += row_size;
}
}
table_offset += row_size * num_rows;
break;
case BIT_FIELDMARSHAL:
row_count = max_rows(2,
rows.field,
rows.param);
if (row_count > (0xFFFF >> 0x01))
index_size = 4;
else
index_size = 2;
table_offset += (index_size + index_sizes.blob) * num_rows;
break;
case BIT_DECLSECURITY:
row_count = max_rows(3,
rows.typedef_,
rows.methoddef,
rows.assembly);
if (row_count > (0xFFFF >> 0x02))
index_size = 4;
else
index_size = 2;
table_offset += (2 + index_size + index_sizes.blob) * num_rows;
break;
case BIT_CLASSLAYOUT:
table_offset += (2 + 4 + index_sizes.typedef_) * num_rows;
break;
case BIT_FIELDLAYOUT:
table_offset += (4 + index_sizes.field) * num_rows;
break;
case BIT_STANDALONESIG:
table_offset += (index_sizes.blob) * num_rows;
break;
case BIT_EVENTMAP:
table_offset += (index_sizes.typedef_ + index_sizes.event) * num_rows;
break;
case BIT_EVENTPTR:
table_offset += (index_sizes.event) * num_rows;
break;
case BIT_EVENT:
row_count = max_rows(3,
rows.typedef_,
rows.typeref,
rows.typespec);
if (row_count > (0xFFFF >> 0x02))
index_size = 4;
else
index_size = 2;
table_offset += (2 + index_sizes.string + index_size) * num_rows;
break;
case BIT_PROPERTYMAP:
table_offset += (index_sizes.typedef_ + index_sizes.property) * num_rows;
break;
case BIT_PROPERTYPTR:
table_offset += (index_sizes.property) * num_rows;
break;
case BIT_PROPERTY:
table_offset += (2 + index_sizes.string + index_sizes.blob) * num_rows;
break;
case BIT_METHODSEMANTICS:
row_count = max_rows(2,
rows.event,
rows.property);
if (row_count > (0xFFFF >> 0x01))
index_size = 4;
else
index_size = 2;
table_offset += (2 + index_sizes.methoddef + index_size) * num_rows;
break;
case BIT_METHODIMPL:
row_count = max_rows(2,
rows.methoddef,
rows.memberref);
if (row_count > (0xFFFF >> 0x01))
index_size = 4;
else
index_size = 2;
table_offset += (index_sizes.typedef_ + (index_size * 2)) * num_rows;
break;
case BIT_MODULEREF:
row_ptr = table_offset;
counter = 0;
for (i = 0; i < num_rows; i++)
{
moduleref_table = (PMODULEREF_TABLE) row_ptr;
name = pe_get_dotnet_string(pe,
string_offset,
DOTNET_STRING_INDEX(moduleref_table->Name));
if (name != NULL)
{
set_string(name, pe->object, "modulerefs[%i]", counter);
counter++;
}
row_ptr += index_sizes.string;
}
set_integer(counter, pe->object, "number_of_modulerefs");
table_offset += (index_sizes.string) * num_rows;
break;
case BIT_TYPESPEC:
table_offset += (index_sizes.blob) * num_rows;
break;
case BIT_IMPLMAP:
row_count = max_rows(2,
rows.field,
rows.methoddef);
if (row_count > (0xFFFF >> 0x01))
index_size = 4;
else
index_size = 2;
table_offset += (
2 + index_size + index_sizes.string +
index_sizes.moduleref) * num_rows;
break;
case BIT_FIELDRVA:
table_offset += (4 + index_sizes.field) * num_rows;
break;
case BIT_ENCLOG:
table_offset += (4 + 4) * num_rows;
break;
case BIT_ENCMAP:
table_offset += (4) * num_rows;
break;
case BIT_ASSEMBLY:
row_size = (
4 + 2 + 2 + 2 + 2 + 4 + index_sizes.blob +
(index_sizes.string * 2));
if (!fits_in_pe(pe, table_offset, row_size))
break;
row_ptr = table_offset;
assembly_table = (PASSEMBLY_TABLE) table_offset;
set_integer(assembly_table->MajorVersion,
pe->object, "assembly.version.major");
set_integer(assembly_table->MinorVersion,
pe->object, "assembly.version.minor");
set_integer(assembly_table->BuildNumber,
pe->object, "assembly.version.build_number");
set_integer(assembly_table->RevisionNumber,
pe->object, "assembly.version.revision_number");
if (index_sizes.string == 4)
name = pe_get_dotnet_string(
pe,
string_offset,
*(DWORD*) (
row_ptr + 4 + 2 + 2 + 2 + 2 + 4 +
index_sizes.blob));
else
name = pe_get_dotnet_string(
pe,
string_offset,
*(WORD*) (
row_ptr + 4 + 2 + 2 + 2 + 2 + 4 +
index_sizes.blob));
if (name != NULL)
set_string(name, pe->object, "assembly.name");
if (index_sizes.string == 4)
{
name = pe_get_dotnet_string(
pe,
string_offset,
*(DWORD*) (
row_ptr + 4 + 2 + 2 + 2 + 2 + 4 +
index_sizes.blob +
index_sizes.string));
}
else
{
name = pe_get_dotnet_string(
pe,
string_offset,
*(WORD*) (
row_ptr + 4 + 2 + 2 + 2 + 2 + 4 +
index_sizes.blob +
index_sizes.string));
}
if (name != NULL && strlen(name) > 0)
set_string(name, pe->object, "assembly.culture");
table_offset += row_size * num_rows;
break;
case BIT_ASSEMBLYPROCESSOR:
table_offset += (4) * num_rows;
break;
case BIT_ASSEMBLYOS:
table_offset += (4 + 4 + 4) * num_rows;
break;
case BIT_ASSEMBLYREF:
row_size = (2 + 2 + 2 + 2 + 4 + (index_sizes.blob * 2) + (index_sizes.string * 2));
row_ptr = table_offset;
for (i = 0; i < num_rows; i++)
{
if (!fits_in_pe(pe, table_offset, row_size))
break;
assemblyref_table = (PASSEMBLYREF_TABLE) row_ptr;
set_integer(assemblyref_table->MajorVersion,
pe->object, "assembly_refs[%i].version.major", i);
set_integer(assemblyref_table->MinorVersion,
pe->object, "assembly_refs[%i].version.minor", i);
set_integer(assemblyref_table->BuildNumber,
pe->object, "assembly_refs[%i].version.build_number", i);
set_integer(assemblyref_table->RevisionNumber,
pe->object, "assembly_refs[%i].version.revision_number", i);
blob_offset = pe->data + metadata_root + streams->blob->Offset;
if (index_sizes.blob == 4)
blob_offset += \
assemblyref_table->PublicKeyOrToken.PublicKeyOrToken_Long;
else
blob_offset += \
assemblyref_table->PublicKeyOrToken.PublicKeyOrToken_Short;
blob_result = dotnet_parse_blob_entry(pe, blob_offset);
if (blob_result.size == 0 ||
!fits_in_pe(pe, blob_offset, blob_result.length))
{
row_ptr += row_size;
continue;
}
if (blob_result.length > 0)
{
blob_offset += blob_result.size;
set_sized_string((char*) blob_offset,
blob_result.length, pe->object,
"assembly_refs[%i].public_key_or_token", i);
}
if (index_sizes.string == 4)
name = pe_get_dotnet_string(pe,
string_offset,
*(DWORD*) (row_ptr + 2 + 2 + 2 + 2 + 4 + index_sizes.blob));
else
name = pe_get_dotnet_string(pe,
string_offset,
*(WORD*) (row_ptr + 2 + 2 + 2 + 2 + 4 + index_sizes.blob));
if (name != NULL)
set_string(name, pe->object, "assembly_refs[%i].name", i);
row_ptr += row_size;
}
set_integer(i, pe->object, "number_of_assembly_refs");
table_offset += row_size * num_rows;
break;
case BIT_ASSEMBLYREFPROCESSOR:
table_offset += (4 + index_sizes.assemblyrefprocessor) * num_rows;
break;
case BIT_ASSEMBLYREFOS:
table_offset += (4 + 4 + 4 + index_sizes.assemblyref) * num_rows;
break;
case BIT_FILE:
table_offset += (4 + index_sizes.string + index_sizes.blob) * num_rows;
break;
case BIT_EXPORTEDTYPE:
row_count = max_rows(3, rows.file, rows.assemblyref, rows.exportedtype);
if (row_count > (0xFFFF >> 0x02))
index_size = 4;
else
index_size = 2;
table_offset += (4 + 4 + (index_sizes.string * 2) + index_size) * num_rows;
break;
case BIT_MANIFESTRESOURCE:
row_count = max_rows(2, rows.file, rows.assemblyref);
if (row_count > (0xFFFF >> 0x02))
index_size = 4;
else
index_size = 2;
row_size = (4 + 4 + index_sizes.string + index_size);
counter = 0;
row_ptr = table_offset;
for (i = 0; i < num_rows; i++)
{
if (!fits_in_pe(pe, row_ptr, row_size))
break;
manifestresource_table = (PMANIFESTRESOURCE_TABLE) row_ptr;
resource_offset = manifestresource_table->Offset;
if (index_size == 4)
implementation = *(DWORD*) (row_ptr + 4 + 4 + index_sizes.string);
else
implementation = *(WORD*) (row_ptr + 4 + 4 + index_sizes.string);
if (implementation != 0)
{
row_ptr += row_size;
continue;
}
if (!fits_in_pe(
pe,
pe->data + resource_base + resource_offset,
sizeof(DWORD)))
{
row_ptr += row_size;
continue;
}
resource_size = *(DWORD*)(pe->data + resource_base + resource_offset);
if (!fits_in_pe(
pe, pe->data + resource_base +
resource_offset,
resource_size))
{
row_ptr += row_size;
continue;
}
set_integer(resource_base + resource_offset + 4,
pe->object, "resources[%i].offset", counter);
set_integer(resource_size,
pe->object, "resources[%i].length", counter);
name = pe_get_dotnet_string(pe,
string_offset,
DOTNET_STRING_INDEX(manifestresource_table->Name));
if (name != NULL)
set_string(name, pe->object, "resources[%i].name", counter);
row_ptr += row_size;
counter++;
}
set_integer(counter, pe->object, "number_of_resources");
table_offset += row_size * num_rows;
break;
case BIT_NESTEDCLASS:
table_offset += (index_sizes.typedef_ * 2) * num_rows;
break;
case BIT_GENERICPARAM:
row_count = max_rows(2, rows.typedef_, rows.methoddef);
if (row_count > (0xFFFF >> 0x01))
index_size = 4;
else
index_size = 2;
table_offset += (2 + 2 + index_size + index_sizes.string) * num_rows;
break;
case BIT_METHODSPEC:
row_count = max_rows(2, rows.methoddef, rows.memberref);
if (row_count > (0xFFFF >> 0x01))
index_size = 4;
else
index_size = 2;
table_offset += (index_size + index_sizes.blob) * num_rows;
break;
case BIT_GENERICPARAMCONSTRAINT:
row_count = max_rows(3, rows.typedef_, rows.typeref, rows.typespec);
if (row_count > (0xFFFF >> 0x02))
index_size = 4;
else
index_size = 2;
table_offset += (index_sizes.genericparam + index_size) * num_rows;
break;
default:
return;
}
matched_bits++;
}
}
void dotnet_parse_tilde(
PE* pe,
ut64 metadata_root,
PCLI_HEADER cli_header,
PSTREAMS streams)
{
PTILDE_HEADER tilde_header;
ut64 resource_base;
uint32_t* row_offset = NULL;
int bit_check;
int matched_bits = 0;
ROWS rows;
INDEX_SIZES index_sizes;
memset(&rows, '\0', sizeof(ROWS));
memset(&index_sizes, 2, sizeof(index_sizes));
tilde_header = (PTILDE_HEADER) (
pe->data +
metadata_root +
streams->tilde->Offset);
if (!struct_fits_in_pe(pe, tilde_header, TILDE_HEADER))
return;
if (tilde_header->HeapSizes & 0x01)
index_sizes.string = 4;
if (tilde_header->HeapSizes & 0x02)
index_sizes.guid = 4;
if (tilde_header->HeapSizes & 0x04)
index_sizes.blob = 4;
row_offset = (uint32_t*) (tilde_header + 1);
for (bit_check = 0; bit_check < 64; bit_check++)
{
if (!((tilde_header->Valid >> bit_check) & 0x01))
continue;
#define ROW_CHECK(name) if (fits_in_pe(pe, row_offset, (matched_bits + 1) * sizeof(uint32_t))) rows.name = *(row_offset + matched_bits);
#define ROW_CHECK_WITH_INDEX(name) ROW_CHECK(name); if (rows.name > 0xFFFF) index_sizes.name = 4;
switch (bit_check)
{
case BIT_MODULE:
ROW_CHECK(module);
break;
case BIT_MODULEREF:
ROW_CHECK_WITH_INDEX(moduleref);
break;
case BIT_ASSEMBLYREF:
ROW_CHECK_WITH_INDEX(assemblyref);
break;
case BIT_ASSEMBLYREFPROCESSOR:
ROW_CHECK_WITH_INDEX(assemblyrefprocessor);
break;
case BIT_TYPEREF:
ROW_CHECK(typeref);
break;
case BIT_METHODDEF:
ROW_CHECK_WITH_INDEX(methoddef);
break;
case BIT_MEMBERREF:
ROW_CHECK_WITH_INDEX(memberref);
break;
case BIT_TYPEDEF:
ROW_CHECK_WITH_INDEX(typedef_);
break;
case BIT_TYPESPEC:
ROW_CHECK(typespec);
break;
case BIT_FIELD:
ROW_CHECK_WITH_INDEX(field);
break;
case BIT_PARAM:
ROW_CHECK_WITH_INDEX(param);
break;
case BIT_PROPERTY:
ROW_CHECK_WITH_INDEX(property);
break;
case BIT_INTERFACEIMPL:
ROW_CHECK(interfaceimpl);
break;
case BIT_EVENT:
ROW_CHECK_WITH_INDEX(event);
break;
case BIT_STANDALONESIG:
ROW_CHECK(standalonesig);
break;
case BIT_ASSEMBLY:
ROW_CHECK(assembly);
break;
case BIT_FILE:
ROW_CHECK(file);
break;
case BIT_EXPORTEDTYPE:
ROW_CHECK(exportedtype);
break;
case BIT_MANIFESTRESOURCE:
ROW_CHECK(manifestresource);
break;
case BIT_GENERICPARAM:
ROW_CHECK_WITH_INDEX(genericparam);
break;
case BIT_GENERICPARAMCONSTRAINT:
ROW_CHECK(genericparamconstraint);
break;
case BIT_METHODSPEC:
ROW_CHECK(methodspec);
break;
default:
break;
}
matched_bits++;
}
resource_base = pe_rva_to_offset(pe, cli_header->Resources.VirtualAddress);
dotnet_parse_tilde_2(
pe,
tilde_header,
resource_base,
metadata_root,
rows,
index_sizes,
streams);
}
#if 0
pe->data = block_data;
pe->data_size = block->size;
pe->object = module_object;
pe->header = pe_header;
module_object->data = pe;
dotnet_parse_com(pe, block->base);
#endif
void dotnet_parse_com(PE* pe, ut64 baddr) {
PIMAGE_DATA_DIRECTORY directory;
PCLI_HEADER cli_header;
PNET_METADATA metadata;
ut64 metadata_root;
char* end;
STREAMS headers;
WORD num_streams;
directory = pe_get_directory_entry (pe, PE_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
st64 offset = pe_rva_to_offset (pe, directory->VirtualAddress);
if (offset < 0 || !struct_fits_in_pe(pe, pe->data + offset, CLI_HEADER)) {
return;
}
cli_header = (PCLI_HEADER) (pe->data + offset);
offset = metadata_root = pe_rva_to_offset(
pe, cli_header->MetaData.VirtualAddress);
if (!struct_fits_in_pe(pe, pe->data + offset, NET_METADATA))
return;
metadata = (PNET_METADATA) (pe->data + offset);
if (metadata->Magic != NET_METADATA_MAGIC)
return;
if (metadata->Length == 0 ||
metadata->Length > 255 ||
metadata->Length % 4 != 0 ||
!fits_in_pe(pe, pe->data + offset, metadata->Length))
{
return;
}
end = (char*) memmem((void*) metadata->Version, metadata->Length, "\0", 1);
if (end != NULL)
set_sized_string(metadata->Version,
(end - metadata->Version),
pe->object,
"version");
offset += sizeof(NET_METADATA) + metadata->Length + 2;
if (!fits_in_pe(pe, pe->data + offset, 2))
return;
num_streams = (WORD) *(pe->data + offset);
offset += 2;
headers = dotnet_parse_stream_headers(pe, offset, metadata_root, num_streams);
if (headers.guid) {
dotnet_parse_guid(pe, metadata_root, headers.guid);
}
if (headers.tilde && headers.string && headers.blob) {
dotnet_parse_tilde (pe, metadata_root, cli_header, &headers);
}
if (headers.us) {
dotnet_parse_us (pe, metadata_root, headers.us);
}
}
void dotnet_parse(const ut8 *buf, int size, ut64 baddr) {
PE pe = { buf, (DWORD)size, NULL};
dotnet_parse_com (&pe, baddr);
}
