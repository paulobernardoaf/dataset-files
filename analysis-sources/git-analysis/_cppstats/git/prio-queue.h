typedef int (*prio_queue_compare_fn)(const void *one, const void *two, void *cb_data);
struct prio_queue_entry {
unsigned ctr;
void *data;
};
struct prio_queue {
prio_queue_compare_fn compare;
unsigned insertion_ctr;
void *cb_data;
int alloc, nr;
struct prio_queue_entry *array;
};
void prio_queue_put(struct prio_queue *, void *thing);
void *prio_queue_get(struct prio_queue *);
void *prio_queue_peek(struct prio_queue *);
void clear_prio_queue(struct prio_queue *);
void prio_queue_reverse(struct prio_queue *);
