#if !defined(COMPAT_TERMINAL_H)
#define COMPAT_TERMINAL_H

char *git_terminal_prompt(const char *prompt, int echo);


int read_key_without_echo(struct strbuf *buf);

#endif 
