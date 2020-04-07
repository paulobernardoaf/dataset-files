typedef struct Aatree Aatree;
struct Aatree {
int level;
Aatree *left;
Aatree *right;
char *key;
char *value;
};
extern Aatree aa_nil;
Aatree *aa_singleton(const char *key, const char *value);
Aatree *aa_insert(Aatree *T, const char *key, const char *value);
const char *aa_search(const Aatree *T, const char *key);
void aa_destroy(Aatree *T);
