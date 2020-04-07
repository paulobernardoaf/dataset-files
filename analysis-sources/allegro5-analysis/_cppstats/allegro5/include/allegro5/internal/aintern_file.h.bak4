#if !defined(__al_included_allegro5_aintern_file_h)
#define __al_included_allegro5_aintern_file_h

#if defined(__cplusplus)
extern "C" {
#endif


extern const ALLEGRO_FILE_INTERFACE _al_file_interface_stdio;

#define ALLEGRO_UNGETC_SIZE 16

struct ALLEGRO_FILE
{
const ALLEGRO_FILE_INTERFACE *vtable;
void *userdata;
unsigned char ungetc[ALLEGRO_UNGETC_SIZE];
int ungetc_len;
};

#if defined(__cplusplus)
}
#endif

#endif


