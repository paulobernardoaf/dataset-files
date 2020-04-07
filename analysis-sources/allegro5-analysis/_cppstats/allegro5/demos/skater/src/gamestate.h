typedef struct GAMESTATE GAMESTATE;
struct GAMESTATE {
int (*id) (void);
void (*init) (void);
void (*deinit) (void);
int (*update) (void);
void (*draw) (void);
};
