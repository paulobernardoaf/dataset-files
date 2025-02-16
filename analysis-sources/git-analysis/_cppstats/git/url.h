struct strbuf;
int is_url(const char *url);
int is_urlschemechar(int first_flag, int ch);
char *url_decode(const char *url);
char *url_decode_mem(const char *url, int len);
char *url_percent_decode(const char *encoded);
char *url_decode_parameter_name(const char **query);
char *url_decode_parameter_value(const char **query);
void end_url_with_slash(struct strbuf *buf, const char *url);
void str_end_url_with_slash(const char *url, char **dest);
