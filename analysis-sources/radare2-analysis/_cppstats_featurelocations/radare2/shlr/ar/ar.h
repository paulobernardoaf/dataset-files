
#if !defined(_AR_H)
#define _AR_H



R_API RBuffer *ar_open_file(const char *arname, const char *filename);
R_API int ar_close(RBuffer *b);
R_API int ar_read_at(RBuffer *b, ut64 off, void *buf, int count);
R_API int ar_write_at(RBuffer *b, ut64 off, void *buf, int count);

int ar_read(RBuffer *b, void *dest, int len);
int ar_read_until_slash(RBuffer *b, char *buffer, int limit);
int ar_read_header(RBuffer *b, char *buffer);
int ar_read_file(RBuffer *b, char *buffer, bool lookup, RList *files, const char *filename);
int ar_read_filename_table(RBuffer *b, char *buffer, RList *files, const char *filename);
#endif 
