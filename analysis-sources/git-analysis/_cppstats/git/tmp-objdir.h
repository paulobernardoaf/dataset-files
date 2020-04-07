struct tmp_objdir;
struct tmp_objdir *tmp_objdir_create(void);
const char **tmp_objdir_env(const struct tmp_objdir *);
int tmp_objdir_migrate(struct tmp_objdir *);
int tmp_objdir_destroy(struct tmp_objdir *);
void tmp_objdir_add_as_alternate(const struct tmp_objdir *);
