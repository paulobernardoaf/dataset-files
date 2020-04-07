

#include <r_socket.h>
#if __UNIX__

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>

static int set_interface_attribs (int fd, int speed, int parity) {
struct termios tty;
memset (&tty, 0, sizeof tty);
if (tcgetattr (fd, &tty) != 0) {
return -1;
}
cfsetospeed (&tty, speed);
cfsetispeed (&tty, speed);

tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; 


tty.c_iflag &= ~IGNBRK; 
tty.c_lflag = 0; 

tty.c_oflag = 0; 
tty.c_cc[VMIN] = 0; 
tty.c_cc[VTIME] = 5; 

tty.c_iflag &= ~(IXON | IXOFF | IXANY); 

tty.c_cflag |= (CLOCAL | CREAD);

tty.c_cflag &= ~(PARENB | PARODD); 
tty.c_cflag |= parity;
tty.c_cflag &= ~CSTOPB;
#if defined(CRTSCTS)
tty.c_cflag &= ~CRTSCTS;
#endif

if (tcsetattr (fd, TCSANOW, &tty) != 0) {
return -1;
}
return 0;
}

R_API int r_socket_connect_serial(RSocket *sock, const char *path, int speed, int parity) {
int fd = open (path, O_RDWR | O_SYNC | O_BINARY, 0); 
if (fd == -1) {
return -1;
}
if (speed < 1) {
speed = 9600; 
}
(void)set_interface_attribs (fd, speed, parity);
sock->fd = fd;
return fd;
}

#else 

R_API int r_socket_connect_serial(RSocket *sock, const char *path, int speed, int parity) {
return -1;
}
#endif
