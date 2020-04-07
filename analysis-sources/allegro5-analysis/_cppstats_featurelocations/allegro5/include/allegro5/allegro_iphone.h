












#if !defined(A5_IPHONE_ALLEGRO_H)
#define A5_IPHONE_ALLEGRO_H

#if defined(__cplusplus)
extern "C" {
#endif





enum ALLEGRO_IPHONE_STATUSBAR_ORIENTATION {
ALLEGRO_IPHONE_STATUSBAR_ORIENTATION_PORTRAIT = 0,
ALLEGRO_IPHONE_STATUSBAR_ORIENTATION_PORTRAIT_UPSIDE_DOWN,
ALLEGRO_IPHONE_STATUSBAR_ORIENTATION_LANDSCAPE_RIGHT,
ALLEGRO_IPHONE_STATUSBAR_ORIENTATION_LANDSCAPE_LEFT
};

AL_FUNC(void, al_iphone_set_statusbar_orientation, (int orientation));
AL_FUNC(double, al_iphone_get_last_shake_time, (void));
AL_FUNC(float, al_iphone_get_battery_level, (void));

#if defined(__cplusplus)
}
#endif

#endif 
