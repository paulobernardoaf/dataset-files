#pragma once
#include "../util/darray.h"
#include "../util/cf-parser.h"
#include "graphics.h"
#include "shader-parser.h"
#if defined(__cplusplus)
extern "C" {
#endif
struct dstr;
enum ep_var_type {
EP_VAR_NONE,
EP_VAR_IN = EP_VAR_NONE,
EP_VAR_INOUT,
EP_VAR_OUT,
EP_VAR_UNIFORM
};
struct ep_var {
char *type, *name, *mapping;
enum ep_var_type var_type;
};
static inline void ep_var_init(struct ep_var *epv)
{
memset(epv, 0, sizeof(struct ep_var));
}
static inline void ep_var_free(struct ep_var *epv)
{
bfree(epv->type);
bfree(epv->name);
bfree(epv->mapping);
}
struct ep_param {
char *type, *name;
DARRAY(uint8_t) default_val;
DARRAY(char *) properties;
struct gs_effect_param *param;
bool is_const, is_property, is_uniform, is_texture, written;
int writeorder, array_count;
DARRAY(struct ep_param) annotations;
};
extern void ep_param_writevar(struct dstr *dst, struct darray *use_params);
static inline void ep_param_init(struct ep_param *epp, char *type, char *name,
bool is_property, bool is_const,
bool is_uniform)
{
epp->type = type;
epp->name = name;
epp->is_property = is_property;
epp->is_const = is_const;
epp->is_uniform = is_uniform;
epp->is_texture = (astrcmp_n(epp->type, "texture", 7) == 0);
epp->written = false;
epp->writeorder = false;
epp->array_count = 0;
da_init(epp->default_val);
da_init(epp->properties);
da_init(epp->annotations);
}
static inline void ep_param_free(struct ep_param *epp)
{
bfree(epp->type);
bfree(epp->name);
da_free(epp->default_val);
da_free(epp->properties);
for (size_t i = 0; i < epp->annotations.num; i++)
ep_param_free(epp->annotations.array + i);
da_free(epp->annotations);
}
struct ep_struct {
char *name;
DARRAY(struct ep_var) vars; 
bool written;
};
static inline bool ep_struct_mapped(struct ep_struct *eps)
{
if (eps->vars.num > 0)
return eps->vars.array[0].mapping != NULL;
return false;
}
static inline void ep_struct_init(struct ep_struct *eps)
{
memset(eps, 0, sizeof(struct ep_struct));
}
static inline void ep_struct_free(struct ep_struct *eps)
{
size_t i;
bfree(eps->name);
for (i = 0; i < eps->vars.num; i++)
ep_var_free(eps->vars.array + i);
da_free(eps->vars);
}
struct ep_sampler {
char *name;
DARRAY(char *) states;
DARRAY(char *) values;
bool written;
};
static inline void ep_sampler_init(struct ep_sampler *eps)
{
memset(eps, 0, sizeof(struct ep_sampler));
}
static inline void ep_sampler_free(struct ep_sampler *eps)
{
size_t i;
for (i = 0; i < eps->states.num; i++)
bfree(eps->states.array[i]);
for (i = 0; i < eps->values.num; i++)
bfree(eps->values.array[i]);
bfree(eps->name);
da_free(eps->states);
da_free(eps->values);
}
struct ep_pass {
char *name;
DARRAY(struct cf_token) vertex_program;
DARRAY(struct cf_token) fragment_program;
struct gs_effect_pass *pass;
};
static inline void ep_pass_init(struct ep_pass *epp)
{
memset(epp, 0, sizeof(struct ep_pass));
}
static inline void ep_pass_free(struct ep_pass *epp)
{
bfree(epp->name);
da_free(epp->vertex_program);
da_free(epp->fragment_program);
}
struct ep_technique {
char *name;
DARRAY(struct ep_pass) passes; 
};
static inline void ep_technique_init(struct ep_technique *ept)
{
memset(ept, 0, sizeof(struct ep_technique));
}
static inline void ep_technique_free(struct ep_technique *ept)
{
size_t i;
for (i = 0; i < ept->passes.num; i++)
ep_pass_free(ept->passes.array + i);
bfree(ept->name);
da_free(ept->passes);
}
struct ep_func {
char *name, *ret_type, *mapping;
struct dstr contents;
DARRAY(struct ep_var) param_vars;
DARRAY(const char *) func_deps;
DARRAY(const char *) struct_deps;
DARRAY(const char *) param_deps;
DARRAY(const char *) sampler_deps;
bool written;
};
static inline void ep_func_init(struct ep_func *epf, char *ret_type, char *name)
{
memset(epf, 0, sizeof(struct ep_func));
epf->name = name;
epf->ret_type = ret_type;
}
static inline void ep_func_free(struct ep_func *epf)
{
size_t i;
for (i = 0; i < epf->param_vars.num; i++)
ep_var_free(epf->param_vars.array + i);
bfree(epf->name);
bfree(epf->ret_type);
bfree(epf->mapping);
dstr_free(&epf->contents);
da_free(epf->param_vars);
da_free(epf->func_deps);
da_free(epf->struct_deps);
da_free(epf->param_deps);
da_free(epf->sampler_deps);
}
struct effect_parser {
gs_effect_t *effect;
DARRAY(struct ep_param) params;
DARRAY(struct ep_struct) structs;
DARRAY(struct ep_func) funcs;
DARRAY(struct ep_sampler) samplers;
DARRAY(struct ep_technique) techniques;
DARRAY(struct cf_lexer) files;
DARRAY(struct cf_token) tokens;
struct gs_effect_pass *cur_pass;
struct cf_parser cfp;
};
static inline void ep_init(struct effect_parser *ep)
{
da_init(ep->params);
da_init(ep->structs);
da_init(ep->funcs);
da_init(ep->samplers);
da_init(ep->techniques);
da_init(ep->files);
da_init(ep->tokens);
ep->cur_pass = NULL;
cf_parser_init(&ep->cfp);
}
extern void ep_free(struct effect_parser *ep);
extern bool ep_parse(struct effect_parser *ep, gs_effect_t *effect,
const char *effect_string, const char *file);
#if defined(__cplusplus)
}
#endif
