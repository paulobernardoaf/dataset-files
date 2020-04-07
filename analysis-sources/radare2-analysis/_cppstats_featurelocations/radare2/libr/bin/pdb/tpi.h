#if !defined(TPI_H)
#define TPI_H


void init_tpi_stream(STpiStream *tpi_stream);


int parse_tpi_stream(void *parsed_pdb_stream, R_STREAM_FILE *stream);


int parse_sctring(SCString *sctr, unsigned char *leaf_data, unsigned int *read_bytes, unsigned int len);



void init_scstring(SCString *cstr, unsigned int size, char *name);
#endif 
