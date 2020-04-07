
#if !defined(CCAN_LIST_H)
#define CCAN_LIST_H
#include <assert.h>
#include "ccan/str/str.h"
#include "ccan/container_of/container_of.h"
#include "ccan/check_type/check_type.h"














struct list_node
{
struct list_node *next, *prev;
};













struct list_head
{
struct list_node n;
};

#define LIST_LOC __FILE__ ":" stringify(__LINE__)
#define list_debug(h, loc) ((void)loc, h)
#define list_debug_node(n, loc) ((void)loc, n)













#define LIST_HEAD_INIT(name) { { &(name).n, &(name).n } }














#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)













static inline void list_head_init(struct list_head *h)
{
h->n.next = h->n.prev = &h->n;
}








static inline void list_node_init(struct list_node *n)
{
n->next = n->prev = n;
}


















#define list_add_after(h, p, n) list_add_after_(h, p, n, LIST_LOC)
static inline void list_add_after_(struct list_head *h,
struct list_node *p,
struct list_node *n,
const char *abortstr)
{
n->next = p->next;
n->prev = p;
p->next->prev = n;
p->next = n;
(void)list_debug(h, abortstr);
}














#define list_add(h, n) list_add_(h, n, LIST_LOC)
static inline void list_add_(struct list_head *h,
struct list_node *n,
const char *abortstr)
{
list_add_after_(h, &h->n, n, abortstr);
}
















#define list_add_before(h, p, n) list_add_before_(h, p, n, LIST_LOC)
static inline void list_add_before_(struct list_head *h,
struct list_node *p,
struct list_node *n,
const char *abortstr)
{
n->next = p;
n->prev = p->prev;
p->prev->next = n;
p->prev = n;
(void)list_debug(h, abortstr);
}











#define list_add_tail(h, n) list_add_tail_(h, n, LIST_LOC)
static inline void list_add_tail_(struct list_head *h,
struct list_node *n,
const char *abortstr)
{
list_add_before_(h, &h->n, n, abortstr);
}










#define list_empty(h) list_empty_(h, LIST_LOC)
static inline int list_empty_(const struct list_head *h, const char* abortstr)
{
(void)list_debug(h, abortstr);
return h->n.next == &h->n;
}













#if !defined(CCAN_LIST_DEBUG)
#define list_empty_nodebug(h) list_empty(h)
#else
static inline int list_empty_nodebug(const struct list_head *h)
{
return h->n.next == &h->n;
}
#endif











static inline bool list_empty_nocheck(const struct list_head *h)
{
return h->n.next == &h->n;
}















#define list_del(n) list_del_(n, LIST_LOC)
static inline void list_del_(struct list_node *n, const char* abortstr)
{
(void)list_debug_node(n, abortstr);
n->next->prev = n->prev;
n->prev->next = n->next;
#if defined(CCAN_LIST_DEBUG)

n->next = n->prev = NULL;
#endif
}















#define list_del_init(n) list_del_init_(n, LIST_LOC)
static inline void list_del_init_(struct list_node *n, const char *abortstr)
{
list_del_(n, abortstr);
list_node_init(n);
}















static inline void list_del_from(struct list_head *h, struct list_node *n)
{
#if defined(CCAN_LIST_DEBUG)
{

struct list_node *i;
for (i = h->n.next; i != n; i = i->next)
assert(i != &h->n);
}
#endif 


assert(!list_empty(h));
list_del(n);
}



















#define list_swap(o, n) list_swap_(o, n, LIST_LOC)
static inline void list_swap_(struct list_node *o,
struct list_node *n,
const char* abortstr)
{
(void)list_debug_node(o, abortstr);
*n = *o;
n->next->prev = n;
n->prev->next = n;
#if defined(CCAN_LIST_DEBUG)

o->next = o->prev = NULL;
#endif
}














#define list_entry(n, type, member) container_of(n, type, member)















#define list_top(h, type, member) ((type *)list_top_((h), list_off_(type, member)))


static inline const void *list_top_(const struct list_head *h, size_t off)
{
if (list_empty(h))
return NULL;
return (const char *)h->n.next - off;
}















#define list_pop(h, type, member) ((type *)list_pop_((h), list_off_(type, member)))


static inline const void *list_pop_(const struct list_head *h, size_t off)
{
struct list_node *n;

if (list_empty(h))
return NULL;
n = h->n.next;
list_del(n);
return (const char *)n - off;
}















#define list_tail(h, type, member) ((type *)list_tail_((h), list_off_(type, member)))


static inline const void *list_tail_(const struct list_head *h, size_t off)
{
if (list_empty(h))
return NULL;
return (const char *)h->n.prev - off;
}














#define list_for_each(h, i, member) list_for_each_off(h, i, list_off_var_(i, member))















#define list_for_each_rev(h, i, member) list_for_each_rev_off(h, i, list_off_var_(i, member))





















#define list_for_each_rev_safe(h, i, nxt, member) list_for_each_rev_safe_off(h, i, nxt, list_off_var_(i, member))



















#define list_for_each_safe(h, i, nxt, member) list_for_each_safe_off(h, i, nxt, list_off_var_(i, member))
















#define list_next(h, i, member) ((list_typeof(i))list_entry_or_null(list_debug(h, __FILE__ ":" stringify(__LINE__)), (i)->member.next, list_off_var_((i), member)))


















#define list_prev(h, i, member) ((list_typeof(i))list_entry_or_null(list_debug(h, __FILE__ ":" stringify(__LINE__)), (i)->member.prev, list_off_var_((i), member)))




















#define list_append_list(t, f) list_append_list_(t, f, __FILE__ ":" stringify(__LINE__))

static inline void list_append_list_(struct list_head *to,
struct list_head *from,
const char *abortstr)
{
struct list_node *from_tail = list_debug(from, abortstr)->n.prev;
struct list_node *to_tail = list_debug(to, abortstr)->n.prev;


to->n.prev = from_tail;
from_tail->next = &to->n;
to_tail->next = &from->n;
from->n.prev = to_tail;


list_del(&from->n);
list_head_init(from);
}














#define list_prepend_list(t, f) list_prepend_list_(t, f, LIST_LOC)
static inline void list_prepend_list_(struct list_head *to,
struct list_head *from,
const char *abortstr)
{
struct list_node *from_tail = list_debug(from, abortstr)->n.prev;
struct list_node *to_head = list_debug(to, abortstr)->n.next;


to->n.next = &from->n;
from->n.prev = &to->n;
to_head->prev = from_tail;
from_tail->next = to_head;


list_del(&from->n);
list_head_init(from);
}


#define list_for_each_off_dir_(h, i, off, dir) for (i = list_node_to_off_(list_debug(h, LIST_LOC)->n.dir, (off)); list_node_from_off_((void *)i, (off)) != &(h)->n; i = list_node_to_off_(list_node_from_off_((void *)i, (off))->dir, (off)))






#define list_for_each_safe_off_dir_(h, i, nxt, off, dir) for (i = list_node_to_off_(list_debug(h, LIST_LOC)->n.dir, (off)), nxt = list_node_to_off_(list_node_from_off_(i, (off))->dir, (off)); list_node_from_off_(i, (off)) != &(h)->n; i = nxt, nxt = list_node_to_off_(list_node_from_off_(i, (off))->dir, (off)))






































#define list_for_each_off(h, i, off) list_for_each_off_dir_((h),(i),(off),next)










#define list_for_each_rev_off(h, i, off) list_for_each_off_dir_((h),(i),(off),prev)


















#define list_for_each_safe_off(h, i, nxt, off) list_for_each_safe_off_dir_((h),(i),(nxt),(off),next)


















#define list_for_each_rev_safe_off(h, i, nxt, off) list_for_each_safe_off_dir_((h),(i),(nxt),(off),prev)



#define list_entry_off(n, type, off) ((type *)list_node_from_off_((n), (off)))


#define list_head_off(h, type, off) ((type *)list_head_off((h), (off)))


#define list_tail_off(h, type, off) ((type *)list_tail_((h), (off)))


#define list_add_off(h, n, off) list_add((h), list_node_from_off_((n), (off)))


#define list_del_off(n, off) list_del(list_node_from_off_((n), (off)))


#define list_del_from_off(h, n, off) list_del_from(h, list_node_from_off_((n), (off)))



static inline void *list_node_to_off_(struct list_node *node, size_t off)
{
return (void *)((char *)node - off);
}
static inline struct list_node *list_node_from_off_(void *ptr, size_t off)
{
return (struct list_node *)((char *)ptr + off);
}


#define list_off_(type, member) (container_off(type, member) + check_type(((type *)0)->member, struct list_node))



#define list_off_var_(var, member) (container_off_var(var, member) + check_type(var->member, struct list_node))



#if HAVE_TYPEOF
#define list_typeof(var) typeof(var)
#else
#define list_typeof(var) void *
#endif


static inline void *list_entry_or_null(const struct list_head *h,
const struct list_node *n,
size_t off)
{
if (n == &h->n)
return NULL;
return (char *)n - off;
}
#endif 
