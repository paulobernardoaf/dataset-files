#include "raylib.h"
#define RNET_IMPLEMENTATION
#include "rnet.h"
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [network] example - tcp server");
InitNetworkDevice(); 
const char *pingmsg = "Ping!";
const char *pongmsg = "Pong!";
bool ping = false;
bool pong = false;
float elapsed = 0.0f;
float delay = 1.0f;
bool connected = false;
SocketConfig serverConfig = {
.host = "127.0.0.1", 
.port = "4950", 
.type = SOCKET_TCP,
.server = true,
.nonblocking = true
};
SocketConfig connectionConfig = { .nonblocking = true };
Socket *connection = NULL;
SocketSet *socketSet = NULL;
SocketResult *serverResult = NULL;
char receiveBuffer[512] = { 0 };
serverResult = LoadSocketResult();
if (!SocketCreate(&serverConfig, serverResult)) 
{
TraceLog(LOG_WARNING, "Failed to open server: status %d, errno %d", serverResult->status, serverResult->socket->status);
} 
else
{
if (!SocketBind(&serverConfig, serverResult))
{
TraceLog(LOG_WARNING, "Failed to bind server: status %d, errno %d", serverResult->status, serverResult->socket->status);
} 
else
{
if (!(serverConfig.type == SOCKET_UDP))
{
if (!SocketListen(&serverConfig, serverResult)) TraceLog(LOG_WARNING, "Failed to start listen server: status %d, errno %d", serverResult->status, serverResult->socket->status);
}
}
}
socketSet = LoadSocketSet(2);
AddSocket(socketSet, serverResult->socket);
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
if (connected)
{
int active = CheckSockets(socketSet, 0);
if (active != 0) TraceLog(LOG_DEBUG, "There are currently %d socket(s) with data to be processed.", active);
int bytesRecv = 0;
if (IsSocketReady(connection)) bytesRecv = SocketReceive(connection, receiveBuffer, strlen(pingmsg) + 1);
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
SocketSend(connection, pingmsg, strlen(pingmsg) + 1);
} 
else if (pong)
{
pong = false;
SocketSend(connection, pongmsg, strlen(pingmsg) + 1);
}
elapsed = 0.0f;
}
}
else
{
int active = CheckSockets(socketSet, 0);
if (active != 0) TraceLog(LOG_DEBUG, "There are currently %d socket(s) with data to be processed.", active);
if (active > 0) 
{
if ((connection = SocketAccept(serverResult->socket, &connectionConfig)) != NULL) 
{
AddSocket(socketSet, connection);
connected = true;
ping = true;
}
}
}
BeginDrawing();
ClearBackground(RAYWHITE);
EndDrawing();
}
CloseNetworkDevice(); 
CloseWindow(); 
return 0;
}