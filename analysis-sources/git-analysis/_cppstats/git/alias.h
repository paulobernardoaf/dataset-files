struct string_list;
char *alias_lookup(const char *alias);
int split_cmdline(char *cmdline, const char ***argv);
const char *split_cmdline_strerror(int cmdline_errno);
void list_aliases(struct string_list *list);
