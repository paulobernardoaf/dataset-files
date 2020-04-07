#include <r_util.h>
#include "arch.h"
static ut64 parse_size(char *s, char **end) {
if (*s == '.') {
return strtoul (s + 1, end, 10);
}
char *has_dot = strchr (s, '.');
if (has_dot) {
*has_dot++ = 0;
ut64 a = strtoul (s, end, 0) << 3;
ut64 b = strtoul (has_dot, end, 0);
return a + b;
}
return strtoul (s, end, 0) << 3;
}
gdb_reg_t *parse_def(char **tok) {
char *end;
gdb_reg_t *r = R_NEW0 (gdb_reg_t);
if (!r) {
return NULL;
}
strcpy (r->name, tok[1]);
r->size = parse_size (tok[2], &end);
if (*end != '\0' || !r->size) {
free (r);
return NULL;
}
if (!strcmp (tok[3], "?")) {
free (r);
return NULL;
} else {
r->offset = parse_size (tok[3], &end);
}
return r;
}
#define PARSER_MAX_TOKENS 8
gdb_reg_t *arch_parse_reg_profile(const char * reg_profile) {
char *tok[PARSER_MAX_TOKENS];
char tmp[128];
int i, j, l;
const char *p = reg_profile;
RList *gdb_regs_list = r_list_newf (free);
RListIter *iter;
gdb_reg_t *reg;
l = 0;
do {
l++;
if (*p == '#') {
const char *q = p;
while (*q != '\n') {
q++;
}
p = q;
continue;
}
j = 0;
while (*p) {
while (*p == ' ' || *p == '\t') {
p++;
}
if (*p == '\n') {
break;
}
if (*p == '#') {
for (i = 0; *p != '\n'; p++) {
if (i < sizeof (tmp) - 1) {
tmp[i++] = *p;
}
}
} else {
for (i = 0; isgraph ((const unsigned char)*p) && i < sizeof (tmp) - 1;) {
tmp[i++] = *p++;
}
}
tmp[i] = '\0';
if (j > PARSER_MAX_TOKENS - 1) {
break;
}
tok[j++] = strdup (tmp);
}
if (j) {
char *first = tok[0];
if (*first != '=') {
reg = parse_def (tok);
if (!reg) {
eprintf ("gdb_regs: Parse error @ line %d\n", l);
r_list_free (gdb_regs_list);
return NULL;
}
r_list_append (gdb_regs_list, reg);
}
for (i = 0; i < j; i++) {
free (tok[i]);
}
}
} while (*p++);
gdb_reg_t *gdb_regs = malloc (r_list_length (gdb_regs_list) + 1 * sizeof (gdb_reg_t));
if (!gdb_regs) {
return NULL;
}
i = 0;
r_list_foreach (gdb_regs_list, iter, reg) {
memcpy (reg, gdb_regs + i, sizeof (gdb_reg_t));
i++;
}
memset (gdb_regs + i, 0, sizeof (gdb_reg_t));
r_list_free (gdb_regs_list);
return gdb_regs;
}
