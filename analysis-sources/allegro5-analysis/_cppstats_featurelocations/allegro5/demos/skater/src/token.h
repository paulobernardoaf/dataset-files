








#if !defined(__TOKENISER_H)
#define __TOKENISER_H

#include <stdio.h>
















extern int Error, Lines;
extern char ErrorText[1024];



















enum TokenTypes {
TK_OPENBRACE, TK_CLOSEBRACE, TK_COMMA,

TK_STRING, TK_NUMBER, TK_COMMENT,

TK_UNKNOWN
};















struct Tok {
enum TokenTypes Type;

char Text[256];
double FQuantity;
int IQuantity;
};

extern struct Tok Token;
extern ALLEGRO_FILE *input; 

extern void GetToken(void);
extern void ExpectToken(enum TokenTypes Type);

#endif
