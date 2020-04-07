


































#include "raylib.h" 


#if !defined(EXTERNAL_CONFIG_FLAGS)
#include "config.h" 
#endif

#include "rlgl.h" 

#include <math.h> 



















static float EaseCubicInOut(float t, float b, float c, float d); 






void DrawPixel(int posX, int posY, Color color)
{
rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2i(posX, posY);
rlVertex2i(posX + 1, posY + 1);
rlEnd();
}


void DrawPixelV(Vector2 position, Color color)
{
rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(position.x, position.y);
rlVertex2f(position.x + 1.0f, position.y + 1.0f);
rlEnd();
}


void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color)
{
rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2i(startPosX, startPosY);
rlVertex2i(endPosX, endPosY);
rlEnd();
}


void DrawLineV(Vector2 startPos, Vector2 endPos, Color color)
{
rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(startPos.x, startPos.y);
rlVertex2f(endPos.x, endPos.y);
rlEnd();
}


void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color)
{
if (startPos.x > endPos.x)
{
Vector2 tempPos = startPos;
startPos = endPos;
endPos = tempPos;
}

float dx = endPos.x - startPos.x;
float dy = endPos.y - startPos.y;

float d = sqrtf(dx*dx + dy*dy);
float angle = asinf(dy/d);

rlEnableTexture(GetShapesTexture().id);

rlPushMatrix();
rlTranslatef((float)startPos.x, (float)startPos.y, 0.0f);
rlRotatef(RAD2DEG*angle, 0.0f, 0.0f, 1.0f);
rlTranslatef(0, (thick > 1.0f)? -thick/2.0f : -1.0f, 0.0f);

rlBegin(RL_QUADS);
rlColor4ub(color.r, color.g, color.b, color.a);
rlNormal3f(0.0f, 0.0f, 1.0f);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(0.0f, 0.0f);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(0.0f, thick);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(d, thick);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(d, 0.0f);
rlEnd();
rlPopMatrix();

rlDisableTexture();
}


void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color)
{
#define LINE_DIVISIONS 24 

Vector2 previous = startPos;
Vector2 current;

for (int i = 1; i <= LINE_DIVISIONS; i++)
{


current.y = EaseCubicInOut((float)i, startPos.y, endPos.y - startPos.y, (float)LINE_DIVISIONS);
current.x = previous.x + (endPos.x - startPos.x)/ (float)LINE_DIVISIONS;

DrawLineEx(previous, current, thick, color);

previous = current;
}
}


void DrawLineStrip(Vector2 *points, int pointsCount, Color color)
{
if (pointsCount >= 2)
{
if (rlCheckBufferLimit(pointsCount)) rlglDraw();

rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);

for (int i = 0; i < pointsCount - 1; i++)
{
rlVertex2f(points[i].x, points[i].y);
rlVertex2f(points[i + 1].x, points[i + 1].y);
}
rlEnd();
}
}


void DrawCircle(int centerX, int centerY, float radius, Color color)
{
DrawCircleV((Vector2){ (float)centerX, (float)centerY }, radius, color);
}


void DrawCircleSector(Vector2 center, float radius, int startAngle, int endAngle, int segments, Color color)
{
if (radius <= 0.0f) radius = 0.1f; 


if (endAngle < startAngle)
{

int tmp = startAngle;
startAngle = endAngle;
endAngle = tmp;
}

if (segments < 4)
{

#define CIRCLE_ERROR_RATE 0.5f


float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
segments = (endAngle - startAngle)*ceilf(2*PI/th)/360;

if (segments <= 0) segments = 4;
}

float stepLength = (float)(endAngle - startAngle)/(float)segments;
float angle = startAngle;

#if defined(SUPPORT_QUADS_DRAW_MODE)
if (rlCheckBufferLimit(4*segments/2)) rlglDraw();

rlEnableTexture(GetShapesTexture().id);

rlBegin(RL_QUADS);

for (int i = 0; i < segments/2; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x, center.y);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength*2))*radius, center.y + cosf(DEG2RAD*(angle + stepLength*2))*radius);

angle += (stepLength*2);
}


if (segments%2)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x, center.y);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x, center.y);
}
rlEnd();

rlDisableTexture();
#else
if (rlCheckBufferLimit(3*segments)) rlglDraw();

rlBegin(RL_TRIANGLES);
for (int i = 0; i < segments; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlVertex2f(center.x, center.y);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);

angle += stepLength;
}
rlEnd();
#endif
}

void DrawCircleSectorLines(Vector2 center, float radius, int startAngle, int endAngle, int segments, Color color)
{
if (radius <= 0.0f) radius = 0.1f; 


if (endAngle < startAngle)
{

int tmp = startAngle;
startAngle = endAngle;
endAngle = tmp;
}

if (segments < 4)
{

#if !defined(CIRCLE_ERROR_RATE)
#define CIRCLE_ERROR_RATE 0.5f
#endif


float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
segments = (endAngle - startAngle)*ceilf(2*PI/th)/360;

if (segments <= 0) segments = 4;
}

float stepLength = (float)(endAngle - startAngle)/(float)segments;
float angle = startAngle;


bool showCapLines = true;
int limit = 2*(segments + 2);
if ((endAngle - startAngle)%360 == 0) { limit = 2*segments; showCapLines = false; }

if (rlCheckBufferLimit(limit)) rlglDraw();

rlBegin(RL_LINES);
if (showCapLines)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(center.x, center.y);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
}

for (int i = 0; i < segments; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);

angle += stepLength;
}

if (showCapLines)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(center.x, center.y);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
}
rlEnd();
}



void DrawCircleGradient(int centerX, int centerY, float radius, Color color1, Color color2)
{
if (rlCheckBufferLimit(3*36)) rlglDraw();

rlBegin(RL_TRIANGLES);
for (int i = 0; i < 360; i += 10)
{
rlColor4ub(color1.r, color1.g, color1.b, color1.a);
rlVertex2f(centerX, centerY);
rlColor4ub(color2.r, color2.g, color2.b, color2.a);
rlVertex2f(centerX + sinf(DEG2RAD*i)*radius, centerY + cosf(DEG2RAD*i)*radius);
rlColor4ub(color2.r, color2.g, color2.b, color2.a);
rlVertex2f(centerX + sinf(DEG2RAD*(i + 10))*radius, centerY + cosf(DEG2RAD*(i + 10))*radius);
}
rlEnd();
}



void DrawCircleV(Vector2 center, float radius, Color color)
{
DrawCircleSector(center, radius, 0, 360, 36, color);
}


void DrawCircleLines(int centerX, int centerY, float radius, Color color)
{
if (rlCheckBufferLimit(2*36)) rlglDraw();

rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);


for (int i = 0; i < 360; i += 10)
{
rlVertex2f(centerX + sinf(DEG2RAD*i)*radius, centerY + cosf(DEG2RAD*i)*radius);
rlVertex2f(centerX + sinf(DEG2RAD*(i + 10))*radius, centerY + cosf(DEG2RAD*(i + 10))*radius);
}
rlEnd();
}


void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color)
{
if (rlCheckBufferLimit(3*36)) rlglDraw();

rlBegin(RL_TRIANGLES);
for (int i = 0; i < 360; i += 10)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(centerX, centerY);
rlVertex2f(centerX + sinf(DEG2RAD*i)*radiusH, centerY + cosf(DEG2RAD*i)*radiusV);
rlVertex2f(centerX + sinf(DEG2RAD*(i + 10))*radiusH, centerY + cosf(DEG2RAD*(i + 10))*radiusV);
}
rlEnd();
}


void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color)
{
if (rlCheckBufferLimit(2*36)) rlglDraw();

rlBegin(RL_LINES);
for (int i = 0; i < 360; i += 10)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(centerX + sinf(DEG2RAD*i)*radiusH, centerY + cosf(DEG2RAD*i)*radiusV);
rlVertex2f(centerX + sinf(DEG2RAD*(i + 10))*radiusH, centerY + cosf(DEG2RAD*(i + 10))*radiusV);
}
rlEnd();
}

void DrawRing(Vector2 center, float innerRadius, float outerRadius, int startAngle, int endAngle, int segments, Color color)
{
if (startAngle == endAngle) return;


if (outerRadius < innerRadius)
{
float tmp = outerRadius;
outerRadius = innerRadius;
innerRadius = tmp;

if (outerRadius <= 0.0f) outerRadius = 0.1f;
}


if (endAngle < startAngle)
{

int tmp = startAngle;
startAngle = endAngle;
endAngle = tmp;
}

if (segments < 4)
{

#if !defined(CIRCLE_ERROR_RATE)
#define CIRCLE_ERROR_RATE 0.5f
#endif


float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/outerRadius, 2) - 1);
segments = (endAngle - startAngle)*ceilf(2*PI/th)/360;

if (segments <= 0) segments = 4;
}


if (innerRadius <= 0.0f)
{
DrawCircleSector(center, outerRadius, startAngle, endAngle, segments, color);
return;
}

float stepLength = (float)(endAngle - startAngle)/(float)segments;
float angle = startAngle;

#if defined(SUPPORT_QUADS_DRAW_MODE)
if (rlCheckBufferLimit(4*segments)) rlglDraw();

rlEnableTexture(GetShapesTexture().id);

rlBegin(RL_QUADS);
for (int i = 0; i < segments; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*innerRadius, center.y + cosf(DEG2RAD*angle)*innerRadius);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*outerRadius);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*innerRadius);

angle += stepLength;
}
rlEnd();

rlDisableTexture();
#else
if (rlCheckBufferLimit(6*segments)) rlglDraw();

rlBegin(RL_TRIANGLES);
for (int i = 0; i < segments; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlVertex2f(center.x + sinf(DEG2RAD*angle)*innerRadius, center.y + cosf(DEG2RAD*angle)*innerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*innerRadius);

rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*innerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*outerRadius);

angle += stepLength;
}
rlEnd();
#endif
}

void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, int startAngle, int endAngle, int segments, Color color)
{
if (startAngle == endAngle) return;


if (outerRadius < innerRadius)
{
float tmp = outerRadius;
outerRadius = innerRadius;
innerRadius = tmp;

if (outerRadius <= 0.0f) outerRadius = 0.1f;
}


if (endAngle < startAngle)
{

int tmp = startAngle;
startAngle = endAngle;
endAngle = tmp;
}

if (segments < 4)
{

#if !defined(CIRCLE_ERROR_RATE)
#define CIRCLE_ERROR_RATE 0.5f
#endif


float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/outerRadius, 2) - 1);
segments = (endAngle - startAngle)*ceilf(2*PI/th)/360;

if (segments <= 0) segments = 4;
}

if (innerRadius <= 0.0f)
{
DrawCircleSectorLines(center, outerRadius, startAngle, endAngle, segments, color);
return;
}

float stepLength = (float)(endAngle - startAngle)/(float)segments;
float angle = startAngle;

bool showCapLines = true;
int limit = 4*(segments + 1);
if ((endAngle - startAngle)%360 == 0) { limit = 4*segments; showCapLines = false; }

if (rlCheckBufferLimit(limit)) rlglDraw();

rlBegin(RL_LINES);
if (showCapLines)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*innerRadius, center.y + cosf(DEG2RAD*angle)*innerRadius);
}

for (int i = 0; i < segments; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*outerRadius);

rlVertex2f(center.x + sinf(DEG2RAD*angle)*innerRadius, center.y + cosf(DEG2RAD*angle)*innerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*innerRadius);

angle += stepLength;
}

if (showCapLines)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*innerRadius, center.y + cosf(DEG2RAD*angle)*innerRadius);
}
rlEnd();
}


void DrawRectangle(int posX, int posY, int width, int height, Color color)
{
DrawRectangleV((Vector2){ (float)posX, (float)posY }, (Vector2){ (float)width, (float)height }, color);
}



void DrawRectangleV(Vector2 position, Vector2 size, Color color)
{
DrawRectanglePro((Rectangle){ position.x, position.y, size.x, size.y }, (Vector2){ 0.0f, 0.0f }, 0.0f, color);
}


void DrawRectangleRec(Rectangle rec, Color color)
{
DrawRectanglePro(rec, (Vector2){ 0.0f, 0.0f }, 0.0f, color);
}


void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color)
{
rlEnableTexture(GetShapesTexture().id);

rlPushMatrix();
rlTranslatef(rec.x, rec.y, 0.0f);
rlRotatef(rotation, 0.0f, 0.0f, 1.0f);
rlTranslatef(-origin.x, -origin.y, 0.0f);

rlBegin(RL_QUADS);
rlNormal3f(0.0f, 0.0f, 1.0f);
rlColor4ub(color.r, color.g, color.b, color.a);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(0.0f, 0.0f);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(0.0f, rec.height);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(rec.width, rec.height);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(rec.width, 0.0f);
rlEnd();
rlPopMatrix();

rlDisableTexture();
}



void DrawRectangleGradientV(int posX, int posY, int width, int height, Color color1, Color color2)
{
DrawRectangleGradientEx((Rectangle){ (float)posX, (float)posY, (float)width, (float)height }, color1, color2, color2, color1);
}



void DrawRectangleGradientH(int posX, int posY, int width, int height, Color color1, Color color2)
{
DrawRectangleGradientEx((Rectangle){ (float)posX, (float)posY, (float)width, (float)height }, color1, color1, color2, color2);
}



void DrawRectangleGradientEx(Rectangle rec, Color col1, Color col2, Color col3, Color col4)
{
rlEnableTexture(GetShapesTexture().id);

rlPushMatrix();
rlBegin(RL_QUADS);
rlNormal3f(0.0f, 0.0f, 1.0f);


rlColor4ub(col1.r, col1.g, col1.b, col1.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(rec.x, rec.y);

rlColor4ub(col2.r, col2.g, col2.b, col2.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(rec.x, rec.y + rec.height);

rlColor4ub(col3.r, col3.g, col3.b, col3.a);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(rec.x + rec.width, rec.y + rec.height);

rlColor4ub(col4.r, col4.g, col4.b, col4.a);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(rec.x + rec.width, rec.y);
rlEnd();
rlPopMatrix();

rlDisableTexture();
}



void DrawRectangleLines(int posX, int posY, int width, int height, Color color)
{
#if defined(SUPPORT_QUADS_DRAW_MODE)
DrawRectangle(posX, posY, width, 1, color);
DrawRectangle(posX + width - 1, posY + 1, 1, height - 2, color);
DrawRectangle(posX, posY + height - 1, width, 1, color);
DrawRectangle(posX, posY + 1, 1, height - 2, color);
#else
rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2i(posX + 1, posY + 1);
rlVertex2i(posX + width, posY + 1);

rlVertex2i(posX + width, posY + 1);
rlVertex2i(posX + width, posY + height);

rlVertex2i(posX + width, posY + height);
rlVertex2i(posX + 1, posY + height);

rlVertex2i(posX + 1, posY + height);
rlVertex2i(posX + 1, posY + 1);
rlEnd();
#endif
}


void DrawRectangleLinesEx(Rectangle rec, int lineThick, Color color)
{
if (lineThick > rec.width || lineThick > rec.height)
{
if (rec.width > rec.height) lineThick = (int)rec.height/2;
else if (rec.width < rec.height) lineThick = (int)rec.width/2;
}

DrawRectangle( (int)rec.x, (int)rec.y, (int)rec.width, lineThick, color);
DrawRectangle( (int)(rec.x - lineThick + rec.width), (int)(rec.y + lineThick), lineThick, (int)(rec.height - lineThick*2.0f), color);
DrawRectangle( (int)rec.x, (int)(rec.y + rec.height - lineThick), (int)rec.width, lineThick, color);
DrawRectangle( (int)rec.x, (int)(rec.y + lineThick), lineThick, (int)(rec.height - lineThick*2), color);
}


void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color)
{

if ((roundness <= 0.0f) || (rec.width < 1) || (rec.height < 1 ))
{
DrawRectangleRec(rec, color);
return;
}

if (roundness >= 1.0f) roundness = 1.0f;


float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
if (radius <= 0.0f) return;


if (segments < 4)
{

#if !defined(CIRCLE_ERROR_RATE)
#define CIRCLE_ERROR_RATE 0.5f
#endif

float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
segments = ceilf(2*PI/th)/4;
if (segments <= 0) segments = 4;
}

float stepLength = 90.0f/(float)segments;

















const Vector2 point[12] = { 
{(float)rec.x + radius, rec.y}, {(float)(rec.x + rec.width) - radius, rec.y}, { rec.x + rec.width, (float)rec.y + radius }, 
{rec.x + rec.width, (float)(rec.y + rec.height) - radius}, {(float)(rec.x + rec.width) - radius, rec.y + rec.height}, 
{(float)rec.x + radius, rec.y + rec.height}, { rec.x, (float)(rec.y + rec.height) - radius}, {rec.x, (float)rec.y + radius}, 
{(float)rec.x + radius, (float)rec.y + radius}, {(float)(rec.x + rec.width) - radius, (float)rec.y + radius}, 
{(float)(rec.x + rec.width) - radius, (float)(rec.y + rec.height) - radius}, {(float)rec.x + radius, (float)(rec.y + rec.height) - radius} 
};

const Vector2 centers[4] = { point[8], point[9], point[10], point[11] };
const float angles[4] = { 180.0f, 90.0f, 0.0f, 270.0f };

#if defined(SUPPORT_QUADS_DRAW_MODE)
if (rlCheckBufferLimit(16*segments/2 + 5*4)) rlglDraw();

rlEnableTexture(GetShapesTexture().id);

rlBegin(RL_QUADS);

for (int k = 0; k < 4; ++k) 
{
float angle = angles[k];
const Vector2 center = centers[k];

for (int i = 0; i < segments/2; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x, center.y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength*2))*radius, center.y + cosf(DEG2RAD*(angle + stepLength*2))*radius);
angle += (stepLength*2);
}

if (segments%2)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x, center.y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x, center.y);
}
}


rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[0].x, point[0].y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[8].x, point[8].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[9].x, point[9].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[1].x, point[1].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[2].x, point[2].y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[9].x, point[9].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[10].x, point[10].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[3].x, point[3].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[11].x, point[11].y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[5].x, point[5].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[4].x, point[4].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[10].x, point[10].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[7].x, point[7].y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[6].x, point[6].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[11].x, point[11].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[8].x, point[8].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[8].x, point[8].y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[11].x, point[11].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[10].x, point[10].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[9].x, point[9].y);

rlEnd();
rlDisableTexture();
#else
if (rlCheckBufferLimit(12*segments + 5*6)) rlglDraw(); 

rlBegin(RL_TRIANGLES);

for (int k = 0; k < 4; ++k) 
{
float angle = angles[k];
const Vector2 center = centers[k];
for (int i = 0; i < segments; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(center.x, center.y);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius);
angle += stepLength;
}
}


rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(point[0].x, point[0].y);
rlVertex2f(point[8].x, point[8].y);
rlVertex2f(point[9].x, point[9].y);
rlVertex2f(point[1].x, point[1].y);
rlVertex2f(point[0].x, point[0].y);
rlVertex2f(point[9].x, point[9].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(point[9].x, point[9].y);
rlVertex2f(point[10].x, point[10].y);
rlVertex2f(point[3].x, point[3].y);
rlVertex2f(point[2].x, point[2].y);
rlVertex2f(point[9].x, point[9].y);
rlVertex2f(point[3].x, point[3].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(point[11].x, point[11].y);
rlVertex2f(point[5].x, point[5].y);
rlVertex2f(point[4].x, point[4].y);
rlVertex2f(point[10].x, point[10].y);
rlVertex2f(point[11].x, point[11].y);
rlVertex2f(point[4].x, point[4].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(point[7].x, point[7].y);
rlVertex2f(point[6].x, point[6].y);
rlVertex2f(point[11].x, point[11].y);
rlVertex2f(point[8].x, point[8].y);
rlVertex2f(point[7].x, point[7].y);
rlVertex2f(point[11].x, point[11].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(point[8].x, point[8].y);
rlVertex2f(point[11].x, point[11].y);
rlVertex2f(point[10].x, point[10].y);
rlVertex2f(point[9].x, point[9].y);
rlVertex2f(point[8].x, point[8].y);
rlVertex2f(point[10].x, point[10].y);
rlEnd();
#endif
}


void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, int lineThick, Color color)
{
if (lineThick < 0) lineThick = 0;


if (roundness <= 0.0f)
{
DrawRectangleLinesEx((Rectangle){rec.x-lineThick, rec.y-lineThick, rec.width+2*lineThick, rec.height+2*lineThick}, lineThick, color);
return;
}

if (roundness >= 1.0f) roundness = 1.0f;


float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
if (radius <= 0.0f) return;


if (segments < 4)
{

#if !defined(CIRCLE_ERROR_RATE)
#define CIRCLE_ERROR_RATE 0.5f
#endif

float th = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
segments = ceilf(2*PI/th)/2;
if (segments <= 0) segments = 4;
}

float stepLength = 90.0f/(float)segments;
const float outerRadius = radius + (float)lineThick, innerRadius = radius;

















const Vector2 point[16] = {
{(float)rec.x + innerRadius, rec.y - lineThick}, {(float)(rec.x + rec.width) - innerRadius, rec.y - lineThick}, { rec.x + rec.width + lineThick, (float)rec.y + innerRadius }, 
{rec.x + rec.width + lineThick, (float)(rec.y + rec.height) - innerRadius}, {(float)(rec.x + rec.width) - innerRadius, rec.y + rec.height + lineThick}, 
{(float)rec.x + innerRadius, rec.y + rec.height + lineThick}, { rec.x - lineThick, (float)(rec.y + rec.height) - innerRadius}, {rec.x - lineThick, (float)rec.y + innerRadius}, 
{(float)rec.x + innerRadius, rec.y}, {(float)(rec.x + rec.width) - innerRadius, rec.y}, 
{ rec.x + rec.width, (float)rec.y + innerRadius }, {rec.x + rec.width, (float)(rec.y + rec.height) - innerRadius}, 
{(float)(rec.x + rec.width) - innerRadius, rec.y + rec.height}, {(float)rec.x + innerRadius, rec.y + rec.height}, 
{ rec.x, (float)(rec.y + rec.height) - innerRadius}, {rec.x, (float)rec.y + innerRadius} 
};

const Vector2 centers[4] = {
{(float)rec.x + innerRadius, (float)rec.y + innerRadius}, {(float)(rec.x + rec.width) - innerRadius, (float)rec.y + innerRadius}, 
{(float)(rec.x + rec.width) - innerRadius, (float)(rec.y + rec.height) - innerRadius}, {(float)rec.x + innerRadius, (float)(rec.y + rec.height) - innerRadius} 
};

const float angles[4] = { 180.0f, 90.0f, 0.0f, 270.0f };

if (lineThick > 1)
{
#if defined(SUPPORT_QUADS_DRAW_MODE)
if (rlCheckBufferLimit(4*4*segments + 4*4)) rlglDraw(); 

rlEnableTexture(GetShapesTexture().id);

rlBegin(RL_QUADS);

for (int k = 0; k < 4; ++k) 
{
float angle = angles[k];
const Vector2 center = centers[k];
for (int i = 0; i < segments; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*innerRadius, center.y + cosf(DEG2RAD*angle)*innerRadius);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*outerRadius);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*innerRadius);

angle += stepLength;
}
}


rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[0].x, point[0].y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[8].x, point[8].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[9].x, point[9].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[1].x, point[1].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[2].x, point[2].y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[10].x, point[10].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[11].x, point[11].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[3].x, point[3].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[13].x, point[13].y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[5].x, point[5].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[4].x, point[4].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[12].x, point[12].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[15].x, point[15].y);
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[7].x, point[7].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(point[6].x, point[6].y);
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(point[14].x, point[14].y);

rlEnd();
rlDisableTexture();
#else
if (rlCheckBufferLimit(4*6*segments + 4*6)) rlglDraw(); 

rlBegin(RL_TRIANGLES);


for (int k = 0; k < 4; ++k) 
{
float angle = angles[k];
const Vector2 center = centers[k];

for (int i = 0; i < segments; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlVertex2f(center.x + sinf(DEG2RAD*angle)*innerRadius, center.y + cosf(DEG2RAD*angle)*innerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*innerRadius);

rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*innerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*innerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*outerRadius);

angle += stepLength;
}
}


rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(point[0].x, point[0].y);
rlVertex2f(point[8].x, point[8].y);
rlVertex2f(point[9].x, point[9].y);
rlVertex2f(point[1].x, point[1].y);
rlVertex2f(point[0].x, point[0].y);
rlVertex2f(point[9].x, point[9].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(point[10].x, point[10].y);
rlVertex2f(point[11].x, point[11].y);
rlVertex2f(point[3].x, point[3].y);
rlVertex2f(point[2].x, point[2].y);
rlVertex2f(point[10].x, point[10].y);
rlVertex2f(point[3].x, point[3].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(point[13].x, point[13].y);
rlVertex2f(point[5].x, point[5].y);
rlVertex2f(point[4].x, point[4].y);
rlVertex2f(point[12].x, point[12].y);
rlVertex2f(point[13].x, point[13].y);
rlVertex2f(point[4].x, point[4].y);


rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(point[7].x, point[7].y);
rlVertex2f(point[6].x, point[6].y);
rlVertex2f(point[14].x, point[14].y);
rlVertex2f(point[15].x, point[15].y);
rlVertex2f(point[7].x, point[7].y);
rlVertex2f(point[14].x, point[14].y);
rlEnd();
#endif
}
else
{

if (rlCheckBufferLimit(8*segments + 4*2)) rlglDraw(); 

rlBegin(RL_LINES);


for (int k = 0; k < 4; ++k) 
{
float angle = angles[k];
const Vector2 center = centers[k];

for (int i = 0; i < segments; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(center.x + sinf(DEG2RAD*angle)*outerRadius, center.y + cosf(DEG2RAD*angle)*outerRadius);
rlVertex2f(center.x + sinf(DEG2RAD*(angle + stepLength))*outerRadius, center.y + cosf(DEG2RAD*(angle + stepLength))*outerRadius);
angle += stepLength;
}
}

for(int i = 0; i < 8; i += 2)
{
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(point[i].x, point[i].y);
rlVertex2f(point[i + 1].x, point[i + 1].y);
}
rlEnd();
}
}



void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
if (rlCheckBufferLimit(4)) rlglDraw();

#if defined(SUPPORT_QUADS_DRAW_MODE)
rlEnableTexture(GetShapesTexture().id);

rlBegin(RL_QUADS);
rlColor4ub(color.r, color.g, color.b, color.a);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(v1.x, v1.y);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(v2.x, v2.y);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(v2.x, v2.y);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(v3.x, v3.y);
rlEnd();

rlDisableTexture();
#else
rlBegin(RL_TRIANGLES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(v1.x, v1.y);
rlVertex2f(v2.x, v2.y);
rlVertex2f(v3.x, v3.y);
rlEnd();
#endif
}



void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color)
{
if (rlCheckBufferLimit(6)) rlglDraw();

rlBegin(RL_LINES);
rlColor4ub(color.r, color.g, color.b, color.a);
rlVertex2f(v1.x, v1.y);
rlVertex2f(v2.x, v2.y);

rlVertex2f(v2.x, v2.y);
rlVertex2f(v3.x, v3.y);

rlVertex2f(v3.x, v3.y);
rlVertex2f(v1.x, v1.y);
rlEnd();
}



void DrawTriangleFan(Vector2 *points, int pointsCount, Color color)
{
if (pointsCount >= 3)
{
if (rlCheckBufferLimit((pointsCount - 2)*4)) rlglDraw();

rlEnableTexture(GetShapesTexture().id);
rlBegin(RL_QUADS);
rlColor4ub(color.r, color.g, color.b, color.a);

for (int i = 1; i < pointsCount - 1; i++)
{
rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(points[0].x, points[0].y);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(points[i].x, points[i].y);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(points[i + 1].x, points[i + 1].y);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(points[i + 1].x, points[i + 1].y);
}
rlEnd();
rlDisableTexture();
}
}



void DrawTriangleStrip(Vector2 *points, int pointsCount, Color color)
{
if (pointsCount >= 3)
{
if (rlCheckBufferLimit(pointsCount)) rlglDraw();

rlBegin(RL_TRIANGLES);
rlColor4ub(color.r, color.g, color.b, color.a);

for (int i = 2; i < pointsCount; i++)
{
if ((i%2) == 0)
{
rlVertex2f(points[i].x, points[i].y);
rlVertex2f(points[i - 2].x, points[i - 2].y);
rlVertex2f(points[i - 1].x, points[i - 1].y);
}
else
{
rlVertex2f(points[i].x, points[i].y);
rlVertex2f(points[i - 1].x, points[i - 1].y);
rlVertex2f(points[i - 2].x, points[i - 2].y);
}
}
rlEnd();
}
}


void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color)
{
if (sides < 3) sides = 3;
float centralAngle = 0.0f;

if (rlCheckBufferLimit(4*(360/sides))) rlglDraw();

rlPushMatrix();
rlTranslatef(center.x, center.y, 0.0f);
rlRotatef(rotation, 0.0f, 0.0f, 1.0f);

#if defined(SUPPORT_QUADS_DRAW_MODE)
rlEnableTexture(GetShapesTexture().id);

rlBegin(RL_QUADS);
for (int i = 0; i < sides; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(0, 0);

rlTexCoord2f(GetShapesTextureRec().x/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);

rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, (GetShapesTextureRec().y + GetShapesTextureRec().height)/GetShapesTexture().height);
rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);

centralAngle += 360.0f/(float)sides;
rlTexCoord2f((GetShapesTextureRec().x + GetShapesTextureRec().width)/GetShapesTexture().width, GetShapesTextureRec().y/GetShapesTexture().height);
rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);
}
rlEnd();
rlDisableTexture();
#else
rlBegin(RL_TRIANGLES);
for (int i = 0; i < sides; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlVertex2f(0, 0);
rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);

centralAngle += 360.0f/(float)sides;
rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);
}
rlEnd();
#endif
rlPopMatrix();
}


void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color)
{
if (sides < 3) sides = 3;
float centralAngle = 0.0f;

if (rlCheckBufferLimit(3*(360/sides))) rlglDraw();

rlPushMatrix();
rlTranslatef(center.x, center.y, 0.0f);
rlRotatef(rotation, 0.0f, 0.0f, 1.0f);

rlBegin(RL_LINES);
for (int i = 0; i < sides; i++)
{
rlColor4ub(color.r, color.g, color.b, color.a);

rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);
centralAngle += 360.0f/(float)sides;
rlVertex2f(sinf(DEG2RAD*centralAngle)*radius, cosf(DEG2RAD*centralAngle)*radius);
}
rlEnd();
rlPopMatrix();
}






bool CheckCollisionPointRec(Vector2 point, Rectangle rec)
{
bool collision = false;

if ((point.x >= rec.x) && (point.x <= (rec.x + rec.width)) && (point.y >= rec.y) && (point.y <= (rec.y + rec.height))) collision = true;

return collision;
}


bool CheckCollisionPointCircle(Vector2 point, Vector2 center, float radius)
{
return CheckCollisionCircles(point, 0, center, radius);
}


bool CheckCollisionPointTriangle(Vector2 point, Vector2 p1, Vector2 p2, Vector2 p3)
{
bool collision = false;

float alpha = ((p2.y - p3.y)*(point.x - p3.x) + (p3.x - p2.x)*(point.y - p3.y)) /
((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));

float beta = ((p3.y - p1.y)*(point.x - p3.x) + (p1.x - p3.x)*(point.y - p3.y)) /
((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));

float gamma = 1.0f - alpha - beta;

if ((alpha > 0) && (beta > 0) & (gamma > 0)) collision = true;

return collision;
}


bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2)
{
bool collision = false;

if ((rec1.x < (rec2.x + rec2.width) && (rec1.x + rec1.width) > rec2.x) &&
(rec1.y < (rec2.y + rec2.height) && (rec1.y + rec1.height) > rec2.y)) collision = true;

return collision;
}


bool CheckCollisionCircles(Vector2 center1, float radius1, Vector2 center2, float radius2)
{
bool collision = false;

float dx = center2.x - center1.x; 
float dy = center2.y - center1.y; 

float distance = sqrtf(dx*dx + dy*dy); 

if (distance <= (radius1 + radius2)) collision = true;

return collision;
}



bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec)
{
int recCenterX = (int)(rec.x + rec.width/2.0f);
int recCenterY = (int)(rec.y + rec.height/2.0f);

float dx = fabsf(center.x - (float)recCenterX);
float dy = fabsf(center.y - (float)recCenterY);

if (dx > (rec.width/2.0f + radius)) { return false; }
if (dy > (rec.height/2.0f + radius)) { return false; }

if (dx <= (rec.width/2.0f)) { return true; }
if (dy <= (rec.height/2.0f)) { return true; }

float cornerDistanceSq = (dx - rec.width/2.0f)*(dx - rec.width/2.0f) +
(dy - rec.height/2.0f)*(dy - rec.height/2.0f);

return (cornerDistanceSq <= (radius*radius));
}


Rectangle GetCollisionRec(Rectangle rec1, Rectangle rec2)
{
Rectangle retRec = { 0, 0, 0, 0 };

if (CheckCollisionRecs(rec1, rec2))
{
float dxx = fabsf(rec1.x - rec2.x);
float dyy = fabsf(rec1.y - rec2.y);

if (rec1.x <= rec2.x)
{
if (rec1.y <= rec2.y)
{
retRec.x = rec2.x;
retRec.y = rec2.y;
retRec.width = rec1.width - dxx;
retRec.height = rec1.height - dyy;
}
else
{
retRec.x = rec2.x;
retRec.y = rec1.y;
retRec.width = rec1.width - dxx;
retRec.height = rec2.height - dyy;
}
}
else
{
if (rec1.y <= rec2.y)
{
retRec.x = rec1.x;
retRec.y = rec2.y;
retRec.width = rec2.width - dxx;
retRec.height = rec1.height - dyy;
}
else
{
retRec.x = rec1.x;
retRec.y = rec1.y;
retRec.width = rec2.width - dxx;
retRec.height = rec2.height - dyy;
}
}

if (rec1.width > rec2.width)
{
if (retRec.width >= rec2.width) retRec.width = rec2.width;
}
else
{
if (retRec.width >= rec1.width) retRec.width = rec1.width;
}

if (rec1.height > rec2.height)
{
if (retRec.height >= rec2.height) retRec.height = rec2.height;
}
else
{
if (retRec.height >= rec1.height) retRec.height = rec1.height;
}
}

return retRec;
}







static float EaseCubicInOut(float t, float b, float c, float d)
{
if ((t /= 0.5f*d) < 1) return 0.5f*c*t*t*t + b;

t -= 2;

return 0.5f*c*(t*t*t + 2.0f) + b;
}
