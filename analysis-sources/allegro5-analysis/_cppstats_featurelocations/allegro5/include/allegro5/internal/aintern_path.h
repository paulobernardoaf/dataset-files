#if !defined(__al_included_allegro5_aintern_path_h)
#define __al_included_allegro5_aintern_path_h

struct ALLEGRO_PATH {
ALLEGRO_USTR *drive;
ALLEGRO_USTR *filename;
_AL_VECTOR segments; 
ALLEGRO_USTR *basename;
ALLEGRO_USTR *full_string;
};

#endif


