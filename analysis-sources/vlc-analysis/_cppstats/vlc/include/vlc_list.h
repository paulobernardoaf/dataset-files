#include <stdalign.h>
#include <stdbool.h>
struct vlc_list
{
struct vlc_list *prev;
struct vlc_list *next;
};
#define VLC_LIST_INITIALIZER(h) { h, h }
static inline void vlc_list_init(struct vlc_list *restrict head)
{
head->prev = head;
head->next = head;
}
static inline void vlc_list_add_between(struct vlc_list *restrict node,
struct vlc_list *prev,
struct vlc_list *next)
{
node->prev = prev;
node->next = next;
prev->next = node;
next->prev = node;
}
static inline void vlc_list_add_after(struct vlc_list *restrict node,
struct vlc_list *prev)
{
vlc_list_add_between(node, prev, prev->next);
}
static inline void vlc_list_add_before(struct vlc_list *restrict node,
struct vlc_list *next)
{
vlc_list_add_between(node, next->prev, next);
}
static inline void vlc_list_append(struct vlc_list *restrict node,
struct vlc_list *head)
{
vlc_list_add_before(node, head);
}
static inline void vlc_list_prepend(struct vlc_list *restrict node,
struct vlc_list *head)
{
vlc_list_add_after(node, head);
}
static inline void vlc_list_remove(struct vlc_list *restrict node)
{
struct vlc_list *prev = node->prev;
struct vlc_list *next = node->next;
prev->next = next;
next->prev = prev;
}
static inline void vlc_list_replace(const struct vlc_list *original,
struct vlc_list *restrict substitute)
{
vlc_list_add_between(substitute, original->prev, original->next);
}
static inline bool vlc_list_is_empty(const struct vlc_list *head)
{
return head->next == head;
}
static inline bool vlc_list_is_first(const struct vlc_list *node,
const struct vlc_list *head)
{
return node->prev == head;
}
static inline bool vlc_list_is_last(const struct vlc_list *node,
const struct vlc_list *head)
{
return node->next == head;
}
struct vlc_list_it
{
const struct vlc_list *head;
struct vlc_list *current;
struct vlc_list *next;
};
static inline
struct vlc_list_it vlc_list_it_start(const struct vlc_list *head)
{
struct vlc_list *first = head->next;
return (struct vlc_list_it){ head, first, first->next };
}
static inline bool vlc_list_it_continue(const struct vlc_list_it *restrict it)
{
return it->current != it->head;
}
static inline void vlc_list_it_next(struct vlc_list_it *restrict it)
{
struct vlc_list *next = it->next;
it->current = next;
it->next = next->next;
}
#define vlc_list_entry_aligned_size(p) ((sizeof (*(p)) + sizeof (max_align_t) - 1) / sizeof (max_align_t))
#define vlc_list_entry_dummy(p) (0 ? (p) : ((void *)(&(max_align_t[vlc_list_entry_aligned_size(p)]){})))
#define vlc_list_offset_p(p, member) ((p) = vlc_list_entry_dummy(p), (char *)(&(p)->member) - (char *)(p))
#define vlc_list_entry_p(node, p, member) (0 ? (p) : (void *)(((char *)(node)) - vlc_list_offset_p(p, member)))
#define vlc_list_foreach(pos, head, member) for (struct vlc_list_it vlc_list_it_##pos = vlc_list_it_start(head); vlc_list_it_continue(&(vlc_list_it_##pos)) && ((pos) = vlc_list_entry_p((vlc_list_it_##pos).current, pos, member), true); vlc_list_it_next(&(vlc_list_it_##pos)))
#define vlc_list_entry(ptr, type, member) container_of(ptr, type, member)
static inline void *vlc_list_first_or_null(const struct vlc_list *head,
size_t offset)
{
if (vlc_list_is_empty(head))
return NULL;
return ((char *)(head->next)) - offset;
}
static inline void *vlc_list_last_or_null(const struct vlc_list *head,
size_t offset)
{
if (vlc_list_is_empty(head))
return NULL;
return ((char *)(head->prev)) - offset;
}
static inline void *vlc_list_prev_or_null(const struct vlc_list *head,
struct vlc_list *node,
size_t offset)
{
if (vlc_list_is_first(node, head))
return NULL;
return ((char *)(node->prev)) - offset;
}
static inline void *vlc_list_next_or_null(const struct vlc_list *head,
struct vlc_list *node,
size_t offset)
{
if (vlc_list_is_last(node, head))
return NULL;
return ((char *)(node->next)) - offset;
}
#define vlc_list_first_entry_or_null(head, type, member) ((type *)vlc_list_first_or_null(head, offsetof (type, member)))
#define vlc_list_last_entry_or_null(head, type, member) ((type *)vlc_list_last_or_null(head, offsetof (type, member)))
#define vlc_list_prev_entry_or_null(head, entry, type, member) ((type *)vlc_list_prev_or_null(head, &(entry)->member, offsetof (type, member)))
#define vlc_list_next_entry_or_null(head, entry, type, member) ((type *)vlc_list_next_or_null(head, &(entry)->member, offsetof (type, member)))
