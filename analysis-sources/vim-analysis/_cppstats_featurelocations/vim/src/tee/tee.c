




























#if !defined(_MSC_VER)
#include <unistd.h>
#endif
#include <malloc.h>
#include <stdio.h>
#include <fcntl.h>

#if defined(_WIN32)
#define sysconf(x) -1
#endif

void usage(void)
{
fprintf(stderr,
"tee usage:\n\
\ttee [-a] file ... file_n\n\
\n\
\t-a\tappend to files instead of truncating\n\
\nTee reads its input, and writes to each of the specified files,\n\
as well as to the standard output.\n\
\n\
This version supplied with Vim 4.2 to make ':make' possible.\n\
For a more complete and stable version, consider getting\n\
[a port of] the GNU shellutils package.\n\
");
}






int
myfread(char *buf, int elsize , int max, FILE *fp)
{
int c;
int n = 0;

while ((n < max) && ((c = getchar()) != EOF))
{
*(buf++) = c;
n++;
if (c == '\n' || c == '\r')
break;
}
return n;
}


void
main(int argc, char *argv[])
{
int append = 0;
int numfiles;
int opt;
int maxfiles;
FILE **filepointers;
int i;
char buf[BUFSIZ];
int n;
int optind = 1;

for (i = 1; i < argc; i++)
{
if (argv[i][0] != '-')
break;
if (!strcmp(argv[i], "-a"))
append++;
else
usage();
optind++;
}

numfiles = argc - optind;

if (numfiles == 0)
{
fprintf(stderr, "doesn't make much sense using tee without any file name arguments...\n");
usage();
exit(2);
}

maxfiles = sysconf(_SC_OPEN_MAX); 
if (maxfiles < 0)
maxfiles = 10;
if (numfiles + 3 > maxfiles) 
{
fprintf(stderr, "Sorry, there is a limit of max %d files.\n", maxfiles - 3);
exit(1);
}
filepointers = calloc(numfiles, sizeof(FILE *));
if (filepointers == NULL)
{
fprintf(stderr, "Error allocating memory for %d files\n", numfiles);
exit(1);
}
for (i = 0; i < numfiles; i++)
{
filepointers[i] = fopen(argv[i+optind], append ? "ab" : "wb");
if (filepointers[i] == NULL)
{
fprintf(stderr, "Can't open \"%s\"\n", argv[i+optind]);
exit(1);
}
}
setmode(fileno(stdin), O_BINARY);
fflush(stdout); 
setmode(fileno(stdout), O_BINARY);

while ((n = myfread(buf, sizeof(char), sizeof(buf), stdin)) > 0)
{
fwrite(buf, sizeof(char), n, stdout);
fflush(stdout);
for (i = 0; i < numfiles; i++)
{
if (filepointers[i] &&
fwrite(buf, sizeof(char), n, filepointers[i]) != n)
{
fprintf(stderr, "Error writing to file \"%s\"\n", argv[i+optind]);
fclose(filepointers[i]);
filepointers[i] = NULL;
}
}
}
for (i = 0; i < numfiles; i++)
{
if (filepointers[i])
fclose(filepointers[i]);
}

exit(0);
}
