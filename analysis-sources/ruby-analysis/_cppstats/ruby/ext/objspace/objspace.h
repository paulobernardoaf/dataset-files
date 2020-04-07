struct allocation_info {
int living;
VALUE flags;
VALUE klass;
const char *path;
unsigned long line;
const char *class_path;
VALUE mid;
size_t generation;
};
struct allocation_info *objspace_lookup_allocation_info(VALUE obj);
