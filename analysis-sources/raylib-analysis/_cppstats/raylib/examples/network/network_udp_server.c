#include "raylib.h"
#define RNET_IMPLEMENTATION
#include "rnet.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [network] example - udp server");
InitNetworkDevice(); 
const char *pingmsg = "Ping!";
const char *pongmsg = "Pong!";
bool ping = false;
bool pong = false;
float elapsed = 0.0f;
float delay = 1.0f;
SocketConfig serverConfig = {
.host = "127.0.0.1", 
.port = "4950", 
.server = true, 
.type = SOCKET_UDP, 
.nonblocking = true
};
SocketResult *serverResult = NULL;
SocketSet *socketSet = NULL;
char receiveBuffer[512] = { 0 };
serverResult = LoadSocketResult();
if (!SocketCreate(&serverConfig, serverResult)) TraceLog(LOG_WARNING, "Failed to open server: status %d, errno %d", serverResult->status, serverResult->socket->status);
else if (!SocketBind(&serverConfig, serverResult)) TraceLog(LOG_WARNING, "Failed to bind server: status %d, errno %d", serverResult->status, serverResult->socket->status);
socketSet = LoadSocketSet(1);
AddSocket(socketSet, serverResult->socket);
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
int active = CheckSockets(socketSet, 0);
if (active != 0) TraceLog(LOG_INFO, "There are currently %d socket(s) with data to be processed.", active);
int bytesRecv = SocketReceive(serverResult->socket, receiveBuffer, strlen(pingmsg) + 1);
if (bytesRecv > 0) 
{
if (strcmp(receiveBuffer, pingmsg) == 0) { pong = true; }
if (strcmp(receiveBuffer, pongmsg) == 0) { ping = true; }
}
elapsed += GetFrameTime();
if (elapsed > delay)
{
if (ping) 
{
ping = false;
SocketSend(serverResult->socket, pingmsg, strlen(pingmsg) + 1);
} 
else if (pong) 
{
pong = false;
SocketSend(serverResult->socket, pongmsg, strlen(pongmsg) + 1);
}
elapsed = 0.0f;
}
BeginDrawing();
ClearBackground(RAYWHITE);
EndDrawing();
}
CloseNetworkDevice(); 
CloseWindow(); 
return 0;
}