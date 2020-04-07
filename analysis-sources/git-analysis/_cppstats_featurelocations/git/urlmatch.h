#if !defined(URL_MATCH_H)
#define URL_MATCH_H

#include "string-list.h"

struct url_info {

char *url;

const char *err;



size_t url_len; 
size_t scheme_len; 
size_t user_off; 
size_t user_len; 

size_t passwd_off; 
size_t passwd_len; 

size_t host_off; 
size_t host_len; 

size_t port_off; 
size_t port_len; 


size_t path_off; 


size_t path_len; 

};

char *url_normalize(const char *, struct url_info *);

struct urlmatch_item {
size_t hostmatch_len;
size_t pathmatch_len;
char user_matched;
};

struct urlmatch_config {
struct string_list vars;
struct url_info url;
const char *section;
const char *key;

void *cb;
int (*collect_fn)(const char *var, const char *value, void *cb);
int (*cascade_fn)(const char *var, const char *value, void *cb);








int (*select_fn)(const struct urlmatch_item *found, const struct urlmatch_item *existing);
};

int urlmatch_config_entry(const char *var, const char *value, void *cb);

#endif 
