#if !defined(__al_included_allegro5_aintern_tls_h)
#define __al_included_allegro5_aintern_tls_h

#if defined(__cplusplus)
extern "C" {
#endif


void _al_tls_init_once(void);
void _al_reinitialize_tls_values(void);

int *_al_tls_get_dtor_owner_count(void);


#if defined(__cplusplus)
}
#endif

#endif


