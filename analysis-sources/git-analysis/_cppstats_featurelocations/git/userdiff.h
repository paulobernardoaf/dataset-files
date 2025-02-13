#if !defined(USERDIFF_H)
#define USERDIFF_H

#include "notes-cache.h"

struct index_state;
struct repository;

struct userdiff_funcname {
const char *pattern;
int cflags;
};

struct userdiff_driver {
const char *name;
const char *external;
int binary;
struct userdiff_funcname funcname;
const char *word_regex;
const char *textconv;
struct notes_cache *textconv_cache;
int textconv_want_cache;
};

int userdiff_config(const char *k, const char *v);
struct userdiff_driver *userdiff_find_by_name(const char *name);
struct userdiff_driver *userdiff_find_by_path(struct index_state *istate,
const char *path);





struct userdiff_driver *userdiff_get_textconv(struct repository *r,
struct userdiff_driver *driver);

#endif 
