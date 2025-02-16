



















#include "raylib.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else 
#define GLSL_VERSION 100
#endif

#define MAX_PALETTES 3
#define COLORS_PER_PALETTE 8
#define VALUES_PER_COLOR 3

static const int palettes[MAX_PALETTES][COLORS_PER_PALETTE*VALUES_PER_COLOR] = {
{ 
0, 0, 0,
255, 0, 0,
0, 255, 0,
0, 0, 255,
0, 255, 255,
255, 0, 255,
255, 255, 0,
255, 255, 255,
},
{ 
4, 12, 6,
17, 35, 24,
30, 58, 41,
48, 93, 66,
77, 128, 97,
137, 162, 87,
190, 220, 127,
238, 255, 204,
},
{ 
21, 25, 26,
138, 76, 88,
217, 98, 117,
230, 184, 193,
69, 107, 115,
75, 151, 166,
165, 189, 194,
255, 245, 247,
}
};

static const char *paletteText[] = {
"3-BIT RGB",
"AMMO-8 (GameBoy-like)",
"RKBV (2-strip film)"
};

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [shaders] example - color palette switch");




Shader shader = LoadShader(0, FormatText("resources/shaders/glsl%i/palette_switch.fs", GLSL_VERSION));



int paletteLoc = GetShaderLocation(shader, "palette");

int currentPalette = 0;
int lineHeight = screenHeight/COLORS_PER_PALETTE;

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


if (IsKeyPressed(KEY_RIGHT)) currentPalette++;
else if (IsKeyPressed(KEY_LEFT)) currentPalette--;

if (currentPalette >= MAX_PALETTES) currentPalette = 0;
else if (currentPalette < 0) currentPalette = MAX_PALETTES - 1;



SetShaderValueV(shader, paletteLoc, palettes[currentPalette], UNIFORM_IVEC3, COLORS_PER_PALETTE);




BeginDrawing();

ClearBackground(RAYWHITE);

BeginShaderMode(shader);

for (int i = 0; i < COLORS_PER_PALETTE; i++)
{


DrawRectangle(0, lineHeight*i, GetScreenWidth(), lineHeight, (Color){ i, i, i, 255 });
}

EndShaderMode();

DrawText("< >", 10, 10, 30, DARKBLUE);
DrawText("CURRENT PALETTE:", 60, 15, 20, RAYWHITE);
DrawText(paletteText[currentPalette], 300, 15, 20, RED);

DrawFPS(700, 15);

EndDrawing();

}



UnloadShader(shader); 

CloseWindow(); 


return 0;
}
