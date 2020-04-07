




















#include "server_setup.h"

#include "getpart.h"

#include "curl_printf.h"


#include "memdebug.h"

int main(int argc, char **argv)
{
char *part;
size_t partlen;

if(argc< 3) {
printf("./testpart main sub\n");
}
else {
int rc = getpart(&part, &partlen, argv[1], argv[2], stdin);
size_t i;
if(rc)
return rc;
for(i = 0; i < partlen; i++)
printf("%c", part[i]);
free(part);
}
return 0;
}
