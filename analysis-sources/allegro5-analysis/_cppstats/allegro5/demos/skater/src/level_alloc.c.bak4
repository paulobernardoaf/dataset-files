#include <allegro5/allegro.h>
#include "level_alloc.h"
















struct Level *NewLevel()
{
struct Level *l = (struct Level *)malloc(sizeof(struct Level));

if (l) {
l->AllTris = NULL;
l->AllEdges = NULL;
l->AllMats = NULL;
l->AllVerts = NULL;
l->AllObjects = NULL;
l->AllObjectTypes = NULL;
l->InitialState = NULL;
l->DoorOpen = l->DoorShut = NULL;
l->Door.CollectNoise = NULL;

SetupQuadTree(&l->DisplayTree, -65536, -65536, 65536, 65536);
SetupQuadTree(&l->CollisionTree, -65536, -65536, 65536, 65536);
}

return l;
}

struct Material *NewMaterial()
{
struct Material *r = (struct Material *)malloc(sizeof(struct Material));

r->Next = NULL;
r->Edge = r->Fill = NULL;
r->Friction = 0;
return r;
}

struct ObjectType *NewObjectType()
{
struct ObjectType *r =
(struct ObjectType *)malloc(sizeof(struct ObjectType));

r->Next = NULL;
r->Image = NULL;
r->CollectNoise = NULL;
return r;
}

struct Triangle *NewTriangle()
{
struct Triangle *r = (struct Triangle *)malloc(sizeof(struct Triangle));

r->Next = NULL;
r->Material = NULL;
r->LastFrame = 0;
return r;
}

struct Object *NewObject()
{
struct Object *r = (struct Object *)malloc(sizeof(struct Object));

r->Next = NULL;
r->Flags = OBJFLAGS_VISIBLE;
r->ObjType = NULL;
return r;
}

struct Edge *NewEdge()
{
struct Edge *r = (struct Edge *)malloc(sizeof(struct Edge));

r->Next = NULL;
r->Material = NULL;
return r;
}

struct Vertex *NewVertex()
{
struct Vertex *v = (struct Vertex *)malloc(sizeof(struct Vertex));

v->Next = NULL;
v->Pos[0] = v->Pos[1] = 0;
v->Edges[0] = v->Edges[1] = NULL;
return v;
}
