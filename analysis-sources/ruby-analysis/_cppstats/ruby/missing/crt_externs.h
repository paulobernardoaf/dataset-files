char ***_NSGetEnviron();
#undef environ
#define environ (*_NSGetEnviron())
