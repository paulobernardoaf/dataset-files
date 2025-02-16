#if defined (__STDC__) || defined (ALMOST_STDC) || defined (HAVE_STRINGIZE)
#define CONCAT2(a,b) a##b
#define CONCAT3(a,b,c) a##b##c
#define CONCAT4(a,b,c,d) a##b##c##d
#define STRINGX(s) #s
#else
#define CONCAT2(a,b) ab
#define CONCAT3(a,b,c) abc
#define CONCAT4(a,b,c,d) abcd
#define STRINGX(s) "s"
#endif
#define XCONCAT2(a,b) CONCAT2(a,b)
#define XCONCAT3(a,b,c) CONCAT3(a,b,c)
#define XCONCAT4(a,b,c,d) CONCAT4(a,b,c,d)
#define XSTRING(s) STRINGX(s) 
