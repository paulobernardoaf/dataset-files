#define USE_TERM_CONSOLE
#define USR_VIM_DIR "$BE_USER_SETTINGS/vim"
#define USR_EXRC_FILE USR_VIM_DIR "/exrc"
#define USR_EXRC_FILE2 USR_VIM_DIR "/vim/exrc"
#define USR_VIMRC_FILE USR_VIM_DIR "/vimrc"
#define USR_VIMRC_FILE2 USR_VIM_DIR "/vim/vimrc"
#define USR_GVIMRC_FILE USR_VIM_DIR "/gvimrc"
#define USR_GVIMRC_FILE2 USR_VIM_DIR "/vim/gvimrc"
#define VIMINFO_FILE USR_VIM_DIR "/viminfo"
#if defined(RUNTIME_GLOBAL)
#if defined(RUNTIME_GLOBAL_AFTER)
#define DFLT_RUNTIMEPATH USR_VIM_DIR "," RUNTIME_GLOBAL ",$VIMRUNTIME," RUNTIME_GLOBAL_AFTER "," USR_VIM_DIR "/after"
#define CLEAN_RUNTIMEPATH RUNTIME_GLOBAL ",$VIMRUNTIME," RUNTIME_GLOBAL_AFTER
#else
#define DFLT_RUNTIMEPATH USR_VIM_DIR "," RUNTIME_GLOBAL ",$VIMRUNTIME," RUNTIME_GLOBAL "/after," USR_VIM_DIR "/after"
#define CLEAN_RUNTIMEPATH RUNTIME_GLOBAL ",$VIMRUNTIME," RUNTIME_GLOBAL "/after"
#endif
#else
#define DFLT_RUNTIMEPATH USR_VIM_DIR ",$VIM/vimfiles,$VIMRUNTIME,$VIM/vimfiles/after," USR_VIM_DIR "/after"
#define CLEAN_RUNTIMEPATH "$VIM/vimfiles,$VIMRUNTIME,$VIM/vimfiles/after"
#endif
