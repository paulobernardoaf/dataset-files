










































#include <allegro5/allegro_primitives.h>
#include "level.h"
#include "global.h"







#define CentreX(b) (((b)->TL.Pos[0]+(b)->BR.Pos[0]) * 0.5f)
#define CentreY(b) (((b)->TL.Pos[1]+(b)->BR.Pos[1]) * 0.5f)
#define ERROR_BOUNDARY 2







static int GetChild(struct BoundingBox *b, double qx, double qy)
{
int ret = 0;

if (qx > CentreX(b))
ret |= 1;
if (qy > CentreY(b))
ret |= 2;
return ret;
}







static double GetX1(int child, struct BoundingBox *b)
{
return (child & 1) ? CentreX(b) : b->TL.Pos[0];
}
static double GetX2(int child, struct BoundingBox *b)
{
return (child & 1) ? b->BR.Pos[0] : CentreX(b);
}
static double GetY1(int child, struct BoundingBox *b)
{
return (child & 2) ? CentreY(b) : b->TL.Pos[1];
}
static double GetY2(int child, struct BoundingBox *b)
{
return (child & 2) ? b->BR.Pos[1] : CentreY(b);
}

















static int ToggleChildPtrY(double yvec, int oldchild)
{
if (((yvec > 0) && (oldchild & 2)) || ((yvec < 0) && !(oldchild & 2))
)
oldchild ^= 2;
return oldchild;
}

static int ToggleChildPtrX(double xvec, int oldchild)
{
if (((xvec > 0) && (oldchild & 1)) || ((xvec < 0) && !(oldchild & 1))
)
oldchild ^= 1;
return oldchild;
}























static int Separated(struct BoundingBox *a, struct BoundingBox *b)
{
if (a->TL.Pos[0] - ERROR_BOUNDARY > b->BR.Pos[0])
return true;
if (a->TL.Pos[1] - ERROR_BOUNDARY > b->BR.Pos[1])
return true;

if (b->TL.Pos[0] - ERROR_BOUNDARY > a->BR.Pos[0])
return true;
if (b->TL.Pos[1] - ERROR_BOUNDARY > a->BR.Pos[1])
return true;

return false;
}













void SetupQuadTree(struct QuadTreeNode *Tree, int x1, int y1, int x2, int y2)
{
Tree->Bounder.TL.Pos[0] = x1;
Tree->Bounder.TL.Pos[1] = y1;
Tree->Bounder.BR.Pos[0] = x2;
Tree->Bounder.BR.Pos[1] = y2;

Tree->NumContents = 0;
Tree->Contents = Tree->PostContents = NULL;
Tree->Children = NULL;
}







void FreeQuadTree(struct QuadTreeNode *Tree)
{
int c;
struct Container *CNext;






if (Tree->Children) {
c = 4;
while (c--)
FreeQuadTree(&Tree->Children[c]);

free((void *)Tree->Children);
Tree->Children = NULL;
}


while (Tree->Contents) {
CNext = Tree->Contents->Next;
free((void *)Tree->Contents);
Tree->Contents = CNext;
}
}
















static void AddContent(struct QuadTreeNode *Tree, struct Container *NewContent,
int divider)
{
int c;
struct Container *CPtr, *Next;












if (Separated(&NewContent->Content.E->Bounder, &Tree->Bounder))
return;







if (Tree->Children) {
c = 4;
while (c--)
AddContent(&Tree->Children[c], NewContent, divider);
} else {






Tree->NumContents++;
CPtr = Tree->Contents;
Tree->Contents = (struct Container *)malloc(sizeof(struct Container));
Tree->Contents->Content = NewContent->Content;
Tree->Contents->Type = NewContent->Type;
Tree->Contents->Next = CPtr;







if ((Tree->NumContents == divider)
&& ((Tree->Bounder.BR.Pos[0] - Tree->Bounder.TL.Pos[0]) > screen_width)
&& ((Tree->Bounder.BR.Pos[1] - Tree->Bounder.TL.Pos[1]) > screen_height)) {

Tree->Children =
(struct QuadTreeNode *)malloc(sizeof(struct QuadTreeNode)
* 4);
c = 4;
while (c--)
SetupQuadTree(&Tree->Children[c],
GetX1(c, &Tree->Bounder), GetY1(c,
&Tree->
Bounder),
GetX2(c, &Tree->Bounder), GetY2(c, &Tree->Bounder));


CPtr = Tree->Contents;
while (CPtr) {
Next = CPtr->Next;

c = 4;
while (c--)
AddContent(&Tree->Children[c], CPtr, divider);

free(CPtr);
CPtr = Next;
}
Tree->Contents = NULL;
}
}
}














static struct QuadTreeNode *GetNode(struct QuadTreeNode *Ptr, double *pos, double *vec)
{
int Child;
int MidX, MidY;


while (Ptr->Children) {

Child = GetChild(&Ptr->Bounder, pos[0], pos[1]);


MidX = CentreX(&Ptr->Bounder);
MidY = CentreY(&Ptr->Bounder);

if (abs((int)pos[0] - MidX) < ERROR_BOUNDARY)
Child = ToggleChildPtrX(vec[0], Child);

if (abs((int)pos[1] - MidY) < ERROR_BOUNDARY)
Child = ToggleChildPtrY(vec[1], Child);


Ptr = &Ptr->Children[Child];
}


return Ptr;
}

struct QuadTreeNode *GetCollisionNode(struct Level *lvl, double *pos,
double *vec)
{
return GetNode(&lvl->CollisionTree, pos, vec);
}







static void set_v(ALLEGRO_VERTEX *vt, double x, double y, double u, double v)
{
vt->x = x;
vt->y = y;
vt->z = 0;
vt->u = u;
vt->v = v;
vt->color = al_map_rgb_f(1, 1, 1);
}






static void DrawTriEdge(struct Triangle *tri, struct BoundingBox *ScrBounder)
{
ALLEGRO_VERTEX PolyEdges[4];
int c, c2;
double x, y, w;



if (Separated(&tri->Bounder, ScrBounder))
return;
c = 3;
while (c--) {
c2 = (c + 1) % 3;
if (tri->EdgeFlags[c] & TRIFLAGS_EDGE) {






x = tri->Edges[c]->Pos[0] - ScrBounder->TL.Pos[0];
y = tri->Edges[c]->Pos[1] - ScrBounder->TL.Pos[1];
w = tri->EdgeFlags[c] & TRIFLAGS_WIDTH;
set_v(PolyEdges + 0, x, y - w, tri->Edges[c]->Pos[0], 0);
set_v(PolyEdges + 3, x, y + w, tri->Edges[c]->Pos[0], al_get_bitmap_height(tri->Material->Edge));
x = tri->Edges[c2]->Pos[0] - ScrBounder->TL.Pos[0] + 1;
y = tri->Edges[c2]->Pos[1] - ScrBounder->TL.Pos[1];
w = tri->EdgeFlags[c2] & TRIFLAGS_WIDTH;
set_v(PolyEdges + 1, x, y - w, tri->Edges[c2]->Pos[0], 0);
set_v(PolyEdges + 2, x, y + w, tri->Edges[c2]->Pos[0], al_get_bitmap_height(tri->Material->Edge));

al_draw_prim(PolyEdges, NULL, tri->Material->Edge, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN);
}
}
}

static void setuv(ALLEGRO_VERTEX *v, struct BoundingBox *ScrBounder)
{
v->u = v->x + ScrBounder->TL.Pos[0];
v->v = v->y + ScrBounder->TL.Pos[1];

v->z = 0;
v->color = al_map_rgb_f(1, 1, 1);
}







static void DrawTriangle(struct Triangle *tri,
struct BoundingBox *ScrBounder)
{
ALLEGRO_VERTEX v[3];



if (Separated(&tri->Bounder, ScrBounder))
return;

v[0].x = tri->Edges[0]->Pos[0] - ScrBounder->TL.Pos[0];
v[0].y = tri->Edges[0]->Pos[1] - ScrBounder->TL.Pos[1];
setuv(&v[0], ScrBounder);
v[1].x = tri->Edges[1]->Pos[0] - ScrBounder->TL.Pos[0];
v[1].y = tri->Edges[1]->Pos[1] - ScrBounder->TL.Pos[1];
setuv(&v[1], ScrBounder);
v[2].x = tri->Edges[2]->Pos[0] - ScrBounder->TL.Pos[0];
v[2].y = tri->Edges[2]->Pos[1] - ScrBounder->TL.Pos[1];
setuv(&v[2], ScrBounder);

al_draw_prim(v, NULL, tri->Material->Fill, 0, 3, ALLEGRO_PRIM_TRIANGLE_STRIP);
}






static void DrawObject(struct Object *obj,
struct BoundingBox *ScrBounder)
{


if (Separated(&obj->Bounder, ScrBounder))
return;

if (obj->Flags & OBJFLAGS_VISIBLE)
al_draw_scaled_rotated_bitmap(obj->ObjType->Image, 0, 0,
(int)(0.5f *
(obj->Bounder.TL.Pos[0] +
obj->Bounder.BR.Pos[0]) -
ScrBounder->TL.Pos[0]) -
(al_get_bitmap_width(obj->ObjType->Image) >> 1),
(int)(0.5f *
(obj->Bounder.TL.Pos[1] +
obj->Bounder.BR.Pos[1]) -
ScrBounder->TL.Pos[1]) -
(al_get_bitmap_height(obj->ObjType->Image) >> 1),
1, 1,
obj->Angle, 0);
}







































static void GetQuadTreeVisibilityList(struct QuadTreeNode *TriTree,
struct Level *Lvl,
struct BoundingBox *ScrBounder)
{
int c;






if (Separated(&TriTree->Bounder, ScrBounder))
return;






if (TriTree->Children) {
c = 4;
while (c--)
GetQuadTreeVisibilityList(&TriTree->Children[c], Lvl, ScrBounder);
} else {





TriTree->Next = Lvl->VisibleList;
Lvl->VisibleList = TriTree;
}
}

static void DrawQuadTreePart(struct Level *Lvl,
struct BoundingBox *ScrBounder,
unsigned int framec, int PostContents)
{
struct Container *Thing, *TriStart;
struct QuadTreeNode *Visible;


Visible = Lvl->VisibleList;
while (Visible) {





Thing = PostContents ? Visible->PostContents : Visible->Contents;
while (Thing && Thing->Type == OBJECT) {
if (Thing->Content.O->LastFrame != framec) {
DrawObject(Thing->Content.O, ScrBounder);
Thing->Content.O->LastFrame = framec;
}
Thing = Thing->Next;
}

TriStart = Thing;
while (Thing) {
if (Thing->Content.T->LastFrame != framec)
DrawTriangle(Thing->Content.T, ScrBounder);
Thing = Thing->Next;
}








Thing = TriStart;
while (Thing) {
if (Thing->Content.T->LastFrame != framec) {
DrawTriEdge(Thing->Content.T, ScrBounder);
Thing->Content.T->LastFrame = framec;
}
Thing = Thing->Next;
}

Visible = Visible->Next;
}
}

void BeginQuadTreeDraw(struct Level *Lvl,
struct QuadTreeNode *TriTree,
struct BoundingBox *ScrBounder, unsigned int framec)
{

Lvl->VisibleList = NULL;
GetQuadTreeVisibilityList(TriTree, Lvl, ScrBounder);
DrawQuadTreePart(Lvl, ScrBounder, framec, false);
}

void EndQuadTreeDraw(struct Level *Lvl,
struct BoundingBox *ScrBounder, unsigned int framec)
{
DrawQuadTreePart(Lvl, ScrBounder, framec, true);
}






#define MAX_COLL 200
#define MAX_DISP 50









void AddTriangle(struct Level *level, struct Triangle *NewTri)
{
struct Container Cont;

Cont.Content.T = NewTri;
Cont.Type = TRIANGLE;
AddContent(&level->DisplayTree, &Cont, MAX_DISP);
}

void AddEdge(struct Level *level, struct Edge *NewEdge)
{
struct Container Cont;

Cont.Content.E = NewEdge;
Cont.Type = EDGE;
AddContent(&level->CollisionTree, &Cont, MAX_COLL);
}

void AddObject(struct Level *level, struct Object *NewObject, int DisplayTree)
{
struct Container Cont;

Cont.Content.O = NewObject;
Cont.Type = OBJECT;

if (DisplayTree) {
if (NewObject->Flags & OBJFLAGS_VISIBLE)
AddContent(&level->DisplayTree, &Cont, MAX_DISP);
} else {
if (NewObject->Flags & FLAGS_COLLIDABLE)
AddContent(&level->CollisionTree, &Cont, MAX_COLL);
}
}








void SplitTree(struct QuadTreeNode *Tree)
{
struct Container *P, *PNext;
int c = 4;

if (Tree->Children) {
while (c--)
SplitTree(&Tree->Children[c]);
} else if (Tree->Contents) {
P = Tree->Contents;
Tree->Contents = Tree->PostContents = NULL;

while (P) {
PNext = P->Next;

if (P->Type == OBJECT) {
if (P->Content.O->Flags & FLAGS_FOREGROUND) {
P->Next = Tree->PostContents;
Tree->PostContents = P;
} else {
P->Next = Tree->Contents;
Tree->Contents = P;
}
} else {
if ((P->Content.T->EdgeFlags[0] | P->Content.T->
EdgeFlags[1] | P->Content.T->EdgeFlags[2])
& FLAGS_FOREGROUND) {
P->Next = Tree->PostContents;
Tree->PostContents = P;
} else {
P->Next = Tree->Contents;
Tree->Contents = P;
}
}

P = PNext;
}
}
}

void OrderTree(struct QuadTreeNode *Tree, int PostTree)
{
struct Container *Objects, *NonObjects, *P, *PNext, **ITree;
int c = 4;

if (Tree->Children) {
while (c--)
OrderTree(&Tree->Children[c], PostTree);
} else {
ITree = PostTree ? &Tree->PostContents : &Tree->Contents;

if (*ITree) {

Objects = NonObjects = NULL;
P = *ITree;
while (P) {
PNext = P->Next;

if (P->Type == OBJECT) {
P->Next = Objects;
Objects = P;
} else {
P->Next = NonObjects;
NonObjects = P;
}

P = PNext;
}


*ITree = Objects;
while (*ITree)
ITree = &(*ITree)->Next;
*ITree = NonObjects;
}
}
}
