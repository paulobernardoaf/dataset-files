

































#include "cache.h"

#include "kwset.h"
#include "compat/obstack.h"

#define NCHAR (UCHAR_MAX + 1)

static void *obstack_chunk_alloc(long size)
{
if (size < 0)
BUG("Cannot allocate a negative amount: %ld", size);
return xmalloc(size);
}
#define obstack_chunk_free free

#define U(c) ((unsigned char) (c))


struct tree
{
struct tree *llink; 
struct tree *rlink; 
struct trie *trie; 
unsigned char label; 
char balance; 
};


struct trie
{
unsigned int accepting; 
struct tree *links; 
struct trie *parent; 
struct trie *next; 
struct trie *fail; 
int depth; 
int shift; 
int maxshift; 
};


struct kwset
{
struct obstack obstack; 
int words; 
struct trie *trie; 
int mind; 
int maxd; 
unsigned char delta[NCHAR]; 
struct trie *next[NCHAR]; 
char *target; 
int mind2; 
unsigned char const *trans; 
};



kwset_t
kwsalloc (unsigned char const *trans)
{
struct kwset *kwset;

kwset = (struct kwset *) xmalloc(sizeof (struct kwset));

obstack_init(&kwset->obstack);
kwset->words = 0;
kwset->trie
= (struct trie *) obstack_alloc(&kwset->obstack, sizeof (struct trie));
if (!kwset->trie)
{
kwsfree((kwset_t) kwset);
return NULL;
}
kwset->trie->accepting = 0;
kwset->trie->links = NULL;
kwset->trie->parent = NULL;
kwset->trie->next = NULL;
kwset->trie->fail = NULL;
kwset->trie->depth = 0;
kwset->trie->shift = 0;
kwset->mind = INT_MAX;
kwset->maxd = -1;
kwset->target = NULL;
kwset->trans = trans;

return (kwset_t) kwset;
}



#define DEPTH_SIZE (CHAR_BIT + CHAR_BIT/2)



const char *
kwsincr (kwset_t kws, char const *text, size_t len)
{
struct kwset *kwset;
register struct trie *trie;
register unsigned char label;
register struct tree *link;
register int depth;
struct tree *links[DEPTH_SIZE];
enum { L, R } dirs[DEPTH_SIZE];
struct tree *t, *r, *l, *rl, *lr;

kwset = (struct kwset *) kws;
trie = kwset->trie;
text += len;



while (len--)
{
label = kwset->trans ? kwset->trans[U(*--text)] : *--text;




link = trie->links;
links[0] = (struct tree *) &trie->links;
dirs[0] = L;
depth = 1;

while (link && label != link->label)
{
links[depth] = link;
if (label < link->label)
dirs[depth++] = L, link = link->llink;
else
dirs[depth++] = R, link = link->rlink;
}




if (!link)
{
link = (struct tree *) obstack_alloc(&kwset->obstack,
sizeof (struct tree));
if (!link)
return "memory exhausted";
link->llink = NULL;
link->rlink = NULL;
link->trie = (struct trie *) obstack_alloc(&kwset->obstack,
sizeof (struct trie));
if (!link->trie)
{
obstack_free(&kwset->obstack, link);
return "memory exhausted";
}
link->trie->accepting = 0;
link->trie->links = NULL;
link->trie->parent = trie;
link->trie->next = NULL;
link->trie->fail = NULL;
link->trie->depth = trie->depth + 1;
link->trie->shift = 0;
link->label = label;
link->balance = 0;


if (dirs[--depth] == L)
links[depth]->llink = link;
else
links[depth]->rlink = link;


while (depth && !links[depth]->balance)
{
if (dirs[depth] == L)
--links[depth]->balance;
else
++links[depth]->balance;
--depth;
}


if (depth && ((dirs[depth] == L && --links[depth]->balance)
|| (dirs[depth] == R && ++links[depth]->balance)))
{
switch (links[depth]->balance)
{
case (char) -2:
switch (dirs[depth + 1])
{
case L:
r = links[depth], t = r->llink, rl = t->rlink;
t->rlink = r, r->llink = rl;
t->balance = r->balance = 0;
break;
case R:
r = links[depth], l = r->llink, t = l->rlink;
rl = t->rlink, lr = t->llink;
t->llink = l, l->rlink = lr, t->rlink = r, r->llink = rl;
l->balance = t->balance != 1 ? 0 : -1;
r->balance = t->balance != (char) -1 ? 0 : 1;
t->balance = 0;
break;
default:
abort ();
}
break;
case 2:
switch (dirs[depth + 1])
{
case R:
l = links[depth], t = l->rlink, lr = t->llink;
t->llink = l, l->rlink = lr;
t->balance = l->balance = 0;
break;
case L:
l = links[depth], r = l->rlink, t = r->llink;
lr = t->llink, rl = t->rlink;
t->llink = l, l->rlink = lr, t->rlink = r, r->llink = rl;
l->balance = t->balance != 1 ? 0 : -1;
r->balance = t->balance != (char) -1 ? 0 : 1;
t->balance = 0;
break;
default:
abort ();
}
break;
default:
abort ();
}

if (dirs[depth - 1] == L)
links[depth - 1]->llink = t;
else
links[depth - 1]->rlink = t;
}
}

trie = link->trie;
}



if (!trie->accepting)
trie->accepting = 1 + 2 * kwset->words;
++kwset->words;


if (trie->depth < kwset->mind)
kwset->mind = trie->depth;
if (trie->depth > kwset->maxd)
kwset->maxd = trie->depth;

return NULL;
}



static void
enqueue (struct tree *tree, struct trie **last)
{
if (!tree)
return;
enqueue(tree->llink, last);
enqueue(tree->rlink, last);
(*last) = (*last)->next = tree->trie;
}




static void
treefails (register struct tree const *tree, struct trie const *fail,
struct trie *recourse)
{
register struct tree *link;

if (!tree)
return;

treefails(tree->llink, fail, recourse);
treefails(tree->rlink, fail, recourse);



while (fail)
{
link = fail->links;
while (link && tree->label != link->label)
if (tree->label < link->label)
link = link->llink;
else
link = link->rlink;
if (link)
{
tree->trie->fail = link->trie;
return;
}
fail = fail->fail;
}

tree->trie->fail = recourse;
}



static void
treedelta (register struct tree const *tree,
register unsigned int depth,
unsigned char delta[])
{
if (!tree)
return;
treedelta(tree->llink, depth, delta);
treedelta(tree->rlink, depth, delta);
if (depth < delta[tree->label])
delta[tree->label] = depth;
}


static int
hasevery (register struct tree const *a, register struct tree const *b)
{
if (!b)
return 1;
if (!hasevery(a, b->llink))
return 0;
if (!hasevery(a, b->rlink))
return 0;
while (a && b->label != a->label)
if (b->label < a->label)
a = a->llink;
else
a = a->rlink;
return !!a;
}



static void
treenext (struct tree const *tree, struct trie *next[])
{
if (!tree)
return;
treenext(tree->llink, next);
treenext(tree->rlink, next);
next[tree->label] = tree->trie;
}



const char *
kwsprep (kwset_t kws)
{
register struct kwset *kwset;
register int i;
register struct trie *curr;
register unsigned char const *trans;
unsigned char delta[NCHAR];

kwset = (struct kwset *) kws;




memset(delta, kwset->mind < UCHAR_MAX ? kwset->mind : UCHAR_MAX, NCHAR);



if (kwset->words == 1 && kwset->trans == NULL)
{
char c;


kwset->target = obstack_alloc(&kwset->obstack, kwset->mind);
if (!kwset->target)
return "memory exhausted";
for (i = kwset->mind - 1, curr = kwset->trie; i >= 0; --i)
{
kwset->target[i] = curr->links->label;
curr = curr->links->trie;
}

for (i = 0; i < kwset->mind; ++i)
delta[U(kwset->target[i])] = kwset->mind - (i + 1);


c = kwset->target[kwset->mind - 1];
for (i = kwset->mind - 2; i >= 0; --i)
if (kwset->target[i] == c)
break;
kwset->mind2 = kwset->mind - (i + 1);
}
else
{
register struct trie *fail;
struct trie *last, *next[NCHAR];



for (curr = last = kwset->trie; curr; curr = curr->next)
{

enqueue(curr->links, &last);

curr->shift = kwset->mind;
curr->maxshift = kwset->mind;


treedelta(curr->links, curr->depth, delta);


treefails(curr->links, curr->fail, kwset->trie);



for (fail = curr->fail; fail; fail = fail->fail)
{



if (!hasevery(fail->links, curr->links))
if (curr->depth - fail->depth < fail->shift)
fail->shift = curr->depth - fail->depth;




if (curr->accepting && fail->maxshift > curr->depth - fail->depth)
fail->maxshift = curr->depth - fail->depth;
}
}



for (curr = kwset->trie->next; curr; curr = curr->next)
{
if (curr->maxshift > curr->parent->maxshift)
curr->maxshift = curr->parent->maxshift;
if (curr->shift > curr->maxshift)
curr->shift = curr->maxshift;
}



for (i = 0; i < NCHAR; ++i)
next[i] = NULL;
treenext(kwset->trie->links, next);

if ((trans = kwset->trans) != NULL)
for (i = 0; i < NCHAR; ++i)
kwset->next[i] = next[U(trans[i])];
else
COPY_ARRAY(kwset->next, next, NCHAR);
}


if ((trans = kwset->trans) != NULL)
for (i = 0; i < NCHAR; ++i)
kwset->delta[i] = delta[U(trans[i])];
else
memcpy(kwset->delta, delta, NCHAR);

return NULL;
}


static size_t
bmexec (kwset_t kws, char const *text, size_t size)
{
struct kwset const *kwset;
register unsigned char const *d1;
register char const *ep, *sp, *tp;
register int d, gc, i, len, md2;

kwset = (struct kwset const *) kws;
len = kwset->mind;

if (len == 0)
return 0;
if (len > size)
return -1;
if (len == 1)
{
tp = memchr (text, kwset->target[0], size);
return tp ? tp - text : -1;
}

d1 = kwset->delta;
sp = kwset->target + len;
gc = U(sp[-2]);
md2 = kwset->mind2;
tp = text + len;


if (size > 12 * len)

for (ep = text + size - 11 * len;;)
{
while (tp <= ep)
{
d = d1[U(tp[-1])], tp += d;
d = d1[U(tp[-1])], tp += d;
if (d == 0)
goto found;
d = d1[U(tp[-1])], tp += d;
d = d1[U(tp[-1])], tp += d;
d = d1[U(tp[-1])], tp += d;
if (d == 0)
goto found;
d = d1[U(tp[-1])], tp += d;
d = d1[U(tp[-1])], tp += d;
d = d1[U(tp[-1])], tp += d;
if (d == 0)
goto found;
d = d1[U(tp[-1])], tp += d;
d = d1[U(tp[-1])], tp += d;
}
break;
found:
if (U(tp[-2]) == gc)
{
for (i = 3; i <= len && U(tp[-i]) == U(sp[-i]); ++i)
;
if (i > len)
return tp - len - text;
}
tp += md2;
}



ep = text + size;
d = d1[U(tp[-1])];
while (d <= ep - tp)
{
d = d1[U((tp += d)[-1])];
if (d != 0)
continue;
if (U(tp[-2]) == gc)
{
for (i = 3; i <= len && U(tp[-i]) == U(sp[-i]); ++i)
;
if (i > len)
return tp - len - text;
}
d = md2;
}

return -1;
}


static size_t
cwexec (kwset_t kws, char const *text, size_t len, struct kwsmatch *kwsmatch)
{
struct kwset const *kwset;
struct trie * const *next;
struct trie const *trie;
struct trie const *accept;
char const *beg, *lim, *mch, *lmch;
register unsigned char c;
register unsigned char const *delta;
register int d;
register char const *end, *qlim;
register struct tree const *tree;
register unsigned char const *trans;

accept = NULL;


kwset = (struct kwset *) kws;
if (len < kwset->mind)
return -1;
next = kwset->next;
delta = kwset->delta;
trans = kwset->trans;
lim = text + len;
end = text;
if ((d = kwset->mind) != 0)
mch = NULL;
else
{
mch = text, accept = kwset->trie;
goto match;
}

if (len >= 4 * kwset->mind)
qlim = lim - 4 * kwset->mind;
else
qlim = NULL;

while (lim - end >= d)
{
if (qlim && end <= qlim)
{
end += d - 1;
while ((d = delta[c = *end]) && end < qlim)
{
end += d;
end += delta[U(*end)];
end += delta[U(*end)];
}
++end;
}
else
d = delta[c = (end += d)[-1]];
if (d)
continue;
beg = end - 1;
trie = next[c];
if (trie->accepting)
{
mch = beg;
accept = trie;
}
d = trie->shift;
while (beg > text)
{
c = trans ? trans[U(*--beg)] : *--beg;
tree = trie->links;
while (tree && c != tree->label)
if (c < tree->label)
tree = tree->llink;
else
tree = tree->rlink;
if (tree)
{
trie = tree->trie;
if (trie->accepting)
{
mch = beg;
accept = trie;
}
}
else
break;
d = trie->shift;
}
if (mch)
goto match;
}
return -1;

match:



if (lim - mch > kwset->maxd)
lim = mch + kwset->maxd;
lmch = NULL;
d = 1;
while (lim - end >= d)
{
if ((d = delta[c = (end += d)[-1]]) != 0)
continue;
beg = end - 1;
if (!(trie = next[c]))
{
d = 1;
continue;
}
if (trie->accepting && beg <= mch)
{
lmch = beg;
accept = trie;
}
d = trie->shift;
while (beg > text)
{
c = trans ? trans[U(*--beg)] : *--beg;
tree = trie->links;
while (tree && c != tree->label)
if (c < tree->label)
tree = tree->llink;
else
tree = tree->rlink;
if (tree)
{
trie = tree->trie;
if (trie->accepting && beg <= mch)
{
lmch = beg;
accept = trie;
}
}
else
break;
d = trie->shift;
}
if (lmch)
{
mch = lmch;
goto match;
}
if (!d)
d = 1;
}

if (kwsmatch)
{
kwsmatch->index = accept->accepting / 2;
kwsmatch->offset[0] = mch - text;
kwsmatch->size[0] = accept->depth;
}
return mch - text;
}








size_t
kwsexec (kwset_t kws, char const *text, size_t size,
struct kwsmatch *kwsmatch)
{
struct kwset const *kwset = (struct kwset *) kws;
if (kwset->words == 1 && kwset->trans == NULL)
{
size_t ret = bmexec (kws, text, size);
if (kwsmatch != NULL && ret != (size_t) -1)
{
kwsmatch->index = 0;
kwsmatch->offset[0] = ret;
kwsmatch->size[0] = kwset->mind;
}
return ret;
}
else
return cwexec(kws, text, size, kwsmatch);
}


void
kwsfree (kwset_t kws)
{
struct kwset *kwset;

kwset = (struct kwset *) kws;
obstack_free(&kwset->obstack, NULL);
free(kws);
}
