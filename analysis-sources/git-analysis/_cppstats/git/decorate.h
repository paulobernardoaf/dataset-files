struct decoration_entry {
const struct object *base;
void *decoration;
};
struct decoration {
const char *name;
unsigned int size;
unsigned int nr;
struct decoration_entry *entries;
};
void *add_decoration(struct decoration *n, const struct object *obj, void *decoration);
void *lookup_decoration(struct decoration *n, const struct object *obj);
