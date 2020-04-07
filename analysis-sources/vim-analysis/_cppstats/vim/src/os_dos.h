#if !defined(USR_VIMRC_FILE)
#define USR_VIMRC_FILE "$HOME\\_vimrc"
#endif
#if !defined(USR_VIMRC_FILE2)
#define USR_VIMRC_FILE2 "$HOME\\vimfiles\\vimrc"
#endif
#if !defined(USR_VIMRC_FILE3)
#define USR_VIMRC_FILE3 "$VIM\\_vimrc"
#endif
#if !defined(VIM_DEFAULTS_FILE)
#define VIM_DEFAULTS_FILE "$VIMRUNTIME\\defaults.vim"
#endif
#if !defined(EVIM_FILE)
#define EVIM_FILE "$VIMRUNTIME\\evim.vim"
#endif
#if !defined(USR_EXRC_FILE)
#define USR_EXRC_FILE "$HOME\\_exrc"
#endif
#if !defined(USR_EXRC_FILE2)
#define USR_EXRC_FILE2 "$VIM\\_exrc"
#endif
#if defined(FEAT_GUI)
#if !defined(SYS_GVIMRC_FILE)
#define SYS_GVIMRC_FILE "$VIM\\gvimrc"
#endif
#if !defined(USR_GVIMRC_FILE)
#define USR_GVIMRC_FILE "$HOME\\_gvimrc"
#endif
#if !defined(USR_GVIMRC_FILE2)
#define USR_GVIMRC_FILE2 "$HOME\\vimfiles\\gvimrc"
#endif
#if !defined(USR_GVIMRC_FILE3)
#define USR_GVIMRC_FILE3 "$VIM\\_gvimrc"
#endif
#if !defined(SYS_MENU_FILE)
#define SYS_MENU_FILE "$VIMRUNTIME\\menu.vim"
#endif
#endif
#if !defined(SYS_OPTWIN_FILE)
#define SYS_OPTWIN_FILE "$VIMRUNTIME\\optwin.vim"
#endif
#if defined(FEAT_VIMINFO)
#if !defined(VIMINFO_FILE)
#define VIMINFO_FILE "$HOME\\_viminfo"
#endif
#if !defined(VIMINFO_FILE2)
#define VIMINFO_FILE2 "$VIM\\_viminfo"
#endif
#endif
#if !defined(VIMRC_FILE)
#define VIMRC_FILE "_vimrc"
#endif
#if !defined(EXRC_FILE)
#define EXRC_FILE "_exrc"
#endif
#if defined(FEAT_GUI)
#if !defined(GVIMRC_FILE)
#define GVIMRC_FILE "_gvimrc"
#endif
#endif
#if !defined(DFLT_HELPFILE)
#define DFLT_HELPFILE "$VIMRUNTIME\\doc\\help.txt"
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
#define SYNTAX_FNAME "$VIMRUNTIME\\syntax\\%s.vim"
#endif
#if !defined(DFLT_BDIR)
#define DFLT_BDIR ".,$TEMP,c:\\tmp,c:\\temp" 
#endif
#if !defined(DFLT_VDIR)
#define DFLT_VDIR "$VIM/vimfiles/view" 
#endif
#if !defined(DFLT_DIR)
#define DFLT_DIR ".,$TEMP,c:\\tmp,c:\\temp" 
#endif
#define DFLT_ERRORFILE "errors.err"
#define DFLT_RUNTIMEPATH "$HOME/vimfiles,$VIM/vimfiles,$VIMRUNTIME,$VIM/vimfiles/after,$HOME/vimfiles/after"
#define CLEAN_RUNTIMEPATH "$VIM/vimfiles,$VIMRUNTIME,$VIM/vimfiles/after"
#define CASE_INSENSITIVE_FILENAME 
#define SPACE_IN_FILENAME
#define BACKSLASH_IN_FILENAME
#define USE_CRNL 
#define HAVE_DUP 
#define HAVE_ST_MODE 
