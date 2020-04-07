












#include "raylib.h"

#include <stdlib.h> 
#include <math.h> 
#include <string.h> 

#define MAX_SAMPLES 512
#define MAX_SAMPLES_PER_UPDATE 4096

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [audio] example - raw audio streaming");

InitAudioDevice(); 


AudioStream stream = InitAudioStream(22050, 16, 1);


short *data = (short *)malloc(sizeof(short)*MAX_SAMPLES);


short *writeBuf = (short *)malloc(sizeof(short)*MAX_SAMPLES_PER_UPDATE);

PlayAudioStream(stream); 


Vector2 mousePosition = { -100.0f, -100.0f };


float frequency = 440.0f;


float oldFrequency = 1.0f;


int readCursor = 0;


int waveLength = 1;

Vector2 position = { 0, 0 };

SetTargetFPS(30); 



while (!WindowShouldClose()) 
{




mousePosition = GetMousePosition();

if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
{
float fp = (float)(mousePosition.y);
frequency = 40.0f + (float)(fp);
}



if (frequency != oldFrequency)
{

int oldWavelength = waveLength;
waveLength = (int)(22050/frequency);
if (waveLength > MAX_SAMPLES/2) waveLength = MAX_SAMPLES/2;
if (waveLength < 1) waveLength = 1;


for (int i = 0; i < waveLength*2; i++)
{
data[i] = (short)(sinf(((2*PI*(float)i/waveLength)))*32000);
}


readCursor = (int)(readCursor * ((float)waveLength / (float)oldWavelength));
oldFrequency = frequency;
}


if (IsAudioStreamProcessed(stream))
{

int writeCursor = 0;

while (writeCursor < MAX_SAMPLES_PER_UPDATE)
{

int writeLength = MAX_SAMPLES_PER_UPDATE-writeCursor;


int readLength = waveLength-readCursor;

if (writeLength > readLength) writeLength = readLength;


memcpy(writeBuf + writeCursor, data + readCursor, writeLength*sizeof(short));


readCursor = (readCursor + writeLength) % waveLength;

writeCursor += writeLength;
}


UpdateAudioStream(stream, writeBuf, MAX_SAMPLES_PER_UPDATE);
}




BeginDrawing();

ClearBackground(RAYWHITE);

DrawText(FormatText("sine frequency: %i",(int)frequency), GetScreenWidth() - 220, 10, 20, RED);
DrawText("click mouse button to change frequency", 10, 10, 20, DARKGRAY);


for (int i = 0; i < screenWidth; i++)
{
position.x = i;
position.y = 250 + 50*data[i*MAX_SAMPLES/screenWidth]/32000;

DrawPixelV(position, RED);
}

EndDrawing();

}



free(data); 
free(writeBuf); 

CloseAudioStream(stream); 
CloseAudioDevice(); 

CloseWindow(); 


return 0;
}
