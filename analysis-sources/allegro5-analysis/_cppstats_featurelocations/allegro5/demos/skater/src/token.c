#include <allegro5/allegro.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"


int Error, Lines;
char ErrorText[1024];
struct Tok Token;
ALLEGRO_FILE *input = NULL;

static int my_ungetc_c = -1;



















































#define whitespace(v) ((v == ' ') || (v == '\r') || (v == '\n') || (v == '\t'))
#define breaker(v) (whitespace(v) || (v == '{') || (v == '}') || (v == ','))






















static int my_fgetc(ALLEGRO_FILE * f)
{
static int LastChar = '\0';
int r;
char TestChar;

if (my_ungetc_c != -1) {
r = my_ungetc_c;
my_ungetc_c = -1;
}
else
r = al_fgetc(f);

if (r == '\n' || r == '\r') {
TestChar = (r == '\n') ? '\r' : '\n';

if (LastChar != TestChar) {
Lines++;
LastChar = r;
} else
LastChar = '\0';
} else
LastChar = r;

return r;
}




static void my_ungetc(int c)
{
my_ungetc_c = c;
}








static void GetTokenInner(void)
{
char *Ptr = Token.Text;


do {
*Ptr = my_fgetc(input);
} while (whitespace(*Ptr));




if (*Ptr == '{') {
Token.Type = TK_OPENBRACE;
return;
}

if (*Ptr == '}') {
Token.Type = TK_CLOSEBRACE;
return;
}

if (*Ptr == ',') {
Token.Type = TK_COMMA;
return;
}

if (*Ptr == '\"') {
Token.Type = TK_STRING;



do {
*Ptr++ = my_fgetc(input);
} while (Ptr[-1] != '\"' && !al_feof(input)
&& (Ptr - Token.Text) < 256);
Ptr[-1] = '\0';
if (al_feof(input) || (strlen(Token.Text) == 255))
Error = 1;
return;
}

if (*Ptr == '#') {
Token.Type = TK_COMMENT;

do {
*Ptr++ = my_fgetc(input);
} while (Ptr[-1] != '\r' && Ptr[-1] != '\n');
Ptr[-1] = '\0';
return;
}

Ptr++;
*Ptr = '\0';



while (1) {
char newc = my_fgetc(input); 



if (breaker(newc) || al_feof(input)) {


char *eptr;

my_ungetc(newc);
Token.IQuantity = strtol(Token.Text, &eptr, 0);
if (!*eptr) {
Token.Type = TK_NUMBER;
Token.FQuantity = (float)Token.IQuantity;
return;
}



Token.FQuantity = (float)strtod(Token.Text, &eptr);
if (!*eptr) {
Token.Type = TK_NUMBER;
Token.IQuantity = (int)Token.FQuantity;
return;
}



Token.Type = TK_UNKNOWN;
return;
}

*Ptr++ = newc;
*Ptr = '\0';
}
}






void GetToken(void)
{
while (1) {
GetTokenInner();
if (Token.Type != TK_COMMENT)
return;
}
}







void ExpectToken(enum TokenTypes Type)
{
GetToken();
if (Token.Type != Type)
Error = 1;
}
