#include "raudio.h" 
#include <stdio.h> 
#if defined(_WIN32)
#include <conio.h> 
#else
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
static int kbhit(void)
{
struct termios oldt, newt;
int ch;
int oldf;
tcgetattr(STDIN_FILENO, &oldt);
newt = oldt;
newt.c_lflag &= ~(ICANON | ECHO);
tcsetattr(STDIN_FILENO, TCSANOW, &newt);
oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
ch = getchar();
tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
fcntl(STDIN_FILENO, F_SETFL, oldf);
if (ch != EOF)
{
ungetc(ch, stdin);
return 1;
}
return 0;
}
static char getch() { return getchar(); }
#endif
#define KEY_ESCAPE 27
int main()
{
static unsigned char key = 0;
InitAudioDevice();
Sound fxWav = LoadSound("resources/audio/weird.wav"); 
Sound fxOgg = LoadSound("resources/audio/tanatana.ogg"); 
Music music = LoadMusicStream("resources/audio/guitar_noodling.ogg");
PlayMusicStream(music);
printf("\nPress s or d to play sounds...\n");
while (key != KEY_ESCAPE)
{
if (kbhit()) key = getch();
if (key == 's')
{
PlaySound(fxWav);
key = 0;
}
if (key == 'd')
{
PlaySound(fxOgg);
key = 0;
}
UpdateMusicStream(music);
}
UnloadSound(fxWav); 
UnloadSound(fxOgg); 
UnloadMusicStream(music); 
CloseAudioDevice();
return 0;
}
