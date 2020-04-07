#define CASE_INSENSITIVE_FILENAME 
#define SPACE_IN_FILENAME
#define USE_FNAME_CASE 
#define USE_TERM_CONSOLE
#define HAVE_AVAIL_MEM
#if !defined(HAVE_CONFIG_H)
#if defined(AZTEC_C) || defined(__amigaos4__)
#define HAVE_STAT_H
#endif
#define HAVE_STDLIB_H
#define HAVE_STRING_H
#define HAVE_FCNTL_H
#define HAVE_STRCSPN
#define HAVE_STRICMP
#define HAVE_STRNICMP
#define HAVE_STRFTIME 
#define HAVE_SETENV
#define HAVE_MEMSET
#define HAVE_QSORT
#if defined(__DATE__) && defined(__TIME__)
#define HAVE_DATE_TIME
#endif
#endif 
#if !defined(DFLT_ERRORFILE)
#define DFLT_ERRORFILE "AztecC.Err" 
#endif
#if !defined(DFLT_RUNTIMEPATH)
#define DFLT_RUNTIMEPATH "home:vimfiles,$VIM/vimfiles,$VIMRUNTIME,$VIM/vimfiles/after,home:vimfiles/after"
#endif
#if !defined(CLEAN_RUNTIMEPATH)
#define CLEAN_RUNTIMEPATH "$VIM/vimfiles,$VIMRUNTIME,$VIM/vimfiles/after"
#endif
#if !defined(BASENAMELEN)
#define BASENAMELEN 26 
#endif
#if !defined(TEMPNAME)
#define TEMPNAME "t:v?XXXXXX"
#define TEMPNAMELEN 12
#endif
#if !defined(PROTO)
#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#if !defined(AZTEC_C)
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#endif
#endif 
#define FNAME_ILLEGAL ";*?`#%" 
#if defined(AZTEC_C)
typedef long off_t;
#endif
#if defined(LATTICE)
#define USE_TMPNAM 
#endif
#if defined(__GNUC__)
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <pwd.h>
#include <dirent.h>
#endif
#include <time.h> 
#if !defined(PROTO)
#if defined(FEAT_ARP)
#include <libraries/arpbase.h>
#endif
#endif 
#include <signal.h>
#if !defined(SYS_VIMRC_FILE)
#define SYS_VIMRC_FILE "$VIM/vimrc"
#endif
#if !defined(SYS_GVIMRC_FILE)
#define SYS_GVIMRC_FILE "$VIM/gvimrc"
#endif
#if !defined(SYS_MENU_FILE)
#define SYS_MENU_FILE "$VIMRUNTIME/menu.vim"
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
#if !defined(USR_EXRC_FILE)
#define USR_EXRC_FILE "s:.exrc"
#endif
#if !defined(USR_EXRC_FILE2)
#define USR_EXRC_FILE2 "home:.exrc"
#endif
#if !defined(USR_VIMRC_FILE)
#define USR_VIMRC_FILE "s:.vimrc"
#endif
#if !defined(USR_VIMRC_FILE2)
#define USR_VIMRC_FILE2 "home:.vimrc"
#endif
#if !defined(USR_VIMRC_FILE3)
#define USR_VIMRC_FILE3 "home:vimfiles:vimrc"
#endif
#if !defined(USR_VIMRC_FILE4)
#define USR_VIMRC_FILE4 "$VIM/.vimrc"
#endif
#if !defined(VIM_DEFAULTS_FILE)
#define VIM_DEFAULTS_FILE "$VIMRUNTIME/defaults.vim"
#endif
#if !defined(EVIM_FILE)
#define EVIM_FILE "$VIMRUNTIME/evim.vim"
#endif
#if !defined(USR_GVIMRC_FILE)
#define USR_GVIMRC_FILE "s:.gvimrc"
#endif
#if !defined(USR_GVIMRC_FILE2)
#define USR_GVIMRC_FILE2 "home:.gvimrc"
#endif
#if !defined(USR_GVIMRC_FILE3)
#define USR_GVIMRC_FILE3 "home:vimfiles:gvimrc"
#endif
#if !defined(USR_GVIMRC_FILE4)
#define USR_GVIMRC_FILE4 "$VIM/.gvimrc"
#endif
#if defined(FEAT_VIMINFO)
#if !defined(VIMINFO_FILE)
#define VIMINFO_FILE "s:.viminfo"
#endif
#endif
#if !defined(EXRC_FILE)
#define EXRC_FILE ".exrc"
#endif
#if !defined(VIMRC_FILE)
#define VIMRC_FILE ".vimrc"
#endif
#if !defined(GVIMRC_FILE)
#define GVIMRC_FILE ".gvimrc"
#endif
#if !defined(DFLT_BDIR)
#define DFLT_BDIR ".,t:" 
#endif
#if !defined(DFLT_DIR)
#define DFLT_DIR ".,t:" 
#endif
#if !defined(DFLT_VDIR)
#define DFLT_VDIR "$VIM/vimfiles/view" 
#endif
#if !defined(DFLT_MAXMEM)
#define DFLT_MAXMEM 256 
#endif
#if !defined(DFLT_MAXMEMTOT)
#define DFLT_MAXMEMTOT 0 
#endif
#if defined(SASC)
int setenv(const char *, const char *);
#endif
#define mch_remove(x) remove((char *)(x))
#define mch_rename(src, dst) rename(src, dst)
#define mch_chdir(s) chdir(s)
#define vim_mkdir(x, y) mch_mkdir(x)
