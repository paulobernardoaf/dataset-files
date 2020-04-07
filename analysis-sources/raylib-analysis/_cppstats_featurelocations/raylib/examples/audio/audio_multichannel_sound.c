












#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [audio] example - Multichannel sound playing");

InitAudioDevice(); 

Sound fxWav = LoadSound("resources/sound.wav"); 
Sound fxOgg = LoadSound("resources/tanatana.ogg"); 

SetSoundVolume(fxWav, 0.2);

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


if (IsKeyPressed(KEY_ENTER)) PlaySoundMulti(fxWav); 
if (IsKeyPressed(KEY_SPACE)) PlaySoundMulti(fxOgg); 




BeginDrawing();

ClearBackground(RAYWHITE);

DrawText("MULTICHANNEL SOUND PLAYING", 20, 20, 20, GRAY);
DrawText("Press SPACE to play new ogg instance!", 200, 120, 20, LIGHTGRAY);
DrawText("Press ENTER to play new wav instance!", 200, 180, 20, LIGHTGRAY);

DrawText(FormatText("CONCURRENT SOUNDS PLAYING: %02i", GetSoundsPlaying()), 220, 280, 20, RED);

EndDrawing();

}



StopSoundMulti(); 

UnloadSound(fxWav); 
UnloadSound(fxOgg); 

CloseAudioDevice(); 

CloseWindow(); 


return 0;
}
