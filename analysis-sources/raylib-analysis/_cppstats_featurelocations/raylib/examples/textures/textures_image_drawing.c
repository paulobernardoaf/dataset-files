












#include "raylib.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [textures] example - image drawing");



Image cat = LoadImage("resources/cat.png"); 
ImageCrop(&cat, (Rectangle){ 100, 10, 280, 380 }); 
ImageFlipHorizontal(&cat); 
ImageResize(&cat, 150, 200); 

Image parrots = LoadImage("resources/parrots.png"); 


ImageDraw(&parrots, cat, (Rectangle){ 0, 0, cat.width, cat.height }, (Rectangle){ 30, 40, cat.width*1.5f, cat.height*1.5f }, WHITE);
ImageCrop(&parrots, (Rectangle){ 0, 50, parrots.width, parrots.height - 100 }); 


ImageDrawPixel(&parrots, 10, 10, RAYWHITE);
ImageDrawCircle(&parrots, 10, 10, 5, RAYWHITE);
ImageDrawRectangle(&parrots, 5, 20, 10, 10, RAYWHITE);

UnloadImage(cat); 


Font font = LoadFont("resources/custom_jupiter_crash.png");


ImageDrawTextEx(&parrots, (Vector2){ 300, 230 }, font, "PARROTS & CAT", font.baseSize, -2, WHITE);

UnloadFont(font); 

Texture2D texture = LoadTextureFromImage(parrots); 
UnloadImage(parrots); 

SetTargetFPS(60);



while (!WindowShouldClose()) 
{







BeginDrawing();

ClearBackground(RAYWHITE);

DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2 - 40, WHITE);
DrawRectangleLines(screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2 - 40, texture.width, texture.height, DARKGRAY);

DrawText("We are drawing only one texture from various images composed!", 240, 350, 10, DARKGRAY);
DrawText("Source images have been cropped, scaled, flipped and copied one over the other.", 190, 370, 10, DARKGRAY);

EndDrawing();

}



UnloadTexture(texture); 

CloseWindow(); 


return 0;
}