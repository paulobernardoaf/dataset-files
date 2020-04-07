






























#include <r_userconf.h>

#if !USE_LIB_MAGIC

#include "file.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#define SZOF(a) (sizeof(a) / sizeof(a[0]))

#if !defined(COMPILE_ONLY)
void file_mdump(struct r_magic *m) {
static const char optyp[] = { FILE_OPS };

(void) eprintf ("[%u", m->lineno);
(void) eprintf ("%.*s %u", m->cont_level & 7, ">>>>>>>>", m->offset);

if (m->flag & INDIR) {
(void) eprintf ("(%s,",

(m->in_type < file_nnames) ?
magic_file_names[m->in_type] : "*bad*");
if (m->in_op & FILE_OPINVERSE)
(void) fputc('~', stderr);
(void) eprintf ("%c%u),",
((m->in_op & FILE_OPS_MASK) < SZOF(optyp)) ?
optyp[m->in_op & FILE_OPS_MASK] : '?',
m->in_offset);
}
(void) eprintf (" %s%s", (m->flag & UNSIGNED) ? "u" : "",

(m->type < file_nnames) ? magic_file_names[m->type] : "*bad*");
if (m->mask_op & FILE_OPINVERSE)
(void) fputc('~', stderr);

if (MAGIC_IS_STRING(m->type)) {
if (m->str_flags) {
(void) fputc('/', stderr);
if (m->str_flags & STRING_COMPACT_BLANK)
(void) fputc(CHAR_COMPACT_BLANK, stderr);
if (m->str_flags & STRING_COMPACT_OPTIONAL_BLANK)
(void) fputc(CHAR_COMPACT_OPTIONAL_BLANK,
stderr);
if (m->str_flags & STRING_IGNORE_LOWERCASE)
(void) fputc(CHAR_IGNORE_LOWERCASE, stderr);
if (m->str_flags & STRING_IGNORE_UPPERCASE)
(void) fputc(CHAR_IGNORE_UPPERCASE, stderr);
if (m->str_flags & REGEX_OFFSET_START)
(void) fputc(CHAR_REGEX_OFFSET_START, stderr);
}
if (m->str_range)
(void) eprintf ("/%u", m->str_range);
}
else {
if ((m->mask_op & FILE_OPS_MASK) < SZOF(optyp))
(void) fputc(optyp[m->mask_op & FILE_OPS_MASK], stderr);
else
(void) fputc('?', stderr);

if (m->num_mask)
(void) eprintf ("%08"PFMT64x, (ut64)m->num_mask);
}
(void) eprintf (",%c", m->reln);

if (m->reln != 'x') {
switch (m->type) {
case FILE_BYTE:
case FILE_SHORT:
case FILE_LONG:
case FILE_LESHORT:
case FILE_LELONG:
case FILE_MELONG:
case FILE_BESHORT:
case FILE_BELONG:
(void) eprintf ("%d", m->value.l);
break;
case FILE_BEQUAD:
case FILE_LEQUAD:
case FILE_QUAD:
(void) eprintf ("%"PFMT64d, (ut64)m->value.q);
break;
case FILE_PSTRING:
case FILE_STRING:
case FILE_REGEX:
case FILE_BESTRING16:
case FILE_LESTRING16:
case FILE_SEARCH:
file_showstr(stderr, m->value.s, (size_t)m->vallen);
break;
case FILE_DATE:
case FILE_LEDATE:
case FILE_BEDATE:
case FILE_MEDATE:
(void)eprintf ("%s,",
file_fmttime(m->value.l, 1));
break;
case FILE_LDATE:
case FILE_LELDATE:
case FILE_BELDATE:
case FILE_MELDATE:
(void)eprintf ("%s,",
file_fmttime(m->value.l, 0));
break;
case FILE_QDATE:
case FILE_LEQDATE:
case FILE_BEQDATE:
(void)eprintf ("%s,",
file_fmttime((ut32)m->value.q, 1));
break;
case FILE_QLDATE:
case FILE_LEQLDATE:
case FILE_BEQLDATE:
(void)eprintf ("%s,",
file_fmttime((ut32)m->value.q, 0));
break;
case FILE_FLOAT:
case FILE_BEFLOAT:
case FILE_LEFLOAT:
(void) eprintf ("%G", m->value.f);
break;
case FILE_DOUBLE:
case FILE_BEDOUBLE:
case FILE_LEDOUBLE:
(void) eprintf ("%G", m->value.d);
break;
case FILE_DEFAULT:

break;
default:
(void) fputs("*bad*", stderr);
break;
}
}
(void) eprintf (",\"%s\"]\n", m->desc);
}
#endif


void file_magwarn(struct r_magic_set *ms, const char *f, ...) {
va_list va;


(void) fflush(stdout);

if (ms->file)
(void) eprintf ("%s, %lu: ", ms->file,
(unsigned long)ms->line);
(void) eprintf ("Warning: ");
va_start(va, f);
(void) vfprintf (stderr, f, va);
va_end(va);
(void) fputc('\n', stderr);
}

const char *file_fmttime(ut32 v, int local) {
char *pp;
time_t t = (time_t)v;
struct tm *tm;
struct tm timestruct;

if (local) {
pp = ctime(&t);
} else {
#if !defined(HAVE_DAYLIGHT)
static int daylight = 0;
#if defined(HAVE_TM_ISDST)
static time_t now = (time_t)0;

if (now == (time_t)0) {
struct tm *tm1;
(void)time(&now);
tm1 = localtime(&now);
if (!tm1)
return "*Invalid time*";
daylight = tm1->tm_isdst;
}
#endif 
#endif 
if (daylight)
t += 3600;
tm = gmtime_r(&t, &timestruct);
if (!tm)
return "*Invalid time*";
pp = asctime (tm);
}

pp[strcspn (pp, "\n")] = '\0';
return pp;
}
#endif
