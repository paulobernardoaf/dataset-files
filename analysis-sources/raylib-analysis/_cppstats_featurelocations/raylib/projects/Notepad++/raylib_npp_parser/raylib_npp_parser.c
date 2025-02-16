






























#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 512

int main(int argc, char *argv[])
{
if (argc > 1)
{
FILE *rFile = fopen(argv[1], "rt");
FILE *rxmlFile = fopen("raylib_npp.xml", "wt");

if ((rFile == NULL) || (rxmlFile == NULL))
{
printf("File could not be opened.\n");
return 0;
}

char *buffer = (char *)calloc(MAX_BUFFER_SIZE, 1);
int count = 0;

while (!feof(rFile))
{

fgets(buffer, MAX_BUFFER_SIZE, rFile);

if (buffer[0] == '/') fprintf(rxmlFile, " <!--%.*s -->\n", strlen(buffer) - 3, buffer + 2);
else if (buffer[0] == '\n') fprintf(rxmlFile, "%s", buffer); 
else if (strncmp(buffer, "RLAPI", 5) == 0) 
{
char funcType[64];
char funcTypeAux[64];
char funcName[64];
char funcDesc[256];

char params[128];
char paramType[8][16];
char paramName[8][32];

int index = 0;
char *ptr = NULL;

sscanf(buffer, "RLAPI %s %[^(]s", funcType, funcName);

if (strcmp(funcType, "const") == 0)
{ 
sscanf(buffer, "RLAPI %s %s %[^(]s", funcType, funcTypeAux, funcName);
strcat(funcType, " ");
strcat(funcType, funcTypeAux);
}

ptr = strchr(buffer, '/');
index = (int)(ptr - buffer);

sscanf(buffer + index, "%[^\n]s", funcDesc); 

ptr = strchr(buffer, '(');

if (ptr != NULL) index = (int)(ptr - buffer);
else printf("Character not found!\n");

sscanf(buffer + (index + 1), "%[^)]s", params); 


char *paramPtr[16]; 
int paramsCount = 0;
paramPtr[paramsCount] = strtok(params, ",");

if ((funcName[0] == '*') && (funcName[1] == '*')) fprintf(rxmlFile, " <KeyWord name=\"%s\" func=\"yes\">\n", funcName + 2);
else if (funcName[0] == '*') fprintf(rxmlFile, " <KeyWord name=\"%s\" func=\"yes\">\n", funcName + 1);
else fprintf(rxmlFile, " <KeyWord name=\"%s\" func=\"yes\">\n", funcName);

fprintf(rxmlFile, " <Overload retVal=\"%s\" descr=\"%s\">", funcType, funcDesc + 3);

bool paramsVoid = false;

char paramConst[8][16];

while (paramPtr[paramsCount] != NULL)
{
sscanf(paramPtr[paramsCount], "%s %s\n", paramType[paramsCount], paramName[paramsCount]);

if (strcmp(paramType[paramsCount], "void") == 0)
{
paramsVoid = true;
break;
}

if ((strcmp(paramType[paramsCount], "const") == 0) || (strcmp(paramType[paramsCount], "unsigned") == 0))
{
sscanf(paramPtr[paramsCount], "%s %s %s\n", paramConst[paramsCount], paramType[paramsCount], paramName[paramsCount]);
fprintf(rxmlFile, "\n <Param name=\"%s %s %s\" />", paramConst[paramsCount], paramType[paramsCount], paramName[paramsCount]);
}
else if (strcmp(paramType[paramsCount], "...") == 0) fprintf(rxmlFile, "\n <Param name=\"...\" />");
else fprintf(rxmlFile, "\n <Param name=\"%s %s\" />", paramType[paramsCount], paramName[paramsCount]);

paramsCount++;
paramPtr[paramsCount] = strtok(NULL, ",");
}

fprintf(rxmlFile, "%s</Overload>\n", paramsVoid ? "" : "\n ");
fprintf(rxmlFile, " </KeyWord>\n");

count++;
printf("Function processed %02i: %s\n", count, funcName);

memset(buffer, 0, MAX_BUFFER_SIZE);
}
}

free(buffer);
fclose(rFile);
fclose(rxmlFile);
}

return 0;
}