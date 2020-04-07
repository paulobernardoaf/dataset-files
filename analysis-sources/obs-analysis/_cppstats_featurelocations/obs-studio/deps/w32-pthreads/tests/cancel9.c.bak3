









































































#include "test.h"
#include <windows.h>


void *
test_udp (void *arg)
{
struct sockaddr_in serverAddress;
struct sockaddr_in clientAddress;
SOCKET UDPSocket;
int addr_len;
int nbyte, bytes;
char buffer[4096];
WORD wsaVersion = MAKEWORD (2, 2);
WSADATA wsaData;

pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

if (WSAStartup (wsaVersion, &wsaData) != 0)
{
return NULL;
}

UDPSocket = socket (AF_INET, SOCK_DGRAM, 0);
if ((int)UDPSocket == -1)
{
printf ("Server: socket ERROR \n");
exit (-1);
}

serverAddress.sin_family = AF_INET;
serverAddress.sin_addr.s_addr = INADDR_ANY;
serverAddress.sin_port = htons (9003);

if (bind
(UDPSocket, (struct sockaddr *) &serverAddress,
sizeof (struct sockaddr_in)))
{
printf ("Server: ERROR can't bind UDPSocket");
exit (-1);
}

addr_len = sizeof (struct sockaddr);

nbyte = 512;

bytes =
recvfrom (UDPSocket, (char *) buffer, nbyte, 0,
(struct sockaddr *) &clientAddress, &addr_len);

closesocket (UDPSocket);
WSACleanup ();

return NULL;
}


void *
test_sleep (void *arg)
{
pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

Sleep (1000);
return NULL;

}

void *
test_wait (void *arg)
{
HANDLE hEvent;
DWORD dwEvent;

pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);

dwEvent = WaitForSingleObject (hEvent, 1000); 

return NULL;
}


int
main ()
{
pthread_t t;
void *result;

if (pthread_win32_test_features_np (PTW32_ALERTABLE_ASYNC_CANCEL))
{
printf ("Cancel sleeping thread.\n");
assert (pthread_create (&t, NULL, test_sleep, NULL) == 0);

Sleep (100);
assert (pthread_cancel (t) == 0);
assert (pthread_join (t, &result) == 0);
assert (result == PTHREAD_CANCELED && "test_sleep" != NULL);

printf ("Cancel waiting thread.\n");
assert (pthread_create (&t, NULL, test_wait, NULL) == 0);

Sleep (100);
assert (pthread_cancel (t) == 0);
assert (pthread_join (t, &result) == 0);
assert (result == PTHREAD_CANCELED && "test_wait");

printf ("Cancel blocked thread (blocked on network I/O).\n");
assert (pthread_create (&t, NULL, test_udp, NULL) == 0);

Sleep (100);
assert (pthread_cancel (t) == 0);
assert (pthread_join (t, &result) == 0);
assert (result == PTHREAD_CANCELED && "test_udp" != NULL);
}
else
{
printf ("Alertable async cancel not available.\n");
}




return 0;
}
