#define L_C 0 
#define L_CC 1 
#define L_MAKE 2 
#define L_PLI 3 
#define L_MACH 4 
#define L_ENG 5 
#define L_PAS 6 
#define L_MAIL 7 
#define L_NEWS 8 
#define L_JAVA 9 
#define L_HTML 10 
#define L_BCPL 11 
#define L_M4 12 
#define L_PO 13 
static const struct {
char human[48];
char mime[16];
} types[] = {
{ "C program", "text/x-c", },
{ "C++ program", "text/x-c++" },
{ "make commands", "text/x-makefile" },
{ "PL/1 program", "text/x-pl1" },
{ "assembler program", "text/x-asm" },
{ "English", "text/plain" },
{ "Pascal program", "text/x-pascal" },
{ "mail", "text/x-mail" },
{ "news", "text/x-news" },
{ "Java program", "text/x-java" },
{ "HTML document", "text/html", },
{ "BCPL program", "text/x-bcpl" },
{ "M4 macro language pre-processor", "text/x-m4" },
{ "PO (gettext message catalogue)", "text/x-po" },
{ "cannot happen error on names.h/types", "error/x-error" }
};
static const struct names {
char name[14];
short type;
} names[] = {
{"msgid", L_PO},
{"dnl", L_M4},
{"import", L_JAVA},
{"\"libhdr\"", L_BCPL},
{"\"LIBHDR\"", L_BCPL},
{"//", L_CC},
{"template", L_CC},
{"virtual", L_CC},
{"class", L_CC},
{"public:", L_CC},
{"private:", L_CC},
{"/*", L_C}, 
{"#include", L_C},
{"char", L_C},
{"The", L_ENG},
{"the", L_ENG},
{"double", L_C},
{"extern", L_C},
{"float", L_C},
{"struct", L_C},
{"union", L_C},
{"CFLAGS", L_MAKE},
{"LDFLAGS", L_MAKE},
{"all:", L_MAKE},
{".PRECIOUS", L_MAKE},
{".ascii", L_MACH},
{".asciiz", L_MACH},
{".byte", L_MACH},
{".even", L_MACH},
{".globl", L_MACH},
{".text", L_MACH},
{"clr", L_MACH},
{"(input,", L_PAS},
{"program", L_PAS},
{"record", L_PAS},
{"dcl", L_PLI},
{"Received:", L_MAIL},
{">From", L_MAIL},
{"Return-Path:",L_MAIL},
{"Cc:", L_MAIL},
{"Newsgroups:", L_NEWS},
{"Path:", L_NEWS},
{"Organization:",L_NEWS},
{"href=", L_HTML},
{"HREF=", L_HTML},
{"<body", L_HTML},
{"<BODY", L_HTML},
{"<html", L_HTML},
{"<HTML", L_HTML},
{"<!--", L_HTML},
};
#define NNAMES (sizeof(names)/sizeof(struct names))
