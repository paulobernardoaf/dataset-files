













#include "raylib.h"

#include "pthread.h" 

#include <stdatomic.h> 

#include <time.h> 



static atomic_bool dataLoaded = ATOMIC_VAR_INIT(false); 
static void *LoadDataThread(void *arg); 

static int dataProgress = 0; 

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [core] example - loading thread");

pthread_t threadId; 

enum { STATE_WAITING, STATE_LOADING, STATE_FINISHED } state = STATE_WAITING;
int framesCounter = 0;

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{


switch (state)
{
case STATE_WAITING:
{
if (IsKeyPressed(KEY_ENTER))
{
int error = pthread_create(&threadId, NULL, &LoadDataThread, NULL);
if (error != 0) TraceLog(LOG_ERROR, "Error creating loading thread");
else TraceLog(LOG_INFO, "Loading thread initialized successfully");

state = STATE_LOADING;
}
} break;
case STATE_LOADING:
{
framesCounter++;
if (atomic_load(&dataLoaded))
{
framesCounter = 0;
state = STATE_FINISHED;
}
} break;
case STATE_FINISHED:
{
if (IsKeyPressed(KEY_ENTER))
{

atomic_store(&dataLoaded, false);
dataProgress = 0;
state = STATE_WAITING;
}
} break;
default: break;
}




BeginDrawing();

ClearBackground(RAYWHITE);

switch (state)
{
case STATE_WAITING: DrawText("PRESS ENTER to START LOADING DATA", 150, 170, 20, DARKGRAY); break;
case STATE_LOADING:
{
DrawRectangle(150, 200, dataProgress, 60, SKYBLUE);
if ((framesCounter/15)%2) DrawText("LOADING DATA...", 240, 210, 40, DARKBLUE);

} break;
case STATE_FINISHED:
{
DrawRectangle(150, 200, 500, 60, LIME);
DrawText("DATA LOADED!", 250, 210, 40, GREEN);

} break;
default: break;
}

DrawRectangleLines(150, 200, 500, 60, DARKGRAY);

EndDrawing();

}



CloseWindow(); 


return 0;
}


static void *LoadDataThread(void *arg)
{
int timeCounter = 0; 
clock_t prevTime = clock(); 


while (timeCounter < 5000)
{
clock_t currentTime = clock() - prevTime;
timeCounter = currentTime*1000/CLOCKS_PER_SEC;



dataProgress = timeCounter/10;
}


atomic_store(&dataLoaded, true);

return NULL;
}
