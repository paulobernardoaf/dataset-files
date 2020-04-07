










#include "raylib.h"

#define NUM_FRAMES 3 

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [textures] example - sprite button");

InitAudioDevice(); 

Sound fxButton = LoadSound("resources/buttonfx.wav"); 
Texture2D button = LoadTexture("resources/button.png"); 


int frameHeight = button.height/NUM_FRAMES;
Rectangle sourceRec = { 0, 0, button.width, frameHeight };


Rectangle btnBounds = { screenWidth/2 - button.width/2, screenHeight/2 - button.height/NUM_FRAMES/2, button.width, frameHeight };

int btnState = 0; 
bool btnAction = false; 

Vector2 mousePoint = { 0.0f, 0.0f };

SetTargetFPS(60);



while (!WindowShouldClose()) 
{


mousePoint = GetMousePosition();
btnAction = false;


if (CheckCollisionPointRec(mousePoint, btnBounds))
{
if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) btnState = 2;
else btnState = 1;

if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) btnAction = true;
}
else btnState = 0;

if (btnAction)
{
PlaySound(fxButton);


}


sourceRec.y = btnState*frameHeight;




BeginDrawing();

ClearBackground(RAYWHITE);

DrawTextureRec(button, sourceRec, (Vector2){ btnBounds.x, btnBounds.y }, WHITE); 

EndDrawing();

}



UnloadTexture(button); 
UnloadSound(fxButton); 

CloseAudioDevice(); 

CloseWindow(); 


return 0;
}