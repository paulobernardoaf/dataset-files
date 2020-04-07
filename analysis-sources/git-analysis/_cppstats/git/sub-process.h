#include "git-compat-util.h"
#include "hashmap.h"
#include "run-command.h"
struct subprocess_entry {
struct hashmap_entry ent;
const char *cmd;
struct child_process process;
};
struct subprocess_capability {
const char *name;
unsigned int flag;
};
int cmd2process_cmp(const void *unused_cmp_data,
const struct hashmap_entry *e,
const struct hashmap_entry *entry_or_key,
const void *unused_keydata);
typedef int(*subprocess_start_fn)(struct subprocess_entry *entry);
int subprocess_start(struct hashmap *hashmap, struct subprocess_entry *entry, const char *cmd,
subprocess_start_fn startfn);
void subprocess_stop(struct hashmap *hashmap, struct subprocess_entry *entry);
struct subprocess_entry *subprocess_find_entry(struct hashmap *hashmap, const char *cmd);
static inline struct child_process *subprocess_get_child_process(
struct subprocess_entry *entry)
{
return &entry->process;
}
int subprocess_handshake(struct subprocess_entry *entry,
const char *welcome_prefix,
int *versions,
int *chosen_version,
struct subprocess_capability *capabilities,
unsigned int *supported_capabilities);
int subprocess_read_status(int fd, struct strbuf *status);
