#include <allegro5/allegro.h>
#include <math.h>
#include <string.h>
#include "defines.h"
#include "game.h"
#include "level.h"
#include "level_alloc.h"
#include "level_file.h"
#include "token.h"
int framec = 0;
struct BoundingBox DispBox;
void DrawLevelBackground(struct Level *lev, double *pos)
{
float w, h;
if (!lev)
return;
h = 480.0;
w = screen_width * 480 / screen_height;
framec++;
DispBox.TL.Pos[0] = pos[0] - w / 2;
DispBox.TL.Pos[1] = pos[1] - h / 2;
DispBox.BR.Pos[0] = DispBox.TL.Pos[0] + w;
DispBox.BR.Pos[1] = DispBox.TL.Pos[1] + h;
BeginQuadTreeDraw(lev, &lev->DisplayTree, &DispBox, framec);
}
void DrawLevelForeground(struct Level *lev)
{
if (!lev)
return;
EndQuadTreeDraw(lev, &DispBox, framec);
}
ALLEGRO_BITMAP *ObtainBitmap(const char *name)
{
DATA_ENTRY *data = demo_data;
while (data && data->name) {
if (!strcmp(data->type, "bitmap") &&
!strcmp(data->name, name)) return data->dat;
data++;
}
return NULL;
}
ALLEGRO_SAMPLE *ObtainSample(const char *name)
{
DATA_ENTRY *data = demo_data;
while (data && data->name) {
if (!strcmp(data->type, "sample") &&
!strcmp(data->name, name)) return data->dat;
data++;
}
return NULL;
}
#undef min
#undef max
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define FreeList(name, type, sp) while(lvl->name){Next = (void *)lvl->name->Next;sp(lvl->name);free(lvl->name);lvl->name = (struct type *)Next;}
#define NoAction(v)
void FreeLevel(struct Level *lvl)
{
void *Next;
if (!lvl)
return;
FreeQuadTree(&lvl->DisplayTree);
FreeQuadTree(&lvl->CollisionTree);
FreeList(AllTris, Triangle, NoAction);
FreeList(AllEdges, Edge, NoAction);
FreeList(AllMats, Material, NoAction);
FreeList(AllObjectTypes, ObjectType, NoAction);
FreeList(AllVerts, Vertex, NoAction);
FreeList(AllObjects, Object, NoAction);
FreeState(lvl->InitialState);
free(lvl);
}
static int CalcIntersection(struct Edge *e1, struct Edge *e2, double *Inter,
int radius)
{
double d1, d2;
d1 =
(e1->EndPoints[0]->Pos[0] + radius * e1->a) * e2->a +
(e1->EndPoints[0]->Pos[1] + radius * e1->b) * e2->b + e2->c;
d2 =
(e1->EndPoints[1]->Pos[0] + radius * e1->a) * e2->a +
(e1->EndPoints[1]->Pos[1] + radius * e1->b) * e2->b + e2->c;
if (!(d1 - d2))
return false;
d1 /= (d1 - d2);
Inter[0] =
e1->EndPoints[0]->Pos[0] + radius * e1->a +
d1 * (e1->EndPoints[1]->Pos[0] - e1->EndPoints[0]->Pos[0]);
Inter[1] =
e1->EndPoints[0]->Pos[1] + radius * e1->b +
d1 * (e1->EndPoints[1]->Pos[1] - e1->EndPoints[0]->Pos[1]);
return true;
}
static void FixVerts(struct Level *NewLev, int radius)
{
struct Vertex *v = NewLev->AllVerts;
while (v) {
if (v->Edges[1]) {
if (!CalcIntersection(v->Edges[0], v->Edges[1], v->Normal, radius)) {
v->Normal[0] = v->Pos[0] + v->Edges[0]->a * radius;
v->Normal[1] = v->Pos[1] + v->Edges[0]->b * radius;
}
} else if (v->Edges[0]) {
double Direction = -1.0f;
if (v->Edges[0]->EndPoints[0] == v)
Direction = 1.0f;
v->Normal[0] =
v->Pos[0] + v->Edges[0]->a * radius +
radius * Direction * v->Edges[0]->b;
v->Normal[1] =
v->Pos[1] + v->Edges[0]->b * radius -
radius * Direction * v->Edges[0]->a;
}
v = v->Next;
}
}
static int FixEdges(struct Level *NewLev, int radius)
{
struct Edge **e = &NewLev->AllEdges;
int NotFinished = false, Failed;
struct Edge OldEdge, *EdgePtr;
(void)radius;
while (*e) {
OldEdge = **e;
Failed =
GetNormal(*e, (*e)->EndPoints[0]->Normal,
(*e)->EndPoints[1]->Normal);
if (Failed || (((*e)->a * OldEdge.a + (*e)->b * OldEdge.b) < 0)) {
NotFinished = true;
EdgePtr =
((*e)->EndPoints[1]->Edges[0] ==
(*e)) ? (*e)->EndPoints[1]->Edges[1] : (*e)->EndPoints[1]->
Edges[0];
if ((*e)->EndPoints[0]->Edges[0] == (*e)) {
if (!EdgePtr) {
(*e)->EndPoints[0]->Edges[0] = (*e)->EndPoints[0]->Edges[1];
(*e)->EndPoints[0]->Edges[1] = NULL;
} else
(*e)->EndPoints[0]->Edges[0] = EdgePtr;
} else
(*e)->EndPoints[0]->Edges[1] = EdgePtr;
if (EdgePtr->EndPoints[0] == (*e)->EndPoints[1])
EdgePtr->EndPoints[0] = (*e)->EndPoints[0];
else
EdgePtr->EndPoints[1] = (*e)->EndPoints[0];
(*e)->EndPoints[1]->Edges[0] = (*e)->EndPoints[1]->Edges[1] = NULL;
EdgePtr = *e;
(*e) = (*e)->Next;
free((void *)EdgePtr);
if (!(*e))
break;
}
e = &((*e)->Next);
}
return NotFinished;
}
static void AddEdges(struct Level *NewLev)
{
struct Edge *e = NewLev->AllEdges;
while (e) {
e->Bounder.TL.Pos[0] =
min(e->EndPoints[0]->Normal[0], e->EndPoints[1]->Normal[0]) - 0.05f;
e->Bounder.TL.Pos[1] =
min(e->EndPoints[0]->Normal[1], e->EndPoints[1]->Normal[1]) - 0.05f;
e->Bounder.BR.Pos[0] =
max(e->EndPoints[0]->Normal[0], e->EndPoints[1]->Normal[0]) + 0.05f;
e->Bounder.BR.Pos[1] =
max(e->EndPoints[0]->Normal[1], e->EndPoints[1]->Normal[1]) + 0.05f;
AddEdge(NewLev, e);
e = e->Next;
}
}
static void ScaleAndAddObjects(struct Level *Lvl)
{
struct Object *O = Lvl->AllObjects;
while (O) {
O->Bounder.BR.Pos[0] = O->Pos[0] + O->ObjType->Radius;
O->Bounder.BR.Pos[1] = O->Pos[1] + O->ObjType->Radius;
O->Bounder.TL.Pos[0] = O->Pos[0] - O->ObjType->Radius;
O->Bounder.TL.Pos[1] = O->Pos[1] - O->ObjType->Radius;
AddObject(Lvl, O, false);
AddObject(Lvl, O, true);
O = O->Next;
}
}
static void AddTriangles(struct Level *Lvl)
{
struct Triangle *Tri = Lvl->AllTris;
while (Tri) {
int c = 3;
Tri->Bounder.TL.Pos[0] = Tri->Bounder.BR.Pos[0] = Tri->Edges[0]->Pos[0];
Tri->Bounder.TL.Pos[1] = Tri->Bounder.BR.Pos[1] = Tri->Edges[0]->Pos[1];
while (c--) {
int lc = c - 1;
if (lc < 0)
lc = 2;
if (Tri->Edges[c]->Pos[0] < Tri->Bounder.TL.Pos[0])
Tri->Bounder.TL.Pos[0] = Tri->Edges[c]->Pos[0];
if (Tri->Edges[c]->Pos[0] > Tri->Bounder.BR.Pos[0])
Tri->Bounder.BR.Pos[0] = Tri->Edges[c]->Pos[0];
if ((Tri->EdgeFlags[c] & TRIFLAGS_EDGE)
|| (Tri->EdgeFlags[lc] & TRIFLAGS_EDGE)) {
if (Tri->Edges[c]->Pos[1] -
(Tri->EdgeFlags[c] & TRIFLAGS_WIDTH) < Tri->Bounder.TL.Pos[1])
Tri->Bounder.TL.Pos[1] =
Tri->Edges[c]->Pos[1] -
(Tri->EdgeFlags[c] & TRIFLAGS_WIDTH);
if (Tri->Edges[c]->Pos[1] + (Tri->EdgeFlags[c] & TRIFLAGS_WIDTH) >
Tri->Bounder.BR.Pos[1])
Tri->Bounder.BR.Pos[1] =
Tri->Edges[c]->Pos[1] +
(Tri->EdgeFlags[c] & TRIFLAGS_WIDTH);
} else {
if (Tri->Edges[c]->Pos[1] < Tri->Bounder.TL.Pos[1])
Tri->Bounder.TL.Pos[1] = Tri->Edges[c]->Pos[1];
if (Tri->Edges[c]->Pos[1] > Tri->Bounder.BR.Pos[1])
Tri->Bounder.BR.Pos[1] = Tri->Edges[c]->Pos[1];
}
}
AddTriangle(Lvl, Tri);
Tri = Tri->Next;
}
}
char *GetLevelError(void)
{
char *LocTemp;
if (!strlen(ErrorText))
snprintf(ErrorText, sizeof(ErrorText), "Unspecified error at line %d",
Lines);
LocTemp = strdup(ErrorText);
snprintf(ErrorText, sizeof(ErrorText), "Level load - %s", LocTemp);
free((void *)LocTemp);
return ErrorText;
}
struct Level *LoadLevel(char const *name, int radius)
{
ALLEGRO_FILE *file;
struct Level *NewLev = NULL;
ErrorText[0] = '\0'; 
Error = 0; 
Lines = 1; 
file = al_fopen(name, "r");
input = file;
if (!input) {
snprintf(ErrorText, sizeof(ErrorText), "Unable to load %s", name);
goto error;
}
NewLev = NewLevel();
LoadMaterials(NewLev);
if (Error) {
goto error;
}
LoadVertices(NewLev);
if (Error) {
goto error;
}
LoadTriangles(NewLev, radius);
if (Error) {
goto error;
}
do
FixVerts(NewLev, radius);
while (FixEdges(NewLev, radius));
AddEdges(NewLev);
LoadObjectTypes(NewLev, radius);
if (Error) {
goto error;
}
if (!(NewLev->DoorOpen = ObtainBitmap("dooropen"))) {
snprintf(ErrorText, sizeof(ErrorText),
"Unable to obtain dooropen sprite");
goto error;
}
if (!(NewLev->DoorShut = ObtainBitmap("doorshut"))) {
snprintf(ErrorText, sizeof(ErrorText),
"Unable to obtain doorshut sprite");
goto error;
}
NewLev->Door.Image = NewLev->DoorShut;
NewLev->Door.CollectNoise = ObtainSample("dooropen");
NewLev->Door.Radius = 14 + radius;
NewLev->TotalObjects = 0;
LoadObjects(NewLev);
if (Error) {
goto error;
}
AddTriangles(NewLev);
ScaleAndAddObjects(NewLev);
SplitTree(&NewLev->DisplayTree);
OrderTree(&NewLev->DisplayTree, false);
OrderTree(&NewLev->DisplayTree, true);
OrderTree(&NewLev->CollisionTree, false);
LoadStats(NewLev);
if (Error) {
goto error;
}
NewLev->InitialState = BorrowState(NewLev);
return NewLev;
error:
if (file)
al_fclose(file);
if (NewLev)
FreeLevel(NewLev);
return NULL;
}
