#include "raylib.h"
#define RNET_IMPLEMENTATION
#include "rnet.h"
float elapsed = 0.0f;
float delay = 1.0f;
bool ping = false;
bool pong = false;
bool connected = false;
bool clientConnected = false;
const char *pingmsg = "Ping!";
const char *pongmsg = "Pong!";
int msglen = 0;
SocketConfig serverConfig = { .host = "127.0.0.1", .port = "4950", .type = SOCKET_TCP, .server = true, .nonblocking = true };
SocketConfig clientConfig = { .host = "127.0.0.1", .port = "4950", .type = SOCKET_TCP, .nonblocking = true };
SocketConfig connectionConfig = { .nonblocking = true };
SocketResult *serverResult = NULL;
SocketResult *clientResult = NULL;
SocketSet *socketSet = NULL;
Socket *connection = NULL;
char receiveBuffer[512] = { 0 };
static void NetworkConnect(void)
{
if ((serverConfig.type == SOCKET_UDP) && (clientConfig.type == SOCKET_UDP))
{
ping = true;
connected = true;
} 
else 
{
if (clientConnected) 
{
int active = CheckSockets(socketSet, 0);
if (active != 0) TraceLog(LOG_INFO, "There are currently %d socket(s) with data to be processed.", active);
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
else
{
elapsed += GetFrameTime();
if (elapsed > delay)
{
if (IsSocketConnected(clientResult->socket)) clientConnected = true;
elapsed = 0.0f;
}
}
}
}
static void UpdateNetwork(void)
{
int active = CheckSockets(socketSet, 0);
if (active != 0) TraceLog(LOG_DEBUG, "There are currently %d socket(s) with data to be processed.", active);
int bytesRecv = 0;
if ((serverConfig.type == SOCKET_UDP) && (clientConfig.type == SOCKET_UDP))
{
if (IsSocketReady(clientResult->socket)) bytesRecv = SocketReceive(clientResult->socket, receiveBuffer, msglen);
if (IsSocketReady(serverResult->socket)) bytesRecv = SocketReceive(serverResult->socket, receiveBuffer, msglen);
} 
else if (IsSocketReady(connection)) bytesRecv = SocketReceive(connection, receiveBuffer, msglen);
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
if (serverConfig.type == SOCKET_UDP && clientConfig.type == SOCKET_UDP) SocketSend(clientResult->socket, pingmsg, msglen);
else SocketSend(clientResult->socket, pingmsg, msglen);
} 
else if (pong)
{
pong = false;
if (serverConfig.type == SOCKET_UDP && clientConfig.type == SOCKET_UDP) SocketSend(clientResult->socket, pongmsg, msglen);
else SocketSend(clientResult->socket, pongmsg, msglen);
}
elapsed = 0.0f;
}
}
int main(void)
{
const int screenWidth = 800;
const int screenHeight = 450;
InitWindow(screenWidth, screenHeight, "raylib [network] example - ping pong");
InitNetworkDevice(); 
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
if (!SocketListen(&serverConfig, serverResult))
{
TraceLog(LOG_WARNING, "Failed to start listen server: status %d, errno %d", serverResult->status, serverResult->socket->status);
}
}
}
}
clientResult = LoadSocketResult();
if (!SocketCreate(&clientConfig, clientResult)) 
{
TraceLog(LOG_WARNING, "Failed to open client: status %d, errno %d", clientResult->status, clientResult->socket->status);
}
else
{
if (!(clientConfig.type == SOCKET_UDP))
{
if (!SocketConnect(&clientConfig, clientResult))
{
TraceLog(LOG_WARNING, "Failed to connect to server: status %d, errno %d", clientResult->status, clientResult->socket->status);
}
}
}
socketSet = LoadSocketSet(3);
AddSocket(socketSet, serverResult->socket);
AddSocket(socketSet, clientResult->socket);
SetTargetFPS(60); 
while (!WindowShouldClose()) 
{
if (connected) UpdateNetwork();
BeginDrawing();
ClearBackground(RAYWHITE);
EndDrawing();
}
CloseNetworkDevice(); 
CloseWindow(); 
return 0;
}