#if !defined(__al_included_allegro5_aintern_prim_soft_h)
#define __al_included_allegro5_aintern_prim_soft_h

struct ALLEGRO_BITMAP;
struct ALLEGRO_VERTEX;

#if defined(__cplusplus)
extern "C" {
#endif

int _al_draw_prim_soft(ALLEGRO_BITMAP* texture, const void* vtxs, const ALLEGRO_VERTEX_DECL* decl, int start, int end, int type);
int _al_draw_prim_indexed_soft(ALLEGRO_BITMAP* texture, const void* vtxs, const ALLEGRO_VERTEX_DECL* decl, const int* indices, int num_vtx, int type);

void _al_line_2d(ALLEGRO_BITMAP* texture, ALLEGRO_VERTEX* v1, ALLEGRO_VERTEX* v2);
void _al_point_2d(ALLEGRO_BITMAP* texture, ALLEGRO_VERTEX* v);

#if defined(__cplusplus)
}
#endif

#endif
