#include "ruby.h"
#include <fcntl.h>
void
Init_fcntl(void)
{
VALUE mFcntl = rb_define_module("Fcntl");
#if defined(F_DUPFD)
rb_define_const(mFcntl, "F_DUPFD", INT2NUM(F_DUPFD));
#endif
#if defined(F_GETFD)
rb_define_const(mFcntl, "F_GETFD", INT2NUM(F_GETFD));
#endif
#if defined(F_GETLK)
rb_define_const(mFcntl, "F_GETLK", INT2NUM(F_GETLK));
#endif
#if defined(F_SETFD)
rb_define_const(mFcntl, "F_SETFD", INT2NUM(F_SETFD));
#endif
#if defined(F_GETFL)
rb_define_const(mFcntl, "F_GETFL", INT2NUM(F_GETFL));
#endif
#if defined(F_SETFL)
rb_define_const(mFcntl, "F_SETFL", INT2NUM(F_SETFL));
#endif
#if defined(F_SETLK)
rb_define_const(mFcntl, "F_SETLK", INT2NUM(F_SETLK));
#endif
#if defined(F_SETLKW)
rb_define_const(mFcntl, "F_SETLKW", INT2NUM(F_SETLKW));
#endif
#if defined(FD_CLOEXEC)
rb_define_const(mFcntl, "FD_CLOEXEC", INT2NUM(FD_CLOEXEC));
#endif
#if defined(F_RDLCK)
rb_define_const(mFcntl, "F_RDLCK", INT2NUM(F_RDLCK));
#endif
#if defined(F_UNLCK)
rb_define_const(mFcntl, "F_UNLCK", INT2NUM(F_UNLCK));
#endif
#if defined(F_WRLCK)
rb_define_const(mFcntl, "F_WRLCK", INT2NUM(F_WRLCK));
#endif
#if defined(O_CREAT)
rb_define_const(mFcntl, "O_CREAT", INT2NUM(O_CREAT));
#endif
#if defined(O_EXCL)
rb_define_const(mFcntl, "O_EXCL", INT2NUM(O_EXCL));
#endif
#if defined(O_NOCTTY)
rb_define_const(mFcntl, "O_NOCTTY", INT2NUM(O_NOCTTY));
#endif
#if defined(O_TRUNC)
rb_define_const(mFcntl, "O_TRUNC", INT2NUM(O_TRUNC));
#endif
#if defined(O_APPEND)
rb_define_const(mFcntl, "O_APPEND", INT2NUM(O_APPEND));
#endif
#if defined(O_NONBLOCK)
rb_define_const(mFcntl, "O_NONBLOCK", INT2NUM(O_NONBLOCK));
#endif
#if defined(O_NDELAY)
rb_define_const(mFcntl, "O_NDELAY", INT2NUM(O_NDELAY));
#endif
#if defined(O_RDONLY)
rb_define_const(mFcntl, "O_RDONLY", INT2NUM(O_RDONLY));
#endif
#if defined(O_RDWR)
rb_define_const(mFcntl, "O_RDWR", INT2NUM(O_RDWR));
#endif
#if defined(O_WRONLY)
rb_define_const(mFcntl, "O_WRONLY", INT2NUM(O_WRONLY));
#endif
#if defined(O_ACCMODE)
rb_define_const(mFcntl, "O_ACCMODE", INT2FIX(O_ACCMODE));
#else
rb_define_const(mFcntl, "O_ACCMODE", INT2FIX(O_RDONLY | O_WRONLY | O_RDWR));
#endif
}
