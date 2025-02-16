#include "strbuf.h"
#define DIR_ITERATOR_PEDANTIC (1 << 0)
#define DIR_ITERATOR_FOLLOW_SYMLINKS (1 << 1)
struct dir_iterator {
struct strbuf path;
const char *relative_path;
const char *basename;
struct stat st;
};
struct dir_iterator *dir_iterator_begin(const char *path, unsigned int flags);
int dir_iterator_advance(struct dir_iterator *iterator);
int dir_iterator_abort(struct dir_iterator *iterator);
