#pragma once
#include "lexer.h"
#if defined(__cplusplus)
extern "C" {
#endif
EXPORT char *cf_literal_to_str(const char *literal, size_t count);
enum cf_token_type {
CFTOKEN_NONE,
CFTOKEN_NAME,
CFTOKEN_NUM,
CFTOKEN_SPACETAB,
CFTOKEN_NEWLINE,
CFTOKEN_STRING,
CFTOKEN_OTHER
};
struct cf_token {
const struct cf_lexer *lex;
struct strref str;
struct strref unmerged_str;
enum cf_token_type type;
};
static inline void cf_token_clear(struct cf_token *t)
{
memset(t, 0, sizeof(struct cf_token));
}
static inline void cf_token_copy(struct cf_token *dst,
const struct cf_token *src)
{
memcpy(dst, src, sizeof(struct cf_token));
}
static inline void cf_token_add(struct cf_token *dst,
const struct cf_token *add)
{
strref_add(&dst->str, &add->str);
strref_add(&dst->unmerged_str, &add->unmerged_str);
}
struct cf_lexer {
char *file;
struct lexer base_lexer;
char *reformatted, *write_offset;
DARRAY(struct cf_token) tokens;
bool unexpected_eof; 
};
EXPORT void cf_lexer_init(struct cf_lexer *lex);
EXPORT void cf_lexer_free(struct cf_lexer *lex);
static inline struct cf_token *cf_lexer_get_tokens(struct cf_lexer *lex)
{
return lex->tokens.array;
}
EXPORT bool cf_lexer_lex(struct cf_lexer *lex, const char *str,
const char *file);
struct cf_def {
struct cf_token name;
DARRAY(struct cf_token) params;
DARRAY(struct cf_token) tokens;
bool macro;
};
static inline void cf_def_init(struct cf_def *cfd)
{
cf_token_clear(&cfd->name);
da_init(cfd->params);
da_init(cfd->tokens);
cfd->macro = false;
}
static inline void cf_def_addparam(struct cf_def *cfd, struct cf_token *param)
{
da_push_back(cfd->params, param);
}
static inline void cf_def_addtoken(struct cf_def *cfd, struct cf_token *token)
{
da_push_back(cfd->tokens, token);
}
static inline struct cf_token *cf_def_getparam(const struct cf_def *cfd,
size_t idx)
{
return cfd->params.array + idx;
}
static inline void cf_def_free(struct cf_def *cfd)
{
cf_token_clear(&cfd->name);
da_free(cfd->params);
da_free(cfd->tokens);
}
struct cf_preprocessor {
struct cf_lexer *lex;
struct error_data *ed;
DARRAY(struct cf_def) defines;
DARRAY(char *) sys_include_dirs;
DARRAY(struct cf_lexer) dependencies;
DARRAY(struct cf_token) tokens;
bool ignore_state;
};
EXPORT void cf_preprocessor_init(struct cf_preprocessor *pp);
EXPORT void cf_preprocessor_free(struct cf_preprocessor *pp);
EXPORT bool cf_preprocess(struct cf_preprocessor *pp, struct cf_lexer *lex,
struct error_data *ed);
static inline void
cf_preprocessor_add_sys_include_dir(struct cf_preprocessor *pp,
const char *include_dir)
{
if (include_dir)
da_push_back(pp->sys_include_dirs, bstrdup(include_dir));
}
EXPORT void cf_preprocessor_add_def(struct cf_preprocessor *pp,
struct cf_def *def);
EXPORT void cf_preprocessor_remove_def(struct cf_preprocessor *pp,
const char *def_name);
static inline struct cf_token *
cf_preprocessor_get_tokens(struct cf_preprocessor *pp)
{
return pp->tokens.array;
}
#if defined(__cplusplus)
}
#endif
