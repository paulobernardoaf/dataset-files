#include <d3d9.h>
#if defined ALLEGRO_CFG_SHADER_HLSL && defined __cplusplus
#include <d3dx9.h>
#endif
#include "allegro5/platform/alplatf.h"
#define ALLEGRO_DIRECT3D ALLEGRO_DIRECT3D_INTERNAL
#if defined(__cplusplus)
extern "C" {
#endif
AL_FUNC(LPDIRECT3DDEVICE9, al_get_d3d_device, (ALLEGRO_DISPLAY *));
AL_FUNC(LPDIRECT3DTEXTURE9, al_get_d3d_system_texture, (ALLEGRO_BITMAP *));
AL_FUNC(LPDIRECT3DTEXTURE9, al_get_d3d_video_texture, (ALLEGRO_BITMAP *));
AL_FUNC(bool, al_have_d3d_non_pow2_texture_support, (void));
AL_FUNC(bool, al_have_d3d_non_square_texture_support, (void));
AL_FUNC(void, al_get_d3d_texture_position, (ALLEGRO_BITMAP *bitmap, int *u, int *v));
AL_FUNC(bool, al_get_d3d_texture_size, (ALLEGRO_BITMAP *bitmap, int *width, int *height));
AL_FUNC(bool, al_is_d3d_device_lost, (ALLEGRO_DISPLAY *display));
AL_FUNC(void, al_set_d3d_device_release_callback, (void (*callback)(ALLEGRO_DISPLAY *display)));
AL_FUNC(void, al_set_d3d_device_restore_callback, (void (*callback)(ALLEGRO_DISPLAY *display)));
#if defined(__cplusplus)
}
#endif
