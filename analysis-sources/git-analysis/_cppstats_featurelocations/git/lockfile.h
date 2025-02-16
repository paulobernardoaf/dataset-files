#if !defined(LOCKFILE_H)
#define LOCKFILE_H










































































































#include "tempfile.h"

struct lock_file {
struct tempfile *tempfile;
};

#define LOCK_INIT { NULL }


#define LOCK_SUFFIX ".lock"
#define LOCK_SUFFIX_LEN 5














#define LOCK_DIE_ON_ERROR 1





#define LOCK_REPORT_ON_ERROR 4











#define LOCK_NO_DEREF 2









int hold_lock_file_for_update_timeout(
struct lock_file *lk, const char *path,
int flags, long timeout_ms);






static inline int hold_lock_file_for_update(
struct lock_file *lk, const char *path,
int flags)
{
return hold_lock_file_for_update_timeout(lk, path, flags, 0);
}




static inline int is_lock_file_locked(struct lock_file *lk)
{
return is_tempfile_active(lk->tempfile);
}






void unable_to_lock_message(const char *path, int err,
struct strbuf *buf);







NORETURN void unable_to_lock_die(const char *path, int err);








static inline FILE *fdopen_lock_file(struct lock_file *lk, const char *mode)
{
return fdopen_tempfile(lk->tempfile, mode);
}





static inline const char *get_lock_file_path(struct lock_file *lk)
{
return get_tempfile_path(lk->tempfile);
}

static inline int get_lock_file_fd(struct lock_file *lk)
{
return get_tempfile_fd(lk->tempfile);
}

static inline FILE *get_lock_file_fp(struct lock_file *lk)
{
return get_tempfile_fp(lk->tempfile);
}





char *get_locked_file_path(struct lock_file *lk);









static inline int close_lock_file_gently(struct lock_file *lk)
{
return close_tempfile_gently(lk->tempfile);
}




















static inline int reopen_lock_file(struct lock_file *lk)
{
return reopen_tempfile(lk->tempfile);
}










int commit_lock_file(struct lock_file *lk);





static inline int commit_lock_file_to(struct lock_file *lk, const char *path)
{
return rename_tempfile(&lk->tempfile, path);
}







static inline void rollback_lock_file(struct lock_file *lk)
{
delete_tempfile(&lk->tempfile);
}

#endif 
