#pragma once
#include "effect-parser.h"
#include "graphics.h"
#if defined(__cplusplus)
extern "C" {
#endif
enum effect_section {
EFFECT_PARAM,
EFFECT_TECHNIQUE,
EFFECT_SAMPLER,
EFFECT_PASS,
EFFECT_ANNOTATION
};
struct gs_effect_param {
char *name;
enum effect_section section;
enum gs_shader_param_type type;
bool changed;
DARRAY(uint8_t) cur_val;
DARRAY(uint8_t) default_val;
gs_effect_t *effect;
gs_samplerstate_t *next_sampler;
DARRAY(struct gs_effect_param) annotations;
};
static inline void effect_param_init(struct gs_effect_param *param)
{
memset(param, 0, sizeof(struct gs_effect_param));
da_init(param->annotations);
}
static inline void effect_param_free(struct gs_effect_param *param)
{
bfree(param->name);
da_free(param->cur_val);
da_free(param->default_val);
size_t i;
for (i = 0; i < param->annotations.num; i++)
effect_param_free(param->annotations.array + i);
da_free(param->annotations);
}
EXPORT void effect_param_parse_property(gs_eparam_t *param,
const char *property);
struct pass_shaderparam {
struct gs_effect_param *eparam;
gs_sparam_t *sparam;
};
struct gs_effect_pass {
char *name;
enum effect_section section;
gs_shader_t *vertshader;
gs_shader_t *pixelshader;
DARRAY(struct pass_shaderparam) vertshader_params;
DARRAY(struct pass_shaderparam) pixelshader_params;
};
static inline void effect_pass_init(struct gs_effect_pass *pass)
{
memset(pass, 0, sizeof(struct gs_effect_pass));
}
static inline void effect_pass_free(struct gs_effect_pass *pass)
{
bfree(pass->name);
da_free(pass->vertshader_params);
da_free(pass->pixelshader_params);
gs_shader_destroy(pass->vertshader);
gs_shader_destroy(pass->pixelshader);
}
struct gs_effect_technique {
char *name;
enum effect_section section;
struct gs_effect *effect;
DARRAY(struct gs_effect_pass) passes;
};
static inline void effect_technique_init(struct gs_effect_technique *t)
{
memset(t, 0, sizeof(struct gs_effect_technique));
}
static inline void effect_technique_free(struct gs_effect_technique *t)
{
size_t i;
for (i = 0; i < t->passes.num; i++)
effect_pass_free(t->passes.array + i);
da_free(t->passes);
bfree(t->name);
}
struct gs_effect {
bool processing;
bool cached;
char *effect_path, *effect_dir;
DARRAY(struct gs_effect_param) params;
DARRAY(struct gs_effect_technique) techniques;
struct gs_effect_technique *cur_technique;
struct gs_effect_pass *cur_pass;
gs_eparam_t *view_proj, *world, *scale;
graphics_t *graphics;
struct gs_effect *next;
size_t loop_pass;
bool looping;
};
static inline void effect_init(gs_effect_t *effect)
{
memset(effect, 0, sizeof(struct gs_effect));
}
static inline void effect_free(gs_effect_t *effect)
{
size_t i;
for (i = 0; i < effect->params.num; i++)
effect_param_free(effect->params.array + i);
for (i = 0; i < effect->techniques.num; i++)
effect_technique_free(effect->techniques.array + i);
da_free(effect->params);
da_free(effect->techniques);
bfree(effect->effect_path);
bfree(effect->effect_dir);
effect->effect_path = NULL;
effect->effect_dir = NULL;
}
EXPORT void effect_upload_params(gs_effect_t *effect, bool changed_only);
EXPORT void effect_upload_shader_params(gs_effect_t *effect,
gs_shader_t *shader,
struct darray *pass_params,
bool changed_only);
#if defined(__cplusplus)
}
#endif
