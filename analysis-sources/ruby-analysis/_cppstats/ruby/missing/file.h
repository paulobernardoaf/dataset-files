#include <fcntl.h>
#if !defined(L_SET)
#define L_SET 0 
#define L_CURR 1 
#define L_INCR 1 
#define L_XTND 2 
#endif
#if !defined(R_OK)
#define R_OK 4 
#define W_OK 2 
#define X_OK 1 
#define F_OK 0 
#endif
