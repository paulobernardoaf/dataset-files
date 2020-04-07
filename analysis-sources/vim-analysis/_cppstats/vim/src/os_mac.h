#if 0
#define OPAQUE_TOOLBOX_STRUCTS 0
#endif
#if defined(HAVE_AVAILABILITYMACROS_H)
#include <AvailabilityMacros.h>
#endif
#if defined(FEAT_GUI_MAC)
#include <Quickdraw.h> 
#include <ToolUtils.h>
#include <LowMem.h>
#include <Scrap.h>
#include <Sound.h>
#include <TextUtils.h>
#include <Memory.h>
#include <OSUtils.h>
#include <Files.h>
#include <Script.h>
#endif
#if defined(__APPLE_CC__) 
#include <unistd.h>
#include <sys/stat.h>
#include <curses.h>
#undef reg
#undef ospeed
#undef OK
#endif
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#define USE_CMD_KEY
#define USE_UNIXFILENAME
#define FEAT_SOURCE_FFS
#define FEAT_SOURCE_FF_MAC
#define USE_EXE_NAME 
#define CASE_INSENSITIVE_FILENAME 
#define SPACE_IN_FILENAME
#define USE_FNAME_CASE 
#define BINARY_FILE_IO
#define EOL_DEFAULT EOL_MAC
#define HAVE_AVAIL_MEM
#if !defined(HAVE_CONFIG_H)
#define HAVE_STRING_H
#define HAVE_STRCSPN
#define HAVE_MEMSET
#define USE_TMPNAM 
#define HAVE_FCNTL_H
#define HAVE_QSORT
#define HAVE_ST_MODE 
#define HAVE_MATH_H
#if defined(__DATE__) && defined(__TIME__)
#define HAVE_DATE_TIME
#endif
#define HAVE_STRFTIME
#endif
#if !defined(SYS_VIMRC_FILE)
#define SYS_VIMRC_FILE "$VIM/vimrc"
#endif
#if !defined(SYS_GVIMRC_FILE)
#define SYS_GVIMRC_FILE "$VIM/gvimrc"
#endif
#if !defined(SYS_MENU_FILE)
#define SYS_MENU_FILE "$VIMRUNTIME/menu.vim"
#endif
#if !defined(SYS_OPTWIN_FILE)
#define SYS_OPTWIN_FILE "$VIMRUNTIME/optwin.vim"
#endif
#if !defined(VIM_DEFAULTS_FILE)
#define VIM_DEFAULTS_FILE "$VIMRUNTIME/defaults.vim"
#endif
#if !defined(EVIM_FILE)
#define EVIM_FILE "$VIMRUNTIME/evim.vim"
#endif
#if defined(FEAT_GUI)
#if !defined(USR_GVIMRC_FILE)
#define USR_GVIMRC_FILE "~/.gvimrc"
#endif
#if !defined(GVIMRC_FILE)
#define GVIMRC_FILE "_gvimrc"
#endif
#endif
#if !defined(USR_VIMRC_FILE)
#define USR_VIMRC_FILE "~/.vimrc"
#endif
#if !defined(USR_EXRC_FILE)
#define USR_EXRC_FILE "~/.exrc"
#endif
#if !defined(VIMRC_FILE)
#define VIMRC_FILE "_vimrc"
#endif
#if !defined(EXRC_FILE)
#define EXRC_FILE "_exrc"
#endif
#if !defined(DFLT_HELPFILE)
#define DFLT_HELPFILE "$VIMRUNTIME/doc/help.txt"
#endif
#if !defined(FILETYPE_FILE)
#define FILETYPE_FILE "filetype.vim"
#endif
#if !defined(FTPLUGIN_FILE)
#define FTPLUGIN_FILE "ftplugin.vim"
#endif
#if !defined(INDENT_FILE)
#define INDENT_FILE "indent.vim"
#endif
#if !defined(FTOFF_FILE)
#define FTOFF_FILE "ftoff.vim"
#endif
#if !defined(FTPLUGOF_FILE)
#define FTPLUGOF_FILE "ftplugof.vim"
#endif
#if !defined(INDOFF_FILE)
#define INDOFF_FILE "indoff.vim"
#endif
#if !defined(SYNTAX_FNAME)
#define SYNTAX_FNAME "$VIMRUNTIME/syntax/%s.vim"
#endif
#if defined(FEAT_VIMINFO)
#if !defined(VIMINFO_FILE)
#define VIMINFO_FILE "~/.viminfo"
#endif
#endif 
#if !defined(DFLT_BDIR)
#define DFLT_BDIR "." 
#endif
#if !defined(DFLT_DIR)
#define DFLT_DIR "." 
#endif
#if !defined(DFLT_VDIR)
#define DFLT_VDIR "$VIM/vimfiles/view" 
#endif
#define DFLT_ERRORFILE "errors.err"
#if !defined(DFLT_RUNTIMEPATH)
#define DFLT_RUNTIMEPATH "~/.vim,$VIM/vimfiles,$VIMRUNTIME,$VIM/vimfiles/after,~/.vim/after"
#endif
#if !defined(CLEAN_RUNTIMEPATH)
#define CLEAN_RUNTIMEPATH "$VIM/vimfiles,$VIMRUNTIME,$VIM/vimfiles/after"
#endif
#define CMDBUFFSIZE 1024 
#if !defined(DFLT_MAXMEM)
#define DFLT_MAXMEM 512 
#endif
#if !defined(DFLT_MAXMEMTOT)
#define DFLT_MAXMEMTOT 2048 
#endif
#define WILDCHAR_LIST "*?[{`$"
#define mch_rename(src, dst) rename(src, dst)
#define mch_remove(x) unlink((char *)(x))
#if !defined(mch_getenv)
#if defined(__APPLE_CC__)
#define mch_getenv(name) ((char_u *)getenv((char *)(name)))
#define mch_setenv(name, val, x) setenv(name, val, x)
#else
#define USE_VIMPTY_GETENV
#define mch_getenv(x) vimpty_getenv(x)
#define mch_setenv(name, val, x) setenv(name, val, x)
#endif
#endif
#if !defined(HAVE_CONFIG_H)
#if defined(__APPLE_CC__)
#define HAVE_TGETENT
#define OSPEED_EXTERN
#define UP_BC_PC_EXTERN
#endif
#endif
#if !defined(SIGPROTOARG)
#define SIGPROTOARG (int)
#endif
#if !defined(SIGDEFARG)
#define SIGDEFARG(s) (s) int s UNUSED;
#endif
#if !defined(SIGDUMMYARG)
#define SIGDUMMYARG 0
#endif
#undef HAVE_AVAIL_MEM
#if !defined(HAVE_CONFIG_H)
#define RETSIGTYPE void
#define SIGRETURN return
#define HAVE_SYS_WAIT_H 1 
#define HAVE_TERMIOS_H 1
#define SYS_SELECT_WITH_SYS_TIME 1
#define HAVE_SELECT 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_PUTENV
#define HAVE_SETENV
#define HAVE_RENAME
#endif
#if !defined(HAVE_CONFIG_H)
#define HAVE_PUTENV
#endif
#define UNKNOWN_CREATOR '\?\?\?\?'
