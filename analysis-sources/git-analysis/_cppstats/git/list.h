struct list_head {
struct list_head *next, *prev;
};
#undef LIST_HEAD
#define LIST_HEAD(name) struct list_head name = { &(name), &(name) }
#define INIT_LIST_HEAD(ptr) (ptr)->next = (ptr)->prev = (ptr)
#define LIST_HEAD_INIT(name) { &(name), &(name) }
static inline void list_add(struct list_head *newp, struct list_head *head)
{
head->next->prev = newp;
newp->next = head->next;
newp->prev = head;
head->next = newp;
}
static inline void list_add_tail(struct list_head *newp, struct list_head *head)
{
head->prev->next = newp;
newp->next = head;
newp->prev = head->prev;
head->prev = newp;
}
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
next->prev = prev;
prev->next = next;
}
static inline void list_del(struct list_head *elem)
{
__list_del(elem->prev, elem->next);
}
static inline void list_del_init(struct list_head *elem)
{
list_del(elem);
INIT_LIST_HEAD(elem);
}
static inline void list_move(struct list_head *elem, struct list_head *head)
{
__list_del(elem->prev, elem->next);
list_add(elem, head);
}
static inline void list_replace(struct list_head *old, struct list_head *newp)
{
newp->next = old->next;
newp->prev = old->prev;
newp->prev->next = newp;
newp->next->prev = newp;
}
static inline void list_splice(struct list_head *add, struct list_head *head)
{
if (add != add->next) {
add->next->prev = head;
add->prev->next = head->next;
head->next->prev = add->prev;
head->next = add->next;
}
}
#define list_entry(ptr, type, member) ((type *) ((char *) (ptr) - offsetof(type, member)))
#define list_first_entry(ptr, type, member) list_entry((ptr)->next, type, member)
#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)
#define list_for_each_safe(pos, p, head) for (pos = (head)->next, p = pos->next; pos != (head); pos = p, p = pos->next)
#define list_for_each_prev(pos, head) for (pos = (head)->prev; pos != (head); pos = pos->prev)
#define list_for_each_prev_safe(pos, p, head) for (pos = (head)->prev, p = pos->prev; pos != (head); pos = p, p = pos->prev)
static inline int list_empty(struct list_head *head)
{
return head == head->next;
}
static inline void list_replace_init(struct list_head *old,
struct list_head *newp)
{
struct list_head *head = old->next;
list_del(old);
list_add_tail(newp, head);
INIT_LIST_HEAD(old);
}
struct volatile_list_head {
volatile struct volatile_list_head *next, *prev;
};
#define VOLATILE_LIST_HEAD(name) volatile struct volatile_list_head name = { &(name), &(name) }
static inline void __volatile_list_del(volatile struct volatile_list_head *prev,
volatile struct volatile_list_head *next)
{
next->prev = prev;
prev->next = next;
}
static inline void volatile_list_del(volatile struct volatile_list_head *elem)
{
__volatile_list_del(elem->prev, elem->next);
}
static inline int volatile_list_empty(volatile struct volatile_list_head *head)
{
return head == head->next;
}
static inline void volatile_list_add(volatile struct volatile_list_head *newp,
volatile struct volatile_list_head *head)
{
head->next->prev = newp;
newp->next = head->next;
newp->prev = head;
head->next = newp;
}
