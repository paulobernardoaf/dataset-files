#include <r_types.h>
#include <r_flag.h>
#include <r_anal.h>
#if defined(__cplusplus)
extern "C" {
#endif
R_LIB_VERSION_HEADER(r_parse);
typedef RList* (*RAnalVarList)(RAnal *anal, RAnalFunction *fcn, int kind);
typedef struct r_parse_t {
void *user;
RSpace *flagspace;
RSpace *notin_flagspace;
bool pseudo;
bool regsub; 
bool relsub; 
bool tailsub; 
bool localvar_only; 
ut64 relsub_addr;
int maxflagnamelen;
int minval;
char *retleave_asm;
struct r_parse_plugin_t *cur;
RList *parsers;
RAnalVarList varlist;
int (*get_ptr_at)(void *user, RAnalVar *var, ut64 addr);
char* (*get_op_ireg)(void *user, ut64 addr);
RAnalBind analb;
RFlagGetAtAddr flag_get; 
RAnalLabelAt label_get;
} RParse;
typedef struct r_parse_plugin_t {
char *name;
char *desc;
bool (*init)(RParse *p, void *user);
int (*fini)(RParse *p, void *user);
int (*parse)(RParse *p, const char *data, char *str);
bool (*assemble)(RParse *p, char *data, char *str);
int (*filter)(RParse *p, ut64 addr, RFlag *f, char *data, char *str, int len, bool big_endian);
bool (*varsub)(RParse *p, RAnalFunction *f, ut64 addr, int oplen, char *data, char *str, int len);
int (*replace)(int argc, const char *argv[], char *newstr);
} RParsePlugin;
#if defined(R_API)
R_API struct r_parse_t *r_parse_new(void);
R_API void r_parse_free(RParse *p);
R_API void r_parse_set_user_ptr(RParse *p, void *user);
R_API bool r_parse_add(RParse *p, RParsePlugin *foo);
R_API bool r_parse_use(RParse *p, const char *name);
R_API bool r_parse_parse(RParse *p, const char *data, char *str);
R_API bool r_parse_assemble(RParse *p, char *data, char *str); 
R_API bool r_parse_filter(RParse *p, ut64 addr, RFlag *f, RAnalHint *hint, char *data, char *str, int len, bool big_endian);
R_API bool r_parse_varsub(RParse *p, RAnalFunction *f, ut64 addr, int oplen, char *data, char *str, int len);
R_API char *r_parse_immtrim(char *opstr);
R_API char *r_parse_c_string(RAnal *anal, const char *code, char **error_msg);
R_API char *r_parse_c_file(RAnal *anal, const char *path, const char *dir, char **error_msg);
R_API void r_parse_c_reset(RParse *p);
typedef struct r_parse_ctype_t RParseCType;
typedef enum {
R_PARSE_CTYPE_TYPE_KIND_IDENTIFIER,
R_PARSE_CTYPE_TYPE_KIND_POINTER,
R_PARSE_CTYPE_TYPE_KIND_ARRAY
} RParseCTypeTypeKind;
typedef enum {
R_PARSE_CTYPE_IDENTIFIER_KIND_UNSPECIFIED,
R_PARSE_CTYPE_IDENTIFIER_KIND_STRUCT,
R_PARSE_CTYPE_IDENTIFIER_KIND_UNION,
R_PARSE_CTYPE_IDENTIFIER_KIND_ENUM
} RParseCTypeTypeIdentifierKind;
typedef struct r_parse_ctype_type_t RParseCTypeType;
struct r_parse_ctype_type_t {
RParseCTypeTypeKind kind;
union {
struct {
RParseCTypeTypeIdentifierKind kind;
char *name;
bool is_const;
} identifier;
struct {
RParseCTypeType *type;
bool is_const;
} pointer;
struct {
RParseCTypeType *type;
ut64 count;
} array;
};
};
R_API RParseCType *r_parse_ctype_new(void);
R_API void r_parse_ctype_free(RParseCType *ctype);
R_API RParseCTypeType *r_parse_ctype_parse(RParseCType *ctype, const char *str, char **error);
R_API void r_parse_ctype_type_free(RParseCTypeType *type);
extern RParsePlugin r_parse_plugin_6502_pseudo;
extern RParsePlugin r_parse_plugin_arm_pseudo;
extern RParsePlugin r_parse_plugin_att2intel;
extern RParsePlugin r_parse_plugin_avr_pseudo;
extern RParsePlugin r_parse_plugin_chip8_pseudo;
extern RParsePlugin r_parse_plugin_dalvik_pseudo;
extern RParsePlugin r_parse_plugin_dummy;
extern RParsePlugin r_parse_plugin_m68k_pseudo;
extern RParsePlugin r_parse_plugin_mips_pseudo;
extern RParsePlugin r_parse_plugin_ppc_pseudo;
extern RParsePlugin r_parse_plugin_sh_pseudo;
extern RParsePlugin r_parse_plugin_wasm_pseudo;
extern RParsePlugin r_parse_plugin_x86_pseudo;
extern RParsePlugin r_parse_plugin_z80_pseudo;
#endif
#if defined(__cplusplus)
}
#endif
