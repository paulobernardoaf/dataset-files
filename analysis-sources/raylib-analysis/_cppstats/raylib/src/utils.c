#include "raylib.h" 
#if !defined(EXTERNAL_CONFIG_FLAGS)
#include "config.h" 
#endif
#include "utils.h"
#if defined(PLATFORM_ANDROID)
#include <errno.h> 
#include <android/log.h> 
#include <android/asset_manager.h> 
#endif
#include <stdlib.h> 
#include <stdio.h> 
#include <stdarg.h> 
#include <string.h> 
#define MAX_TRACELOG_BUFFER_SIZE 128 
#define MAX_UWP_MESSAGES 512 
static int logTypeLevel = LOG_INFO; 
static int logTypeExit = LOG_ERROR; 
static TraceLogCallback logCallback = NULL; 
#if defined(PLATFORM_ANDROID)
static AAssetManager *assetManager = NULL; 
static const char *internalDataPath = NULL; 
#endif
#if defined(PLATFORM_UWP)
static int UWPOutMessageId = -1; 
static UWPMessage *UWPOutMessages[MAX_UWP_MESSAGES]; 
static int UWPInMessageId = -1; 
static UWPMessage *UWPInMessages[MAX_UWP_MESSAGES]; 
#endif
#if defined(PLATFORM_ANDROID)
FILE *funopen(const void *cookie, int (*readfn)(void *, char *, int), int (*writefn)(void *, const char *, int),
fpos_t (*seekfn)(void *, fpos_t, int), int (*closefn)(void *));
static int android_read(void *cookie, char *buf, int size);
static int android_write(void *cookie, const char *buf, int size);
static fpos_t android_seek(void *cookie, fpos_t offset, int whence);
static int android_close(void *cookie);
#endif
void SetTraceLogLevel(int logType)
{
logTypeLevel = logType;
}
void SetTraceLogExit(int logType)
{
logTypeExit = logType;
}
void SetTraceLogCallback(TraceLogCallback callback)
{
logCallback = callback;
}
void TraceLog(int logType, const char *text, ...)
{
#if defined(SUPPORT_TRACELOG)
if (logType < logTypeLevel) return;
va_list args;
va_start(args, text);
if (logCallback)
{
logCallback(logType, text, args);
va_end(args);
return;
}
#if defined(PLATFORM_ANDROID)
switch(logType)
{
case LOG_TRACE: __android_log_vprint(ANDROID_LOG_VERBOSE, "raylib", text, args); break;
case LOG_DEBUG: __android_log_vprint(ANDROID_LOG_DEBUG, "raylib", text, args); break;
case LOG_INFO: __android_log_vprint(ANDROID_LOG_INFO, "raylib", text, args); break;
case LOG_WARNING: __android_log_vprint(ANDROID_LOG_WARN, "raylib", text, args); break;
case LOG_ERROR: __android_log_vprint(ANDROID_LOG_ERROR, "raylib", text, args); break;
case LOG_FATAL: __android_log_vprint(ANDROID_LOG_FATAL, "raylib", text, args); break;
default: break;
}
#else
char buffer[MAX_TRACELOG_BUFFER_SIZE] = { 0 };
switch (logType)
{
case LOG_TRACE: strcpy(buffer, "TRACE: "); break;
case LOG_DEBUG: strcpy(buffer, "DEBUG: "); break;
case LOG_INFO: strcpy(buffer, "INFO: "); break;
case LOG_WARNING: strcpy(buffer, "WARNING: "); break;
case LOG_ERROR: strcpy(buffer, "ERROR: "); break;
case LOG_FATAL: strcpy(buffer, "FATAL: "); break;
default: break;
}
strcat(buffer, text);
strcat(buffer, "\n");
vprintf(buffer, args);
#endif
va_end(args);
if (logType >= logTypeExit) exit(1); 
#endif 
}
unsigned char *LoadFileData(const char *fileName, unsigned int *bytesRead)
{
unsigned char *data = NULL;
*bytesRead = 0;
if (fileName != NULL)
{
FILE *file = fopen(fileName, "rb");
if (file != NULL)
{
fseek(file, 0, SEEK_END);
int size = ftell(file);
fseek(file, 0, SEEK_SET);
if (size > 0)
{
data = (unsigned char *)RL_MALLOC(sizeof(unsigned char)*size);
unsigned int count = fread(data, sizeof(unsigned char), size, file);
*bytesRead = count;
if (count != size) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially loaded", fileName);
else TRACELOG(LOG_INFO, "FILEIO: [%s] File loaded successfully", fileName);
}
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read file", fileName);
fclose(file);
}
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
}
else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");
return data;
}
void SaveFileData(const char *fileName, void *data, unsigned int bytesToWrite)
{
if (fileName != NULL)
{
FILE *file = fopen(fileName, "wb");
if (file != NULL)
{
unsigned int count = fwrite(data, sizeof(unsigned char), bytesToWrite, file);
if (count == 0) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to write file", fileName);
else if (count != bytesToWrite) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially written", fileName);
else TRACELOG(LOG_INFO, "FILEIO: [%s] File saved successfully", fileName);
fclose(file);
}
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
}
else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");
}
char *LoadFileText(const char *fileName)
{
char *text = NULL;
if (fileName != NULL)
{
FILE *textFile = fopen(fileName, "rt");
if (textFile != NULL)
{
fseek(textFile, 0, SEEK_END);
int size = ftell(textFile);
fseek(textFile, 0, SEEK_SET);
if (size > 0)
{
text = (char *)RL_MALLOC(sizeof(char)*(size + 1));
int count = fread(text, sizeof(char), size, textFile);
if (count < size) text = RL_REALLOC(text, count + 1);
text[count] = '\0';
TRACELOG(LOG_INFO, "FILEIO: [%s] Text file loaded successfully", fileName);
}
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read text file", fileName);
fclose(textFile);
}
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open text file", fileName);
}
else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");
return text;
}
void SaveFileText(const char *fileName, char *text)
{
if (fileName != NULL)
{
FILE *file = fopen(fileName, "wt");
if (file != NULL)
{
int count = fprintf(file, "%s", text);
if (count == 0) TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to write text file", fileName);
else TRACELOG(LOG_INFO, "FILEIO: [%s] Text file saved successfully", fileName);
fclose(file);
}
else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open text file", fileName);
}
else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");
}
#if defined(PLATFORM_ANDROID)
void InitAssetManager(AAssetManager *manager, const char *dataPath)
{
assetManager = manager;
internalDataPath = dataPath;
}
FILE *android_fopen(const char *fileName, const char *mode)
{
if (mode[0] == 'w') 
{
#undef fopen
return fopen(TextFormat("%s/%s", internalDataPath, fileName), mode);
#define fopen(name, mode) android_fopen(name, mode)
}
else
{
AAsset *asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_UNKNOWN);
if (asset != NULL) return funopen(asset, android_read, android_write, android_seek, android_close);
else return NULL;
}
}
#endif 
#if defined(PLATFORM_ANDROID)
static int android_read(void *cookie, char *buf, int size)
{
return AAsset_read((AAsset *)cookie, buf, size);
}
static int android_write(void *cookie, const char *buf, int size)
{
TRACELOG(LOG_WARNING, "ANDROID: Failed to provide write access to APK");
return EACCES;
}
static fpos_t android_seek(void *cookie, fpos_t offset, int whence)
{
return AAsset_seek((AAsset *)cookie, offset, whence);
}
static int android_close(void *cookie)
{
AAsset_close((AAsset *)cookie);
return 0;
}
#endif 
#if defined(PLATFORM_UWP)
UWPMessage *CreateUWPMessage(void)
{
UWPMessage *msg = (UWPMessage *)RL_MALLOC(sizeof(UWPMessage));
msg->type = UWP_MSG_NONE;
Vector2 v0 = { 0, 0 };
msg->paramVector0 = v0;
msg->paramInt0 = 0;
msg->paramInt1 = 0;
msg->paramChar0 = 0;
msg->paramFloat0 = 0;
msg->paramDouble0 = 0;
msg->paramBool0 = false;
return msg;
}
void DeleteUWPMessage(UWPMessage *msg)
{
RL_FREE(msg);
}
bool UWPHasMessages(void)
{
return (UWPOutMessageId > -1);
}
UWPMessage *UWPGetMessage(void)
{
if (UWPHasMessages()) return UWPOutMessages[UWPOutMessageId--];
return NULL;
}
void UWPSendMessage(UWPMessage *msg)
{
if ((UWPInMessageId + 1) < MAX_UWP_MESSAGES)
{
UWPInMessageId++;
UWPInMessages[UWPInMessageId] = msg;
}
else TRACELOG(LOG_WARNING, "UWP: Not enough array space to register new inbound message");
}
void SendMessageToUWP(UWPMessage *msg)
{
if ((UWPOutMessageId + 1) < MAX_UWP_MESSAGES)
{
UWPOutMessageId++;
UWPOutMessages[UWPOutMessageId] = msg;
}
else TRACELOG(LOG_WARNING, "UWP: Not enough array space to register new outward message");
}
bool HasMessageFromUWP(void)
{
return UWPInMessageId > -1;
}
UWPMessage *GetMessageFromUWP(void)
{
if (HasMessageFromUWP()) return UWPInMessages[UWPInMessageId--];
return NULL;
}
#endif 
