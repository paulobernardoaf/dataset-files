#include <stdio.h>
#define DBLKSIZ 4096
#define PBLKSIZ 1024
#define PAIRMAX 1008 
#define SPLTMAX 10 
#define DIRFEXT ".dir"
#define PAGFEXT ".pag"
typedef struct {
int dirf; 
int pagf; 
int flags; 
int keyptr; 
off_t maxbno; 
long curbit; 
long hmask; 
long blkptr; 
long blkno; 
long pagbno; 
char pagbuf[PBLKSIZ]; 
long dirbno; 
char dirbuf[DBLKSIZ]; 
} DBM;
#define DBM_RDONLY 0x1 
#define DBM_IOERR 0x2 
#define sdbm_rdonly(db) ((db)->flags & DBM_RDONLY)
#define sdbm_error(db) ((db)->flags & DBM_IOERR)
#define sdbm_clearerr(db) ((db)->flags &= ~DBM_IOERR) 
#define sdbm_dirfno(db) ((db)->dirf)
#define sdbm_pagfno(db) ((db)->pagf)
typedef struct {
char *dptr;
int dsize;
} datum;
extern datum nullitem;
#if defined(__STDC__)
#define proto(p) p
#else
#define proto(p) ()
#endif
#define DBM_INSERT 0
#define DBM_REPLACE 1
extern DBM *sdbm_open proto((char *, int, int));
extern void sdbm_close proto((DBM *));
extern datum sdbm_fetch proto((DBM *, datum));
extern int sdbm_delete proto((DBM *, datum));
extern int sdbm_store proto((DBM *, datum, datum, int));
extern datum sdbm_firstkey proto((DBM *));
extern datum sdbm_nextkey proto((DBM *));
extern DBM *sdbm_prep proto((char *, char *, int, int));
extern long sdbm_hash proto((char *, int));
