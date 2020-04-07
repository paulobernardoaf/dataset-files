








#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define LINELEN 200

int
main(int argc, char **argv)
{
char line[LINELEN];
char *p1, *p2;
char *p;
FILE *fd;

if (argc <= 1)
{
fprintf(stderr, "Usage: doctags docfile ... >tags\n");
exit(1);
}
printf("help-tags\ttags\t1\n");
while (--argc > 0)
{
++argv;
fd = fopen(argv[0], "r");
if (fd == NULL)
{
fprintf(stderr, "Unable to open %s for reading\n", argv[0]);
continue;
}
while (fgets(line, LINELEN, fd) != NULL)
{
p1 = strchr(line, '*'); 
while (p1 != NULL)
{
p2 = strchr(p1 + 1, '*'); 
if (p2 != NULL && p2 > p1 + 1) 
{
for (p = p1 + 1; p < p2; ++p)
if (*p == ' ' || *p == '\t' || *p == '|')
break;





if (p == p2
&& (p1 == line || p1[-1] == ' ' || p1[-1] == '\t')
&& (strchr(" \t\n\r", p[1]) != NULL
|| p[1] == '\0'))
{
*p2 = '\0';
++p1;
printf("%s\t%s\t/*", p1, argv[0]);
while (*p1)
{

if (*p1 == '\\' || *p1 == '/')
putchar('\\');
putchar(*p1);
++p1;
}
printf("*\n");
p2 = strchr(p2 + 1, '*'); 
}
}
p1 = p2;
}
}
fclose(fd);
}
return 0;
}
