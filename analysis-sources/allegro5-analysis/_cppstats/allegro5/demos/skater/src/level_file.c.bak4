#include <allegro5/allegro.h>
#include "global.h"
#include <string.h>
#include <math.h>
#include "level_alloc.h"
#include "level_file.h"
#include "token.h"






















void LoadMaterials(struct Level *NewLev)
{
struct Material **NewMatPtr = &NewLev->AllMats;
#if defined(DEMO_USE_ALLEGRO_GL)
ALLEGRO_BITMAP *TmpEdge;
#endif

ExpectToken(TK_OPENBRACE);
while (1) {
GetToken();
switch (Token.Type) {
case TK_CLOSEBRACE:
return;
case TK_OPENBRACE:
{
*NewMatPtr = NewMaterial();

ExpectToken(TK_STRING);
if (!((*NewMatPtr)->Fill = ObtainBitmap(Token.Text))) {
Error = 1;
snprintf(ErrorText, sizeof(ErrorText),
"Could not load material fill %s at line %d",
Token.Text, Lines);
return;
}

ExpectToken(TK_COMMA);
ExpectToken(TK_STRING);
#if defined(DEMO_USE_ALLEGRO_GL)
if (!(TmpEdge = ObtainBitmap(Token.Text))) {
#else
if (!((*NewMatPtr)->Edge = ObtainBitmap(Token.Text))) {
#endif
Error = 1;
snprintf(ErrorText, sizeof(ErrorText),
"Could not load material edge %s at line %d",
Token.Text, Lines);
return;
}

ExpectToken(TK_COMMA);
ExpectToken(TK_NUMBER);
(*NewMatPtr)->Friction = Token.FQuantity;

ExpectToken(TK_CLOSEBRACE);
NewMatPtr = &(*NewMatPtr)->Next;
}
break;
default:
Error = 1;
return;
}
}
}











void LoadVertices(struct Level *NewLev)
{
struct Vertex **NewVertPtr = &NewLev->AllVerts;

ExpectToken(TK_OPENBRACE);
while (1) {
GetToken();
switch (Token.Type) {
case TK_CLOSEBRACE:
return;
case TK_OPENBRACE:
{
*NewVertPtr = NewVertex();

ExpectToken(TK_NUMBER);
(*NewVertPtr)->Pos[0] = Token.FQuantity;

ExpectToken(TK_COMMA);
ExpectToken(TK_NUMBER);
(*NewVertPtr)->Pos[1] = Token.FQuantity;

ExpectToken(TK_CLOSEBRACE);
NewVertPtr = &(*NewVertPtr)->Next;
}
break;
default:
Error = 1;
return;
}
}
}












static void GetVert(struct Level *NewLev, struct Triangle *t, int c)
{
ExpectToken(TK_OPENBRACE);

ExpectToken(TK_NUMBER);
t->Edges[c] = NewLev->AllVerts;
while (t->Edges[c] && Token.IQuantity--)
t->Edges[c] = t->Edges[c]->Next;

if (Token.IQuantity != -1) {
Error = 1;
snprintf(ErrorText, sizeof(ErrorText),
"Unknown vertex referenced at line %d", Lines);
return;
}
ExpectToken(TK_COMMA);

ExpectToken(TK_NUMBER);
t->EdgeFlags[c] = (Token.IQuantity * screen_height) / 480;

GetToken();
switch (Token.Type) {
case TK_COMMA:
{
int Finished = false;

while (!Finished) {
GetToken();
switch (Token.Type) {
case TK_CLOSEBRACE:
Finished = true;
break;
case TK_STRING:
if (!strcmp(Token.Text, "edge"))
t->EdgeFlags[c] |= TRIFLAGS_EDGE;
if (!strcmp(Token.Text, "collidable"))
t->EdgeFlags[c] |= FLAGS_COLLIDABLE;
if (!strcmp(Token.Text, "foreground"))
t->EdgeFlags[c] |= FLAGS_FOREGROUND;
break;
default:
Error = 1;
return;
}
}
}
break;
case TK_CLOSEBRACE:
break;
default:
Error = 1;
return;
}
}







int GetNormal(struct Edge *e, double *v1, double *v2)
{

double length;

e->a = v2[1] - v1[1];
e->b = -(v2[0] - v1[0]);


length = sqrt(e->a * e->a + e->b * e->b);
if (length < 1.0f)
return true;
e->a /= length;
e->b /= length;


e->c = -(v1[0] * e->a + v1[1] * e->b);
return false;
}








static void InitEdge(struct Edge *e, int radius)
{

GetNormal(e, e->EndPoints[0]->Pos, e->EndPoints[1]->Pos);


e->c -= radius * (e->a * e->a + e->b * e->b);


if (!e->EndPoints[0]->Edges[0])
e->EndPoints[0]->Edges[0] = e;
else
e->EndPoints[0]->Edges[1] = e;
if (!e->EndPoints[1]->Edges[0])
e->EndPoints[1]->Edges[0] = e;
else
e->EndPoints[1]->Edges[1] = e;
}












void LoadTriangles(struct Level *NewLev, int radius)
{
struct Triangle *Tri;
struct Edge *NextEdge;
int c;

ExpectToken(TK_OPENBRACE);
while (1) {
GetToken();
switch (Token.Type) {
case TK_CLOSEBRACE:
return;
case TK_OPENBRACE:
{
Tri = NewTriangle();


GetVert(NewLev, Tri, 0);
ExpectToken(TK_COMMA);
GetVert(NewLev, Tri, 1);
ExpectToken(TK_COMMA);
GetVert(NewLev, Tri, 2);
ExpectToken(TK_COMMA);


ExpectToken(TK_NUMBER);

Tri->Material = NewLev->AllMats;
while (Tri->Material && Token.IQuantity--)
Tri->Material = Tri->Material->Next;
if (Token.IQuantity != -1) {
Error = 1;
snprintf(ErrorText, sizeof(ErrorText),
"Unknown material referenced at line %d", Lines);
return;
}


ExpectToken(TK_CLOSEBRACE);


Tri->Next = NewLev->AllTris;
NewLev->AllTris = Tri;


c = 3;
while (c--) {
if (Tri->EdgeFlags[c] & FLAGS_COLLIDABLE) {
NextEdge = NewLev->AllEdges;
NewLev->AllEdges = NewEdge();
NewLev->AllEdges->Material = Tri->Material;
NewLev->AllEdges->Next = NextEdge;

NewLev->AllEdges->EndPoints[0] = Tri->Edges[c];
NewLev->AllEdges->EndPoints[1] = Tri->Edges[(c + 1) % 3];

InitEdge(NewLev->AllEdges, radius);
}
}
}
break;
default:
Error = 1;
return;
}
}
}











void LoadObjectTypes(struct Level *NewLev, int radius)
{
struct ObjectType **NewObjectPtr = &NewLev->AllObjectTypes;
int w, h;

ExpectToken(TK_OPENBRACE);
while (1) {
GetToken();
switch (Token.Type) {
case TK_CLOSEBRACE:
return;
case TK_OPENBRACE:
{
*NewObjectPtr = NewObjectType();

ExpectToken(TK_STRING);
if (!((*NewObjectPtr)->Image = ObtainBitmap(Token.Text))) {
Error = 1;
snprintf(ErrorText, sizeof(ErrorText),
"Could not load object image %s at line %d",
Token.Text, Lines);
return;
}
w = al_get_bitmap_width((*NewObjectPtr)->Image);
h = al_get_bitmap_height((*NewObjectPtr)->Image);
(*NewObjectPtr)->Radius = w > h ? w : h;
(*NewObjectPtr)->Radius += radius;

ExpectToken(TK_COMMA);
ExpectToken(TK_STRING);


(*NewObjectPtr)->CollectNoise = ObtainSample(Token.Text);

ExpectToken(TK_CLOSEBRACE);
NewObjectPtr = &(*NewObjectPtr)->Next;
}
break;
default:
Error = 1;
return;
}
}
}













void LoadObjects(struct Level *NewLev)
{
struct Object *Obj;
int Finished;

ExpectToken(TK_OPENBRACE);
while (1) {
GetToken();
switch (Token.Type) {
case TK_CLOSEBRACE:
return;
case TK_OPENBRACE:
{
Obj = NewObject();
NewLev->TotalObjects++;


ExpectToken(TK_NUMBER);
Obj->Pos[0] = Token.FQuantity;
ExpectToken(TK_COMMA);
ExpectToken(TK_NUMBER);
Obj->Pos[1] = Token.FQuantity;
ExpectToken(TK_COMMA);


ExpectToken(TK_NUMBER);
Obj->Angle = Token.FQuantity;
ExpectToken(TK_COMMA);


ExpectToken(TK_NUMBER);
if (Token.IQuantity >= 0) {
Obj->ObjType = NewLev->AllObjectTypes;
while (Obj->ObjType && Token.IQuantity--)
Obj->ObjType = Obj->ObjType->Next;

if (Token.IQuantity != -1) {
Error = 1;
snprintf(ErrorText, sizeof(ErrorText),
"Unknown object referenced at line %d", Lines);
return;
}
} else {

Obj->ObjType = &NewLev->Door;
Obj->Flags |= OBJFLAGS_DOOR;
NewLev->TotalObjects--;
}


GetToken();
switch (Token.Type) {
case TK_COMMA:
{
Finished = false;
while (!Finished) {
GetToken();
switch (Token.Type) {
case TK_CLOSEBRACE:
Finished = true;
break;
case TK_STRING:
if (!strcmp(Token.Text, "collidable"))
Obj->Flags |= FLAGS_COLLIDABLE;
if (!strcmp(Token.Text, "foreground"))
Obj->Flags |= FLAGS_FOREGROUND;
break;
default:
Error = 1;
return;
}
}
}
break;
case TK_CLOSEBRACE:
break;
default:
Error = 1;
return;
}


Obj->Next = NewLev->AllObjects;
NewLev->AllObjects = Obj;
}
break;
default:
Error = 1;
return;
}
}
}











void LoadStats(struct Level *NewLev)
{
ExpectToken(TK_OPENBRACE);

ExpectToken(TK_NUMBER);
NewLev->PlayerStartPos[0] = Token.FQuantity;
ExpectToken(TK_COMMA);
ExpectToken(TK_NUMBER);
NewLev->PlayerStartPos[1] = Token.FQuantity;
ExpectToken(TK_COMMA);

ExpectToken(TK_NUMBER);
NewLev->ObjectsRequired = Token.IQuantity;
ExpectToken(TK_CLOSEBRACE);
}
