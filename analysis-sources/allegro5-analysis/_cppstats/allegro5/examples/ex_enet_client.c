#include <stdio.h>
#include <stdlib.h>
#include <enet/enet.h>
#include "allegro5/allegro.h"
#include "allegro5/allegro_primitives.h"
#include "common.c"
#include "enet_common.h"
static ENetHost* create_client(void)
{
ENetHost * client;
client = enet_host_create(NULL ,
1 ,
2 ,
57600 / 8 ,
14400 / 8 );
if (client == NULL)
abort_example("Client: Failed to create the client.\n");
return client;
}
static void disconnect_client(ENetHost *client, ENetPeer *server)
{
enet_peer_disconnect(server, 0);
ENetEvent event;
while (enet_host_service (client, &event, 3000) > 0) {
switch (event.type) {
case ENET_EVENT_TYPE_RECEIVE:
enet_packet_destroy(event.packet);
break;
case ENET_EVENT_TYPE_DISCONNECT:
puts("Client: Disconnect succeeded.");
return;
case ENET_EVENT_TYPE_NONE:
case ENET_EVENT_TYPE_CONNECT:
break;
}
}
enet_peer_reset(server);
}
static ENetPeer* connect_client(ENetHost *client, int port)
{
ENetAddress address;
ENetEvent event;
ENetPeer *server;
enet_address_set_host(&address, "localhost");
address.port = port;
server = enet_host_connect(client, &address, 2, 0);
if (server == NULL)
abort_example("Client: No available peers for initiating an ENet connection.\n");
if (enet_host_service(client, &event, 5000) > 0 &&
event.type == ENET_EVENT_TYPE_CONNECT)
{
printf("Client: Connected to %x:%u.\n",
event.peer->address.host,
event.peer->address.port);
}
else
{
enet_peer_reset(server);
abort_example("Client: Connection to server failed.");
}
return server;
}
static void send_receive(ENetHost *client)
{
ENetEvent event;
ServerMessage *msg;
while (enet_host_service(client, &event, 0) > 0) {
if (event.type == ENET_EVENT_TYPE_RECEIVE) {
msg = (ServerMessage*)event.packet->data;
switch (msg->type) {
case POSITION_UPDATE:
players[msg->player_id].x = msg->x;
players[msg->player_id].y = msg->y;
break;
case PLAYER_JOIN:
printf("Client: player #%d joined\n", msg->player_id);
players[msg->player_id].active = true;
players[msg->player_id].x = msg->x;
players[msg->player_id].y = msg->y;
players[msg->player_id].color = msg->color;
break;
case PLAYER_LEAVE:
printf("Client: player #%d left\n", msg->player_id);
players[msg->player_id].active = false;
break;
}
enet_packet_destroy(event.packet);
}
}
}
int main(int argc, char **argv)
{
ALLEGRO_DISPLAY *display;
ALLEGRO_TIMER *timer;
ALLEGRO_EVENT_QUEUE *queue;
ALLEGRO_EVENT event;
ENetHost *client;
ENetPeer *server;
bool update = true; 
bool done = false; 
int dx = 0, dy = 0; 
int port = DEFAULT_PORT;
int i;
if (argc == 2) {
port = atoi(argv[1]);
}
else if (argc > 2)
abort_example("Usage: %s [portnum]", argv[0]);
if (!al_init())
abort_example("Could not init Allegro.\n");
init_platform_specific();
al_install_keyboard();
al_init_primitives_addon();
display = al_create_display(SCREEN_W, SCREEN_H);
if (!display)
abort_example("Error creating display\n");
timer = al_create_timer(1.0 / FPS); 
queue = al_create_event_queue();
al_register_event_source(queue, al_get_keyboard_event_source());
al_register_event_source(queue, al_get_display_event_source(display));
al_register_event_source(queue, al_get_timer_event_source(timer));
al_start_timer(timer);
if (enet_initialize() != 0)
abort_example("An error occurred while initializing ENet.\n");
client = create_client();
server = connect_client(client, port);
bool direction_changed = false;
while (!done) {
al_wait_for_event(queue, &event); 
switch (event.type) {
case ALLEGRO_EVENT_DISPLAY_CLOSE:
done = true;
break;
case ALLEGRO_EVENT_KEY_DOWN:
switch (event.keyboard.keycode) {
case ALLEGRO_KEY_UP:
case ALLEGRO_KEY_W: dy -= 1; direction_changed = true; break;
case ALLEGRO_KEY_DOWN:
case ALLEGRO_KEY_S: dy += 1; direction_changed = true; break;
case ALLEGRO_KEY_LEFT:
case ALLEGRO_KEY_A: dx -= 1; direction_changed = true; break;
case ALLEGRO_KEY_RIGHT:
case ALLEGRO_KEY_D: dx += 1; direction_changed = true; break;
}
break;
case ALLEGRO_EVENT_KEY_UP:
switch (event.keyboard.keycode) {
case ALLEGRO_KEY_UP:
case ALLEGRO_KEY_W: dy += 1; direction_changed = true; break;
case ALLEGRO_KEY_DOWN:
case ALLEGRO_KEY_S: dy -= 1; direction_changed = true; break;
case ALLEGRO_KEY_LEFT:
case ALLEGRO_KEY_A: dx += 1; direction_changed = true; break;
case ALLEGRO_KEY_RIGHT:
case ALLEGRO_KEY_D: dx -= 1; direction_changed = true; break;
}
break;
case (ALLEGRO_EVENT_TIMER):
update = true;
break;
}
if (update && al_is_event_queue_empty(queue)) {
update = false;
if (direction_changed) {
direction_changed = false;
ClientMessage msg = { dx, dy };
ENetPacket *packet = enet_packet_create(&msg,
sizeof(msg),
ENET_PACKET_FLAG_RELIABLE);
enet_peer_send(server, 0, packet);
}
send_receive(client);
al_clear_to_color(al_map_rgb_f(0, 0, 0));
for (i = 0; i < MAX_PLAYER_COUNT; i++) {
if (!players[i].active) continue;
int x = players[i].x;
int y = players[i].y;
ALLEGRO_COLOR color = players[i].color;
al_draw_filled_circle(x, y, PLAYER_SIZE, color);
}
al_flip_display();
}
}
disconnect_client(client, server);
enet_host_destroy(client);
enet_deinitialize();
return 0;
}
