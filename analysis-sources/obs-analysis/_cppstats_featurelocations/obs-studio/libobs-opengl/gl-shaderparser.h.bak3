
















#pragma once







#include <util/dstr.h>
#include <graphics/shader-parser.h>

struct gl_parser_attrib {
struct dstr name;
const char *mapping;
bool input;
};

static inline void gl_parser_attrib_init(struct gl_parser_attrib *attr)
{
memset(attr, 0, sizeof(struct gl_parser_attrib));
}

static inline void gl_parser_attrib_free(struct gl_parser_attrib *attr)
{
dstr_free(&attr->name);
}

struct gl_shader_parser {
enum gs_shader_type type;
const char *input_prefix;
const char *output_prefix;
struct shader_parser parser;
struct dstr gl_string;

DARRAY(uint32_t) texture_samplers;
DARRAY(struct gl_parser_attrib) attribs;
};

static inline void gl_shader_parser_init(struct gl_shader_parser *glsp,
enum gs_shader_type type)
{
glsp->type = type;

if (type == GS_SHADER_VERTEX) {
glsp->input_prefix = "_input_attrib";
glsp->output_prefix = "_vertex_shader_attrib";
} else if (type == GS_SHADER_PIXEL) {
glsp->input_prefix = "_vertex_shader_attrib";
glsp->output_prefix = "_pixel_shader_attrib";
}

shader_parser_init(&glsp->parser);
dstr_init(&glsp->gl_string);
da_init(glsp->texture_samplers);
da_init(glsp->attribs);
}

static inline void gl_shader_parser_free(struct gl_shader_parser *glsp)
{
size_t i;
for (i = 0; i < glsp->attribs.num; i++)
gl_parser_attrib_free(glsp->attribs.array + i);

da_free(glsp->attribs);
da_free(glsp->texture_samplers);
dstr_free(&glsp->gl_string);
shader_parser_free(&glsp->parser);
}

extern bool gl_shader_parse(struct gl_shader_parser *glsp,
const char *shader_str, const char *file);
