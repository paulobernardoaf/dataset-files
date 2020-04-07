#define yyconst const
typedef void* yyscan_t;
typedef struct yy_buffer_state *YY_BUFFER_STATE;
int yylex_init (yyscan_t* scanner);
YY_BUFFER_STATE yy_scan_string (yyconst char *yy_str ,yyscan_t yyscanner );
YY_BUFFER_STATE yy_scan_bytes (yyconst char *bytes, int, yyscan_t yyscanner );
int yylex_destroy (yyscan_t yyscanner );
void yy_delete_buffer (YY_BUFFER_STATE b , yyscan_t yyscanner);
