#include "strbuf.h"
#define MAX_BOUNDARIES 5
struct mailinfo {
FILE *input;
FILE *output;
FILE *patchfile;
struct strbuf name;
struct strbuf email;
int keep_subject;
int keep_non_patch_brackets_in_subject;
int add_message_id;
int use_scissors;
int use_inbody_headers;
const char *metainfo_charset;
struct strbuf *content[MAX_BOUNDARIES];
struct strbuf **content_top;
struct strbuf charset;
unsigned int format_flowed:1;
unsigned int delsp:1;
char *message_id;
enum {
TE_DONTCARE, TE_QP, TE_BASE64
} transfer_encoding;
int patch_lines;
int filter_stage; 
int header_stage; 
struct strbuf inbody_header_accum;
struct strbuf **p_hdr_data;
struct strbuf **s_hdr_data;
struct strbuf log_message;
int input_error;
};
void setup_mailinfo(struct mailinfo *);
int mailinfo(struct mailinfo *, const char *msg, const char *patch);
void clear_mailinfo(struct mailinfo *);
