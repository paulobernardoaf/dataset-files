


































































































#include <r_types.h>
#include <r_lib.h>
#include <r_cmd.h>
#include <r_sign.h>
#include <signal.h>

#define DEBUG 0


#define IDASIG__ARCH__386 0 
#define IDASIG__ARCH__Z80 1 
#define IDASIG__ARCH__I860 2 
#define IDASIG__ARCH__8051 3 
#define IDASIG__ARCH__TMS 4 
#define IDASIG__ARCH__6502 5 
#define IDASIG__ARCH__PDP 6 
#define IDASIG__ARCH__68K 7 
#define IDASIG__ARCH__JAVA 8 
#define IDASIG__ARCH__6800 9 
#define IDASIG__ARCH__ST7 10 
#define IDASIG__ARCH__MC6812 11 
#define IDASIG__ARCH__MIPS 12 
#define IDASIG__ARCH__ARM 13 
#define IDASIG__ARCH__TMSC6 14 
#define IDASIG__ARCH__PPC 15 
#define IDASIG__ARCH__80196 16 
#define IDASIG__ARCH__Z8 17 
#define IDASIG__ARCH__SH 18 
#define IDASIG__ARCH__NET 19 
#define IDASIG__ARCH__AVR 20 
#define IDASIG__ARCH__H8 21 
#define IDASIG__ARCH__PIC 22 
#define IDASIG__ARCH__SPARC 23 
#define IDASIG__ARCH__ALPHA 24 
#define IDASIG__ARCH__HPPA 25 
#define IDASIG__ARCH__H8500 26 
#define IDASIG__ARCH__TRICORE 27 
#define IDASIG__ARCH__DSP56K 28 
#define IDASIG__ARCH__C166 29 
#define IDASIG__ARCH__ST20 30 
#define IDASIG__ARCH__IA64 31 
#define IDASIG__ARCH__I960 32 
#define IDASIG__ARCH__F2MC 33 
#define IDASIG__ARCH__TMS320C54 34 
#define IDASIG__ARCH__TMS320C55 35 
#define IDASIG__ARCH__TRIMEDIA 36 
#define IDASIG__ARCH__M32R 37 
#define IDASIG__ARCH__NEC_78K0 38 
#define IDASIG__ARCH__NEC_78K0S 39 
#define IDASIG__ARCH__M740 40 
#define IDASIG__ARCH__M7700 41 
#define IDASIG__ARCH__ST9 42 
#define IDASIG__ARCH__FR 43 
#define IDASIG__ARCH__MC6816 44 
#define IDASIG__ARCH__M7900 45 
#define IDASIG__ARCH__TMS320C3 46 
#define IDASIG__ARCH__KR1878 47 
#define IDASIG__ARCH__AD218X 48 
#define IDASIG__ARCH__OAKDSP 49 
#define IDASIG__ARCH__TLCS900 50 
#define IDASIG__ARCH__C39 51 
#define IDASIG__ARCH__CR16 52 
#define IDASIG__ARCH__MN102L00 53 
#define IDASIG__ARCH__TMS320C1X 54 
#define IDASIG__ARCH__NEC_V850X 55 
#define IDASIG__ARCH__SCR_ADPT 56 
#define IDASIG__ARCH__EBC 57 
#define IDASIG__ARCH__MSP430 58 
#define IDASIG__ARCH__SPU 59 
#define IDASIG__ARCH__DALVIK 60 


#define IDASIG__FILE__DOS_EXE_OLD 0x00000001
#define IDASIG__FILE__DOS_COM_OLD 0x00000002
#define IDASIG__FILE__BIN 0x00000004
#define IDASIG__FILE__DOSDRV 0x00000008
#define IDASIG__FILE__NE 0x00000010
#define IDASIG__FILE__INTELHEX 0x00000020
#define IDASIG__FILE__MOSHEX 0x00000040
#define IDASIG__FILE__LX 0x00000080
#define IDASIG__FILE__LE 0x00000100
#define IDASIG__FILE__NLM 0x00000200
#define IDASIG__FILE__COFF 0x00000400
#define IDASIG__FILE__PE 0x00000800
#define IDASIG__FILE__OMF 0x00001000
#define IDASIG__FILE__SREC 0x00002000
#define IDASIG__FILE__ZIP 0x00004000
#define IDASIG__FILE__OMFLIB 0x00008000
#define IDASIG__FILE__AR 0x00010000
#define IDASIG__FILE__LOADER 0x00020000
#define IDASIG__FILE__ELF 0x00040000
#define IDASIG__FILE__W32RUN 0x00080000
#define IDASIG__FILE__AOUT 0x00100000
#define IDASIG__FILE__PILOT 0x00200000
#define IDASIG__FILE__DOS_EXE 0x00400000
#define IDASIG__FILE__DOS_COM 0x00800000
#define IDASIG__FILE__AIXAR 0x01000000


#define IDASIG__OS__MSDOS 0x01
#define IDASIG__OS__WIN 0x02
#define IDASIG__OS__OS2 0x04
#define IDASIG__OS__NETWARE 0x08
#define IDASIG__OS__UNIX 0x10
#define IDASIG__OS__OTHER 0x20


#define IDASIG__APP__CONSOLE 0x0001
#define IDASIG__APP__GRAPHICS 0x0002
#define IDASIG__APP__EXE 0x0004
#define IDASIG__APP__DLL 0x0008
#define IDASIG__APP__DRV 0x0010
#define IDASIG__APP__SINGLE_THREADED 0x0020
#define IDASIG__APP__MULTI_THREADED 0x0040
#define IDASIG__APP__16_BIT 0x0080
#define IDASIG__APP__32_BIT 0x0100
#define IDASIG__APP__64_BIT 0x0200


#define IDASIG__FEATURE__STARTUP 0x01
#define IDASIG__FEATURE__CTYPE_CRC 0x02
#define IDASIG__FEATURE__2BYTE_CTYPE 0x04
#define IDASIG__FEATURE__ALT_CTYPE_CRC 0x08
#define IDASIG__FEATURE__COMPRESSED 0x10


#define IDASIG__PARSE__MORE_PUBLIC_NAMES 0x01
#define IDASIG__PARSE__READ_TAIL_BYTES 0x02
#define IDASIG__PARSE__READ_REFERENCED_FUNCTIONS 0x04
#define IDASIG__PARSE__MORE_MODULES_WITH_SAME_CRC 0x08
#define IDASIG__PARSE__MORE_MODULES 0x10


#define IDASIG__FUNCTION__LOCAL 0x02 
#define IDASIG__FUNCTION__UNRESOLVED_COLLISION 0x08 

typedef struct idasig_v5_t {

ut8 magic[6]; 
ut8 version; 
ut8 arch;
ut32 file_types;
ut16 os_types;
ut16 app_types;
ut16 features;
ut16 old_n_functions;
ut16 crc16;
ut8 ctype[12]; 
ut8 library_name_len;
ut16 ctypes_crc16;
} idasig_v5_t;

typedef struct idasig_v6_v7_t {
ut32 n_functions;
} idasig_v6_v7_t;

typedef struct idasig_v8_v9_t {
ut16 pattern_size;
} idasig_v8_v9_t;

typedef struct idasig_v10_t {
ut16 unknown;
} idasig_v10_t;

#if DEBUG
static int header_size = 0;
#endif





















#define R_FLIRT_NAME_MAX 1024

typedef struct RFlirtTailByte {
ut16 offset; 
ut8 value;
} RFlirtTailByte;

typedef struct RFlirtFunction {
char name[R_FLIRT_NAME_MAX];
ut16 offset; 
ut8 negative_offset; 
ut8 is_local; 
ut8 is_collision; 
} RFlirtFunction;

typedef struct RFlirtModule {
ut32 crc_length;
ut32 crc16; 


ut16 length; 
RList *public_functions;
RList *tail_bytes;
RList *referenced_functions;
} RFlirtModule;

typedef struct RFlirtNode {
RList *child_list;
RList *module_list;
ut32 length; 
ut64 variant_mask; 
ut8 *pattern_bytes; 
ut8 *variant_bool_array; 
} RFlirtNode;

static ut8 version; 



#define POLY 0x8408
ut16 crc16(const unsigned char *data_p, size_t length) {
ut8 i;
ut32 data;
ut32 crc = 0xFFFF;

if (length == 0) {
return 0;
}
do {
data = *data_p++;
for (i = 0; i < 8; i++) {
if ((crc ^ data) & 1) {
crc = (crc >> 1) ^ POLY;
} else {
crc >>= 1;
}
data >>= 1;
}
} while (--length > 0);

crc = ~crc;
data = crc;
crc = (crc << 8) | ((data >> 8) & 0xff);
return (ut16) (crc);
}


static bool buf_eof;
static bool buf_err;

static ut8 read_byte(RBuffer *b) {
ut8 r = 0;
int length;

if (buf_eof || buf_err) {
return 0;
}
if ((length = r_buf_read (b, &r, 1)) != 1) {
if (length == -1) {
buf_err = true;
}
if (length == 0) {
buf_eof = true;
}
return 0;
}
return r;
}

static ut16 read_short(RBuffer *b) {
ut16 r = (read_byte (b) << 8);
r += read_byte (b);
return r;
}

static ut32 read_word(RBuffer *b) {
ut32 r = (read_short (b) << 16);
r += read_short (b);
return r;
}

static ut16 read_max_2_bytes(RBuffer *b) {
ut16 r = read_byte (b);
return (r & 0x80)
? ((r & 0x7f) << 8) + read_byte (b)
: r;
}

static ut32 read_multiple_bytes(RBuffer *b) {
ut32 r = read_byte (b);
if ((r & 0x80) != 0x80) {
return r;
}
if ((r & 0xc0) != 0xc0) {
return ((r & 0x7f) << 8) + read_byte (b);
}
if ((r & 0xe0) != 0xe0) {
r = ((r & 0x3f) << 24) + (read_byte (b) << 16);
r += read_short (b);
return r;
}
return read_word (b);
}

static void module_free(RFlirtModule *module) {
if (!module) {
return;
}
if (module->public_functions) {
module->public_functions->free = (RListFree) free;
r_list_free (module->public_functions);
}
if (module->tail_bytes) {
module->tail_bytes->free = (RListFree) free;
r_list_free (module->tail_bytes);
}
if (module->referenced_functions) {
module->referenced_functions->free = (RListFree) free;
r_list_free (module->referenced_functions);
}
free (module);
}

static void node_free(RFlirtNode *node) {
if (!node) {
return;
}
free (node->variant_bool_array);
free (node->pattern_bytes);
if (node->module_list) {
node->module_list->free = (RListFree)module_free;
r_list_free (node->module_list);
}
if (node->child_list) {
node->child_list->free = (RListFree) node_free;
r_list_free (node->child_list);
}
free (node);
}

static void print_module(const RAnal *anal, const RFlirtModule *module) {
RListIter *pub_func_it, *ref_func_it, *tail_byte_it;
RFlirtFunction *func, *ref_func;
RFlirtTailByte *tail_byte;

anal->cb_printf ("%02X %04X %04X ", module->crc_length, module->crc16, module->length);
r_list_foreach (module->public_functions, pub_func_it, func) {
if (func->is_local || func->is_collision) {
anal->cb_printf ("(");
if (func->is_local) {
anal->cb_printf ("l");
}
if (func->is_collision) {
anal->cb_printf ("!");
}
anal->cb_printf (")");
}
anal->cb_printf ("%04X:%s", func->offset, func->name);
if (pub_func_it->n) {
anal->cb_printf (" ");
}
}
if (module->tail_bytes) {
r_list_foreach (module->tail_bytes, tail_byte_it, tail_byte) {
anal->cb_printf (" (%04X: %02X)", tail_byte->offset, tail_byte->value);
}
}
if (module->referenced_functions) {
anal->cb_printf (" (REF ");
r_list_foreach (module->referenced_functions, ref_func_it, ref_func) {
anal->cb_printf ("%04X: %s", ref_func->offset, ref_func->name);
if (ref_func_it->n) {
anal->cb_printf (" ");
}
}
anal->cb_printf (")");
}
anal->cb_printf ("\n");
}


static void print_node_pattern(const RAnal *anal, const RFlirtNode *node) {
int i;
for (i = 0; i < node->length; i++) {
if (node->variant_bool_array[i]) {
anal->cb_printf ("..");
} else {
anal->cb_printf ("%02X", node->pattern_bytes[i]);
}
}
anal->cb_printf (":\n");
}

static void print_indentation(const RAnal *anal, int indent) {
anal->cb_printf ("%s", r_str_pad (' ', indent));
}

static void print_node(const RAnal *anal, const RFlirtNode *node, int indent) {

int i;
RListIter *child_it, *module_it;
RFlirtNode *child;
RFlirtModule *module;

if (node->pattern_bytes) { 
print_indentation (anal, indent);
print_node_pattern (anal, node);
}
if (node->child_list) {
r_list_foreach (node->child_list, child_it, child) {
print_node (anal, child, indent + 1);
}
} else if (node->module_list) {
i = 0;
r_list_foreach (node->module_list, module_it, module) {
print_indentation (anal, indent + 1);
anal->cb_printf ("%d. ", i);
print_module (anal, module);
i++;
}
}
}

static int module_match_buffer(RAnal *anal, const RFlirtModule *module,
ut8 *b, ut64 address, ut32 buf_size) {



RFlirtFunction *flirt_func;
RAnalFunction *next_module_function;
RListIter *tail_byte_it, *flirt_func_it;
RFlirtTailByte *tail_byte;

if (32 + module->crc_length < buf_size &&
module->crc16 != crc16 (b + 32, module->crc_length)) {
return false;
}
if (module->tail_bytes) {
r_list_foreach (module->tail_bytes, tail_byte_it, tail_byte) {
if (32 + module->crc_length + tail_byte->offset < buf_size &&
b[32 + module->crc_length + tail_byte->offset] != tail_byte->value) {
return false;
}
}
}



r_list_foreach (module->public_functions, flirt_func_it, flirt_func) {



next_module_function = r_anal_get_function_at ((RAnal *) anal, address + flirt_func->offset);
if (next_module_function) {
char *name;
int name_offs = 0;
ut32 next_module_function_size;


ut64 flirt_fcn_size = module->length - flirt_func->offset;
RFlirtFunction *next_flirt_func;
RListIter *next_flirt_func_it = flirt_func_it->n;
while (next_flirt_func_it) {
next_flirt_func = next_flirt_func_it->data;
if (!next_flirt_func->is_local && !next_flirt_func->negative_offset) {
flirt_fcn_size = next_flirt_func->offset - flirt_func->offset;
break;
}
next_flirt_func_it = next_flirt_func_it->n;
}

next_module_function_size = r_anal_function_linear_size (next_module_function);
if (next_module_function_size < flirt_fcn_size) {
RListIter *iter;
RListIter *iter_tmp;
RAnalFunction *fcn;
r_list_foreach_safe (anal->fcns, iter, iter_tmp, fcn) {
if (fcn != next_module_function &&
fcn->addr >= next_module_function->addr + next_module_function_size &&
fcn->addr < next_module_function->addr + flirt_fcn_size) {
RListIter *iter_bb;
RAnalBlock *block;
r_list_foreach (fcn->bbs, iter_bb, block) {
r_anal_function_add_block (next_module_function, block);
}
next_module_function->ninstr += fcn->ninstr;
r_anal_function_delete (fcn);
}
}
r_anal_function_resize (next_module_function, flirt_fcn_size);
next_module_function_size = r_anal_function_linear_size (next_module_function);
r_anal_trim_jmprefs ((RAnal *)anal, next_module_function);
}


while (flirt_func->name[name_offs] == '?') { 
name_offs++;
}
if (!flirt_func->name[name_offs]) {
continue;
}
name = r_name_filter2 (flirt_func->name + name_offs);
free (next_module_function->name);
next_module_function->name = r_str_newf ("flirt.%s", name);
anal->flb.set (anal->flb.f, next_module_function->name,
next_module_function->addr, next_module_function_size);
anal->cb_printf ("Found %s\n", next_module_function->name);
free (name);
}
}
return true;
}



static int node_pattern_match(const RFlirtNode *node, ut8 *b, int buf_size) {
int i;
if (buf_size < node->length) {
return false;
}
for (i = 0; i < node->length; i++) {
if (!node->variant_bool_array[i]) {
if (i < node->length && node->pattern_bytes[i] != b[i]) {
return false;
}
}
}
return true;
}

static int node_match_buffer(RAnal *anal, const RFlirtNode *node, ut8 *b, ut64 address, ut32 buf_size, ut32 buf_idx) {
RListIter *node_child_it, *module_it;
RFlirtNode *child;
RFlirtModule *module;

if (node_pattern_match (node, b + buf_idx, buf_size - buf_idx)) {
if (node->child_list) {
r_list_foreach (node->child_list, node_child_it, child) {
if (node_match_buffer (anal, child, b, address, buf_size, buf_idx + node->length)) {
return true;
}
}
} else if (node->module_list) {
r_list_foreach (node->module_list, module_it, module) {
if (module_match_buffer (anal, module, b, address, buf_size)) {
return true;
}
}
}
}

return false;
}

static int node_match_functions(RAnal *anal, const RFlirtNode *root_node) {




if (r_list_length (anal->fcns) == 0) {
anal->cb_printf ("There are no analyzed functions. Have you run 'aa'?\n");
return true;
}

anal->flb.push_fs (anal->flb.f, "flirt");
RListIter *it_func;
RAnalFunction *func;
r_list_foreach (anal->fcns, it_func, func) {
if (func->type != R_ANAL_FCN_TYPE_FCN && func->type != R_ANAL_FCN_TYPE_LOC) { 
continue;
}

ut64 func_size = r_anal_function_linear_size (func);
ut8 *func_buf = malloc (func_size);
if (!func_buf) {
continue;
}
if (!anal->iob.read_at (anal->iob.io, func->addr, func_buf, (int)func_size)) {
eprintf ("Couldn't read function %s at 0x%"PFMT64x"\n", func->name, func->addr);
free (func_buf);
continue;
}
RListIter *node_child_it;
RFlirtNode *child;
r_list_foreach (root_node->child_list, node_child_it, child) {
if (node_match_buffer (anal, child, func_buf, func->addr, func_size, 0)) {
break;
}
}
free (func_buf);
}
anal->flb.pop_fs (anal->flb.f);

return true;
}

static ut8 read_module_tail_bytes(RFlirtModule *module, RBuffer *b) {


int i;
ut8 number_of_tail_bytes;
RFlirtTailByte *tail_byte = NULL;
if (!(module->tail_bytes = r_list_newf ((RListFree) free))) {
goto err_exit;
}

if (version >= 8) { 
number_of_tail_bytes = read_byte (b); 
if (buf_eof || buf_err) {
goto err_exit;
}
} else { 
number_of_tail_bytes = 1;
}
for (i = 0; i < number_of_tail_bytes; i++) {
tail_byte = R_NEW0 (RFlirtTailByte);
if (!tail_byte) {
return false;
}
if (version >= 9) {

tail_byte->offset = read_multiple_bytes (b);
if (buf_eof || buf_err) {
goto err_exit;
}
} else {
tail_byte->offset = read_max_2_bytes (b);
if (buf_eof || buf_err) {
goto err_exit;
}
}
tail_byte->value = read_byte (b);
if (buf_eof || buf_err) {
goto err_exit;
}
r_list_append (module->tail_bytes, tail_byte);
#if DEBUG
eprintf ("READ TAIL BYTE: %04X: %02X\n", tail_byte->offset, tail_byte->value);
#endif
}

return true;

err_exit:
free (tail_byte);
r_list_free (module->tail_bytes);
return false;
}

static ut8 read_module_referenced_functions(RFlirtModule *module, RBuffer *b) {


int i, j;
ut8 number_of_referenced_functions;
ut32 ref_function_name_length;
RFlirtFunction *ref_function = NULL;

module->referenced_functions = r_list_new ();

if (version >= 8) { 
number_of_referenced_functions = read_byte (b); 
if (buf_eof || buf_err) {
goto err_exit;
}
} else { 
number_of_referenced_functions = 1;
}

for (i = 0; i < number_of_referenced_functions; i++) {
ref_function = R_NEW0 (RFlirtFunction);
if (!ref_function) {
goto err_exit;
}
if (version >= 9) {
ref_function->offset = read_multiple_bytes (b);
if (buf_eof || buf_err) {
goto err_exit;
}
} else {
ref_function->offset = read_max_2_bytes (b);
if (buf_eof || buf_err) {
goto err_exit;
}
}
ref_function_name_length = read_byte (b);
if (buf_eof || buf_err) {
goto err_exit;
}
if (!ref_function_name_length) {

ref_function_name_length = read_multiple_bytes (b); 
if (buf_eof || buf_err) {
goto err_exit;
}
}
if ((int) ref_function_name_length < 0 || ref_function_name_length >= R_FLIRT_NAME_MAX) {
goto err_exit;
}
for (j = 0; j < ref_function_name_length; j++) {
ref_function->name[j] = read_byte (b);
if (buf_eof || buf_err) {
goto err_exit;
}
}
if (!ref_function->name[ref_function_name_length]) {

ref_function->negative_offset = true;
} else {
ref_function->name[ref_function_name_length] = '\0';
}
r_list_append (module->referenced_functions, ref_function);
#if DEBUG
eprintf ("(REF: %04X: %s)\n", ref_function->offset, ref_function->name);
#endif
}

return true;

err_exit:
free (ref_function);
return false;
}

static ut8 read_module_public_functions(RFlirtModule *module, RBuffer *b, ut8 *flags) {


int i;
ut16 offset = 0;
ut8 current_byte;
RFlirtFunction *function = NULL;

module->public_functions = r_list_new ();

do {
function = R_NEW0 (RFlirtFunction);
if (version >= 9) { 
offset += read_multiple_bytes (b); 
if (buf_eof || buf_err) {
goto err_exit;
}
} else {
offset += read_max_2_bytes (b); 
if (buf_eof || buf_err) {
goto err_exit;
}
}
function->offset = offset;

current_byte = read_byte (b);
if (buf_eof || buf_err) {
goto err_exit;
}
if (current_byte < 0x20) {
if (current_byte & IDASIG__FUNCTION__LOCAL) { 
function->is_local = true;
}
if (current_byte & IDASIG__FUNCTION__UNRESOLVED_COLLISION) {

function->is_collision = true;
}
if (current_byte & 0x01 || current_byte & 0x04) { 
#if DEBUG

eprintf ("INVESTIGATE PUBLIC NAME FLAG: %02X @ %04X\n", current_byte,
r_buf_tell (b) + header_size);
#endif
}
current_byte = read_byte (b);
if (buf_eof || buf_err) {
goto err_exit;
}
}

for (i = 0; current_byte >= 0x20 && i < R_FLIRT_NAME_MAX; i++) {
function->name[i] = current_byte;
current_byte = read_byte (b);
if (buf_eof || buf_err) {
goto err_exit;
}
}

if (i == R_FLIRT_NAME_MAX) {
eprintf ("Function name too long\n");
function->name[R_FLIRT_NAME_MAX - 1] = '\0';
} else {
function->name[i] = '\0';
}

#if DEBUG
eprintf ("%04X:%s ", function->offset, function->name);
#endif
*flags = current_byte;
r_list_append (module->public_functions, function);
} while (*flags & IDASIG__PARSE__MORE_PUBLIC_NAMES);
#if DEBUG
eprintf ("\n");
#endif

return true;

err_exit:
free (function);
return false;
}

static ut8 parse_leaf(const RAnal *anal, RBuffer *b, RFlirtNode *node) {


ut8 flags, crc_length;
ut16 crc16;
RFlirtModule *module = NULL;

node->module_list = r_list_new ();
do { 

crc_length = read_byte (b); if (buf_eof || buf_err) {
goto err_exit;
}
crc16 = read_short (b); if (buf_eof || buf_err) {
goto err_exit;
}
#if DEBUG
if (crc_length == 0x00 && crc16 != 0x0000) {
eprintf ("WARNING non zero crc of zero length @ %04X\n",
r_buf_tell (b) + header_size);
}
eprintf ("crc_len: %02X crc16: %04X\n", crc_length, crc16);
#endif

do { 
module = R_NEW0 (RFlirtModule);
if (!module) {
goto err_exit;
}

module->crc_length = crc_length;
module->crc16 = crc16;

if (version >= 9) { 

module->length = read_multiple_bytes (b); 
if (buf_eof || buf_err) {
goto err_exit;
}
} else {
module->length = read_max_2_bytes (b); 
if (buf_eof || buf_err) {
goto err_exit;
}
}
#if DEBUG
eprintf ("module_length: %04X\n", module->length);
#endif

if (!read_module_public_functions (module, b, &flags)) {
goto err_exit;
}

if (flags & IDASIG__PARSE__READ_TAIL_BYTES) { 
if (!read_module_tail_bytes (module, b)) {
goto err_exit;
}
}
if (flags & IDASIG__PARSE__READ_REFERENCED_FUNCTIONS) { 
if (!read_module_referenced_functions (module, b)) {
goto err_exit;
}
}

r_list_append (node->module_list, module);
} while (flags & IDASIG__PARSE__MORE_MODULES_WITH_SAME_CRC);
} while (flags & IDASIG__PARSE__MORE_MODULES); 

return true;

err_exit:
module_free (module);
return false;
}

static ut8 read_node_length(RFlirtNode *node, RBuffer *b) {
node->length = read_byte (b);
if (buf_eof || buf_err) {
return false;
}
#if DEBUG
eprintf ("node length: %02X\n", node->length);
#endif
return true;
}

static ut8 read_node_variant_mask(RFlirtNode *node, RBuffer *b) {



if (node->length < 0x10) {
node->variant_mask = read_max_2_bytes (b);
if (buf_eof || buf_err) {
return false;
}
} else if (node->length <= 0x20) {
node->variant_mask = read_multiple_bytes (b);
if (buf_eof || buf_err) {
return false;
}
} else if (node->length <= 0x40) { 
node->variant_mask = ((ut64)read_multiple_bytes (b) << 32) + read_multiple_bytes (b);
if (buf_eof || buf_err) {
return false;
}
}

return true;
}

static bool read_node_bytes(RFlirtNode *node, RBuffer *b) {


int i;
ut64 current_mask_bit = 0;
if ((int) node->length < 0) {
return false;
}
current_mask_bit = 1ULL << (node->length - 1);
if (!(node->pattern_bytes = malloc (node->length))) {
return false;
}
if (!(node->variant_bool_array = malloc (node->length))) {
return false;
}
for (i = 0; i < node->length; i++, current_mask_bit >>= 1) {
node->variant_bool_array[i] = (bool)(node->variant_mask & current_mask_bit);
if (node->variant_mask & current_mask_bit) {
node->pattern_bytes[i] = 0x00;
} else {
node->pattern_bytes[i] = read_byte (b);
if (buf_eof || buf_err) {
return false;
}
}
}
return true;
}

static ut8 parse_tree(const RAnal *anal, RBuffer *b, RFlirtNode *root_node) {


RFlirtNode *node = NULL;
int i, tree_nodes = read_multiple_bytes (b); 
if (buf_eof || buf_err) {
return false;
}
if (tree_nodes == 0) { 
return parse_leaf (anal, b, root_node);
}
root_node->child_list = r_list_new ();

for (i = 0; i < tree_nodes; i++) {
if (!(node = R_NEW0 (RFlirtNode))) {
goto err_exit;
}
if (!read_node_length (node, b)) {
goto err_exit;
}
if (!read_node_variant_mask (node, b)) {
goto err_exit;
}
if (!read_node_bytes (node, b)) {
goto err_exit;
}
r_list_append (root_node->child_list, node);
if (!parse_tree (anal, b, node)) {
goto err_exit; 
}
}
return true;
err_exit:
node_free (node);
return false;
}

#if DEBUG
#define PRINT_ARCH(define, str) if (arch == define) { eprintf (" %s", str); return; }
static void print_arch(ut8 arch) {
PRINT_ARCH (IDASIG__ARCH__386, "386");
PRINT_ARCH (IDASIG__ARCH__Z80, "Z80");
PRINT_ARCH (IDASIG__ARCH__I860, "I860");
PRINT_ARCH (IDASIG__ARCH__8051, "8051");
PRINT_ARCH (IDASIG__ARCH__TMS, "TMS");
PRINT_ARCH (IDASIG__ARCH__6502, "6502");
PRINT_ARCH (IDASIG__ARCH__PDP, "PDP");
PRINT_ARCH (IDASIG__ARCH__68K, "68K");
PRINT_ARCH (IDASIG__ARCH__JAVA, "JAVA");
PRINT_ARCH (IDASIG__ARCH__6800, "6800");
PRINT_ARCH (IDASIG__ARCH__ST7, "ST7");
PRINT_ARCH (IDASIG__ARCH__MC6812, "MC6812");
PRINT_ARCH (IDASIG__ARCH__MIPS, "MIPS");
PRINT_ARCH (IDASIG__ARCH__ARM, "ARM");
PRINT_ARCH (IDASIG__ARCH__TMSC6, "TMSC6");
PRINT_ARCH (IDASIG__ARCH__PPC, "PPC");
PRINT_ARCH (IDASIG__ARCH__80196, "80196");
PRINT_ARCH (IDASIG__ARCH__Z8, "Z8");
PRINT_ARCH (IDASIG__ARCH__SH, "SH");
PRINT_ARCH (IDASIG__ARCH__NET, "NET");
PRINT_ARCH (IDASIG__ARCH__AVR, "AVR");
PRINT_ARCH (IDASIG__ARCH__H8, "H8");
PRINT_ARCH (IDASIG__ARCH__PIC, "PIC");
PRINT_ARCH (IDASIG__ARCH__SPARC, "SPARC");
PRINT_ARCH (IDASIG__ARCH__ALPHA, "ALPHA");
PRINT_ARCH (IDASIG__ARCH__HPPA, "HPPA");
PRINT_ARCH (IDASIG__ARCH__H8500, "H8500");
PRINT_ARCH (IDASIG__ARCH__TRICORE, "TRICORE");
PRINT_ARCH (IDASIG__ARCH__DSP56K, "DSP56K");
PRINT_ARCH (IDASIG__ARCH__C166, "C166");
PRINT_ARCH (IDASIG__ARCH__ST20, "ST20");
PRINT_ARCH (IDASIG__ARCH__IA64, "IA64");
PRINT_ARCH (IDASIG__ARCH__I960, "I960");
PRINT_ARCH (IDASIG__ARCH__F2MC, "F2MC");
PRINT_ARCH (IDASIG__ARCH__TMS320C54, "TMS320C54");
PRINT_ARCH (IDASIG__ARCH__TMS320C55, "TMS320C55");
PRINT_ARCH (IDASIG__ARCH__TRIMEDIA, "TRIMEDIA");
PRINT_ARCH (IDASIG__ARCH__M32R, "M32R");
PRINT_ARCH (IDASIG__ARCH__NEC_78K0, "NEC_78K0");
PRINT_ARCH (IDASIG__ARCH__NEC_78K0S, "NEC_78K0S");
PRINT_ARCH (IDASIG__ARCH__M740, "M740");
PRINT_ARCH (IDASIG__ARCH__M7700, "M7700");
PRINT_ARCH (IDASIG__ARCH__ST9, "ST9");
PRINT_ARCH (IDASIG__ARCH__FR, "FR");
PRINT_ARCH (IDASIG__ARCH__MC6816, "MC6816");
PRINT_ARCH (IDASIG__ARCH__M7900, "M7900");
PRINT_ARCH (IDASIG__ARCH__TMS320C3, "TMS320C3");
PRINT_ARCH (IDASIG__ARCH__KR1878, "KR1878");
PRINT_ARCH (IDASIG__ARCH__AD218X, "AD218X");
PRINT_ARCH (IDASIG__ARCH__OAKDSP, "OAKDSP");
PRINT_ARCH (IDASIG__ARCH__TLCS900, "TLCS900");
PRINT_ARCH (IDASIG__ARCH__C39, "C39");
PRINT_ARCH (IDASIG__ARCH__CR16, "CR16");
PRINT_ARCH (IDASIG__ARCH__MN102L00, "MN102L00");
PRINT_ARCH (IDASIG__ARCH__TMS320C1X, "TMS320C1X");
PRINT_ARCH (IDASIG__ARCH__NEC_V850X, "NEC_V850X");
PRINT_ARCH (IDASIG__ARCH__SCR_ADPT, "SCR_ADPT");
PRINT_ARCH (IDASIG__ARCH__EBC, "EBC");
PRINT_ARCH (IDASIG__ARCH__MSP430, "MSP430");
PRINT_ARCH (IDASIG__ARCH__SPU, "SPU");
PRINT_ARCH (IDASIG__ARCH__DALVIK, "DALVIK");
}

#define PRINT_FLAG(define, str) if (flags & define) { eprintf (" %s", str); }
static void print_file_types(ut32 flags) {
PRINT_FLAG (IDASIG__FILE__DOS_EXE_OLD, "DOS_EXE_OLD");
PRINT_FLAG (IDASIG__FILE__DOS_COM_OLD, "DOS_COM_OLD");
PRINT_FLAG (IDASIG__FILE__BIN, "BIN");
PRINT_FLAG (IDASIG__FILE__DOSDRV, "DOSDRV");
PRINT_FLAG (IDASIG__FILE__NE, "NE");
PRINT_FLAG (IDASIG__FILE__INTELHEX, "INTELHEX");
PRINT_FLAG (IDASIG__FILE__MOSHEX, "MOSHEX");
PRINT_FLAG (IDASIG__FILE__LX, "LX");
PRINT_FLAG (IDASIG__FILE__LE, "LE");
PRINT_FLAG (IDASIG__FILE__NLM, "NLM");
PRINT_FLAG (IDASIG__FILE__COFF, "COFF");
PRINT_FLAG (IDASIG__FILE__PE, "PE");
PRINT_FLAG (IDASIG__FILE__OMF, "OMF");
PRINT_FLAG (IDASIG__FILE__SREC, "SREC");
PRINT_FLAG (IDASIG__FILE__ZIP, "ZIP");
PRINT_FLAG (IDASIG__FILE__OMFLIB, "OMFLIB");
PRINT_FLAG (IDASIG__FILE__AR, "AR");
PRINT_FLAG (IDASIG__FILE__LOADER, "LOADER");
PRINT_FLAG (IDASIG__FILE__ELF, "ELF");
PRINT_FLAG (IDASIG__FILE__W32RUN, "W32RUN");
PRINT_FLAG (IDASIG__FILE__AOUT, "AOUT");
PRINT_FLAG (IDASIG__FILE__PILOT, "PILOT");
PRINT_FLAG (IDASIG__FILE__DOS_EXE, "EXE");
PRINT_FLAG (IDASIG__FILE__AIXAR, "AIXAR");
}

static void print_os_types(ut16 flags) {
PRINT_FLAG (IDASIG__OS__MSDOS, "MSDOS");
PRINT_FLAG (IDASIG__OS__WIN, "WIN");
PRINT_FLAG (IDASIG__OS__OS2, "OS2");
PRINT_FLAG (IDASIG__OS__NETWARE, "NETWARE");
PRINT_FLAG (IDASIG__OS__UNIX, "UNIX");
}

static void print_app_types(ut16 flags) {
PRINT_FLAG (IDASIG__APP__CONSOLE, "CONSOLE");
PRINT_FLAG (IDASIG__APP__GRAPHICS, "GRAPHICS");
PRINT_FLAG (IDASIG__APP__EXE, "EXE");
PRINT_FLAG (IDASIG__APP__DLL, "DLL");
PRINT_FLAG (IDASIG__APP__DRV, "DRV");
PRINT_FLAG (IDASIG__APP__SINGLE_THREADED, "SINGLE_THREADED");
PRINT_FLAG (IDASIG__APP__MULTI_THREADED, "MULTI_THREADED");
PRINT_FLAG (IDASIG__APP__16_BIT, "16_BIT");
PRINT_FLAG (IDASIG__APP__32_BIT, "32_BIT");
PRINT_FLAG (IDASIG__APP__64_BIT, "64_BIT");
}

static void print_features(ut16 flags) {
PRINT_FLAG (IDASIG__FEATURE__STARTUP, "STARTUP");
PRINT_FLAG (IDASIG__FEATURE__CTYPE_CRC, "CTYPE_CRC");
PRINT_FLAG (IDASIG__FEATURE__2BYTE_CTYPE, "2BYTE_CTYPE");
PRINT_FLAG (IDASIG__FEATURE__ALT_CTYPE_CRC, "ALT_CTYPE_CRC");
PRINT_FLAG (IDASIG__FEATURE__COMPRESSED, "COMPRESSED");
}

static void print_header(idasig_v5_t *header) {

eprintf ("version: %d\n", header->version);
eprintf ("arch:"); print_arch (header->arch); eprintf ("\n");
eprintf ("file_types:"); print_file_types (header->file_types); eprintf ("\n");
eprintf ("os_types:"); print_os_types (header->os_types); eprintf ("\n");
eprintf ("app_types:"); print_app_types (header->app_types); eprintf ("\n");
eprintf ("features:"); print_features (header->features); eprintf ("\n");
eprintf ("old_n_functions: %04x\n", header->old_n_functions);
eprintf ("crc16: %04x\n", header->crc16);
eprintf ("ctype: %s\n", header->ctype);
eprintf ("library_name_len: %d\n", header->library_name_len);
eprintf ("ctypes_crc16: %04x\n", header->ctypes_crc16);
}
#endif

static int parse_header(RBuffer *buf, idasig_v5_t *header) {
r_buf_seek (buf, 0, R_BUF_SET);
if (r_buf_read (buf, header->magic, sizeof(header->magic)) != sizeof(header->magic)) {
return false;
}
if (r_buf_read (buf, &header->version, sizeof(header->version)) != sizeof(header->version)) {
return false;
}
if (r_buf_read (buf, &header->arch, sizeof(header->arch)) != sizeof(header->arch)) {
return false;
}
if (r_buf_read (buf, (unsigned char *)&header->file_types, sizeof(header->file_types)) != sizeof(header->file_types)) {
return false;
}
if (r_buf_read (buf, (unsigned char *)&header->os_types, sizeof(header->os_types)) != sizeof(header->os_types)) {
return false;
}
if (r_buf_read (buf, (unsigned char *)&header->app_types, sizeof(header->app_types)) != sizeof(header->app_types)) {
return false;
}
if (r_buf_read (buf, (unsigned char *)&header->features, sizeof(header->features)) != sizeof(header->features)) {
return false;
}
if (r_buf_read (buf, (unsigned char *)&header->old_n_functions, sizeof(header->old_n_functions)) != sizeof(header->old_n_functions)) {
return false;
}
if (r_buf_read (buf, (unsigned char *)&header->crc16, sizeof(header->crc16)) != sizeof(header->crc16)) {
return false;
}
if (r_buf_read (buf, header->ctype, sizeof(header->ctype)) != sizeof(header->ctype)) {
return false;
}
if (r_buf_read (buf, (unsigned char *)&header->library_name_len, sizeof(header->library_name_len)) != sizeof(header->library_name_len)) {
return false;
}
if (r_buf_read (buf, (unsigned char *)&header->ctypes_crc16, sizeof(header->ctypes_crc16)) != sizeof(header->ctypes_crc16)) {
return false;
}

return true;
}

static int parse_v6_v7_header(RBuffer *buf, idasig_v6_v7_t *header) {
if (r_buf_read (buf, (unsigned char *)&header->n_functions, sizeof (header->n_functions)) != sizeof (header->n_functions)) {
return false;
}

return true;
}

static int parse_v8_v9_header(RBuffer *buf, idasig_v8_v9_t *header) {
if (r_buf_read (buf, (unsigned char *)&header->pattern_size, sizeof (header->pattern_size)) != sizeof (header->pattern_size)) {
return false;
}

return true;
}

static int parse_v10_header(RBuffer *buf, idasig_v10_t *header) {
if (r_buf_read (buf, (unsigned char *)&header->unknown, sizeof (header->unknown)) != sizeof (header->unknown)) {
return false;
}

return true;
}

static RFlirtNode *flirt_parse(const RAnal *anal, RBuffer *flirt_buf) {
ut8 *name = NULL;
ut8 *buf = NULL, *decompressed_buf = NULL;
RBuffer *r_buf = NULL;
int size, decompressed_size;
RFlirtNode *node = NULL;
RFlirtNode *ret = NULL;
idasig_v5_t *header = NULL;
idasig_v6_v7_t *v6_v7 = NULL;
idasig_v8_v9_t *v8_v9 = NULL;
idasig_v10_t *v10 = NULL;

buf_eof = false;
buf_err = false;

if (!(version = r_sign_is_flirt (flirt_buf))) {
goto exit;
}

if (version < 5 || version > 10) {
eprintf ("Unsupported flirt signature version\n");
goto exit;
}

if (!(header = R_NEW0 (idasig_v5_t))) {
goto exit;
}

parse_header (flirt_buf, header);

if (version >= 6) {
if (!(v6_v7 = R_NEW0 (idasig_v6_v7_t))) {
goto exit;
}
if (!parse_v6_v7_header (flirt_buf, v6_v7)) {
goto exit;
}

if (version >= 8) {
if (!(v8_v9 = R_NEW0 (idasig_v8_v9_t))) {
goto exit;
}
if (!parse_v8_v9_header (flirt_buf, v8_v9)) {
goto exit;
}

if (version >= 10) {
if (!(v10 = R_NEW0 (idasig_v10_t))) {
goto exit;
}
if (!parse_v10_header (flirt_buf, v10)) {
goto exit;
}
}
}
}

name = malloc (header->library_name_len + 1);
if (!name) {
goto exit;
}

if (r_buf_read (flirt_buf, name, header->library_name_len) != header->library_name_len) {
goto exit;
}

name[header->library_name_len] = '\0';


#if DEBUG
print_header (header);
header_size = r_buf_tell (flirt_buf);
#endif

size = r_buf_size (flirt_buf) - r_buf_tell (flirt_buf);
buf = malloc (size);
if (r_buf_read (flirt_buf, buf, size) != size) {
goto exit;
}

if (header->features & IDASIG__FEATURE__COMPRESSED) {
if (version == 5) {
eprintf ("Sorry we do not support the signatures version 5 compression.\n");
goto exit;
}
if (!(decompressed_buf = r_inflate (buf, size, NULL, &decompressed_size))) {
eprintf ("Decompressing failed.\n");
goto exit;
}

R_FREE (buf);
buf = decompressed_buf;
size = decompressed_size;
}

if (!(node = R_NEW0 (RFlirtNode))) {
goto exit;
}
r_buf = r_buf_new_with_pointers (buf, size, false);
#if DEBUG
r_file_dump ("sig_dump", r_buf->buf, r_buf_size (r_buf));
#endif
if (parse_tree (anal, r_buf, node)) {
ret = node;
} else {
free (node);
}
exit:
free (buf);
r_buf_free (r_buf);
free (header);
free (v6_v7);
free (v8_v9);
free (v10);
free (name);
return ret;
}

R_API int r_sign_is_flirt(RBuffer *buf) {

int ret = false;

idasig_v5_t *header = R_NEW0 (idasig_v5_t);
if (r_buf_read (buf, header->magic, sizeof(header->magic)) != sizeof(header->magic)) {
goto exit;
}

if (strncmp ((const char *) header->magic, "IDASGN", 6)) {
goto exit;
}

if (r_buf_read (buf, &header->version, sizeof(header->version)) != sizeof(header->version)) {
goto exit;
}

ret = header->version;

exit:
free (header);

return ret;
}

R_API void r_sign_flirt_dump(const RAnal *anal, const char *flirt_file) {

RBuffer *flirt_buf;
RFlirtNode *node;

if (!(flirt_buf = r_buf_new_slurp (flirt_file))) {
eprintf ("Can't open %s\n", flirt_file);
return;
}

node = flirt_parse (anal, flirt_buf);
r_buf_free (flirt_buf);
if (node) {
print_node (anal, node, -1);
node_free (node);
return;
} else {
eprintf ("We encountered an error while parsing the file. Sorry.\n");
return;
}
}

R_API void r_sign_flirt_scan(RAnal *anal, const char *flirt_file) {

RBuffer *flirt_buf;
RFlirtNode *node;

if (!(flirt_buf = r_buf_new_slurp (flirt_file))) {
eprintf ("Can't open %s\n", flirt_file);
return;
}

node = flirt_parse (anal, flirt_buf);
r_buf_free (flirt_buf);
if (node) {
if (!node_match_functions (anal, node)) {
eprintf ("Error while scanning the file %s\n", flirt_file);
}
node_free (node);
return;
} else {
eprintf ("We encountered an error while parsing the file %s. Sorry.\n", flirt_file);
return;
}
}
