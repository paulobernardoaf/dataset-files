










#include "raylib.h"

#define RNET_IMPLEMENTATION
#include "rnet.h"

int main(void)
{


const int screenWidth = 800;
const int screenHeight = 450;

InitWindow(screenWidth, screenHeight, "raylib [network] example - resolve host");

InitNetworkDevice(); 

char buffer[ADDRESS_IPV6_ADDRSTRLEN];
unsigned short port = 0;

AddressInformation *address = LoadAddressList(1);







int count = ResolveHost(NULL, "5210", ADDRESS_TYPE_IPV4, 0, address);

if (count > 0)
{
GetAddressHostAndPort(address[0], buffer, &port);
TraceLog(LOG_INFO, "Resolved to ip %s::%d", buffer, port);
}

SetTargetFPS(60); 



while (!WindowShouldClose()) 
{







BeginDrawing();

ClearBackground(RAYWHITE);



EndDrawing();

}



CloseNetworkDevice(); 

CloseWindow(); 


return 0;
}