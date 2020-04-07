#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include <search.h>
#include <assert.h>
struct keysym
{
char xname[32];
char uname[64];
uint32_t xsym;
uint32_t usym;
};
static int cmpkey (const void *va, const void *vb)
{
const struct keysym *ka = va, *kb = vb;
if (ka->xsym > kb->xsym)
return +1;
if (ka->xsym < kb->xsym)
return -1;
return 0;
}
static void printkey (const void *node, const VISIT which, const int depth)
{
if (which != postorder && which != leaf)
return;
const struct keysym *const *psym = node, *sym = *psym;
#if 1
if (sym->xsym <= 0xff) 
return;
if (sym->xsym >= 0x1000100 && sym->xsym <= 0x110ffff) 
return;
#endif
printf ("/* XK_%-20s: %s*/\n", sym->xname, sym->uname);
printf ("{ 0x%08"PRIx32", 0x%04"PRIx32" },\n", sym->xsym, sym->usym);
(void) depth;
}
static int parse (FILE *in)
{
void *root = NULL;
char *line = NULL;
size_t buflen = 0;
ssize_t len;
while ((len = getline (&line, &buflen, in)) != -1)
{
struct keysym *sym = malloc (sizeof (*sym));
if (sym == NULL)
abort ();
int val = sscanf(line,
"#define XK_%31s 0x%"SCNx32" /*%*cU+%"SCNx32" %63[^*]",
sym->xname, &sym->xsym, &sym->usym, sym->uname);
if (val < 3)
{
free (sym);
continue;
}
if (val < 4)
sym->uname[0] = '\0';
void **psym = tsearch (sym, &root, cmpkey);
if (psym == NULL)
abort ();
if (*psym != sym)
free (sym); 
}
free (line);
puts ("/* This file is generated automatically. Do not edit! */");
puts ("/* Entries are sorted from the smallest to the largest XK */");
twalk (root, printkey);
tdestroy (root, free);
if (ferror (in))
{
perror ("Read error");
return 1;
}
return 0;
}
int main (void)
{
return -parse (stdin);
}
