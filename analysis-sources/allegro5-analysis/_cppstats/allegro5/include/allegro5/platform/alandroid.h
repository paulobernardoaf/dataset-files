#if !defined(ALLEGRO_ANDROID)
#error bad include
#endif
#include <fcntl.h>
#if defined(__cplusplus)
extern "C" int main(int argc, char ** argv);
#else
extern int main(int argc, char ** argv);
#endif
