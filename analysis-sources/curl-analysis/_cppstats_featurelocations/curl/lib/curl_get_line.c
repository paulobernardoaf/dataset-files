





















#include "curl_setup.h"

#include "curl_get_line.h"
#include "curl_memory.h"

#include "memdebug.h"





char *Curl_get_line(char *buf, int len, FILE *input)
{
bool partial = FALSE;
while(1) {
char *b = fgets(buf, len, input);
if(b) {
size_t rlen = strlen(b);
if(rlen && (b[rlen-1] == '\n')) {
if(partial) {
partial = FALSE;
continue;
}
return b;
}

partial = TRUE;
}
else
break;
}
return NULL;
}
