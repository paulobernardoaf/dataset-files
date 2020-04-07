#include <allegro5/allegro.h>
#include <math.h>
#include "game.h"
#include "physics.h"























static double FixUp(double *vec, double normalx, double normaly)
{
double mul;

if (KeyFlags & KEYFLAG_JUMPING)
KeyFlags &= ~(KEYFLAG_JUMPING | KEYFLAG_JUMP | KEYFLAG_JUMP_ISSUED);

mul = vec[0] * normalx + vec[1] * normaly;
vec[0] -= mul * normalx;
vec[1] -= mul * normaly;
return mul;
}





















static void SetAngle(double normalx, double normaly, double *a, int JustDoIt)
{
double NewAng = atan2(normalx, -normaly);

if (JustDoIt || ((NewAng < (ALLEGRO_PI * 0.25f)) && (NewAng >= 0)) ||
((NewAng > (-ALLEGRO_PI * 0.25f)) && (NewAng <= 0)) ||
fabs(NewAng - *a) < (ALLEGRO_PI * 0.25f)
)
*a = NewAng;
}














static void DoFriction(double r, struct Edge *E, double *vec)
{

double ForwardSpeed =
(vec[0] * E->b - vec[1] * E->a) / E->Material->Friction,
UpSpeed = vec[0] * E->a + vec[1] * E->b;
double FricLevel;

(void)r;


if (KeyFlags & KEYFLAG_LEFT) {
ForwardSpeed += Pusher;
KeyFlags |= KEYFLAG_FLIP;
}
if (KeyFlags & KEYFLAG_RIGHT) {
ForwardSpeed -= Pusher;
KeyFlags &= ~KEYFLAG_FLIP;
}


FricLevel = 0.05f * E->Material->Friction;
if (ForwardSpeed > 0) {
if (ForwardSpeed < FricLevel)
ForwardSpeed = 0;
else
ForwardSpeed -= FricLevel;
} else {
if (ForwardSpeed > -FricLevel)
ForwardSpeed = 0;
else
ForwardSpeed += FricLevel;
}


if ((KeyFlags & (KEYFLAG_JUMP | KEYFLAG_JUMP_ISSUED)) == KEYFLAG_JUMP) {
UpSpeed += 5.0f;
KeyFlags |= KEYFLAG_JUMP_ISSUED | KEYFLAG_JUMPING;
}


vec[0] = UpSpeed * E->a + ForwardSpeed * E->Material->Friction * E->b;
vec[1] = UpSpeed * E->b - ForwardSpeed * E->Material->Friction * E->a;
}





















#if 0 
struct QuadTreeNode *DoContinuousPhysics(struct Level *lvl, struct QuadTreeNode
*CollTree, double *pos,
double *vec, double TimeToGo,
struct Animation *PAnim)
{
double MoveVec[2];
double CollTime, NCollTime;
double End[2], ColPoint[2];
void *CollPtr;
double d1, d2;
struct Edge *E;
double r;
int Contact;
struct Container *EPtr, *FirstEdge;


CollTree = GetCollisionNode(lvl, pos, vec);
EPtr = CollTree->Contents;
while (EPtr && EPtr->Type != EDGE)
EPtr = EPtr->Next;
FirstEdge = EPtr;


do {
Contact = false;


EPtr = FirstEdge;
while (EPtr) {

d1 =
EPtr->Content.E->a * pos[0] +
EPtr->Content.E->b * pos[1] + EPtr->Content.E->c;

if (d1 >= (-0.5f) && d1 <= 0.05f &&
pos[0] >= EPtr->Content.E->Bounder.TL.Pos[0]
&& pos[0] <= EPtr->Content.E->Bounder.BR.Pos[0]
&& pos[1] >= EPtr->Content.E->Bounder.TL.Pos[1]
&& pos[1] <= EPtr->Content.E->Bounder.BR.Pos[1]
) {
pos[0] += EPtr->Content.E->a * (0.05f - d1);
pos[1] += EPtr->Content.E->b * (0.05f - d1);
Contact = true;
}

EPtr = EPtr->Next;
}

MoveVec[0] = TimeToGo * vec[0];
MoveVec[1] = TimeToGo * vec[1];

CollTime = TimeToGo + 1.0f;
End[0] = pos[0] + MoveVec[0];
End[1] = pos[1] + MoveVec[1];
CollPtr = NULL;




if (End[0] > CollTree->Bounder.BR.Pos[0]) {
NCollTime = (End[0] - CollTree->Bounder.BR.Pos[0]) / MoveVec[0];
if (NCollTime < CollTime) {
CollTime = NCollTime;
CollPtr = (void *)CollTree;
}
}

if (End[0] < CollTree->Bounder.TL.Pos[0]) {
NCollTime = (End[0] - CollTree->Bounder.TL.Pos[0]) / MoveVec[0];
if (NCollTime < CollTime) {
CollTime = NCollTime;
CollPtr = (void *)CollTree;
}
}

if (End[1] > CollTree->Bounder.BR.Pos[1]) {
NCollTime = (End[1] - CollTree->Bounder.BR.Pos[1]) / MoveVec[1];
if (NCollTime < CollTime) {
CollTime = NCollTime;
CollPtr = (void *)CollTree;
}
}

if (End[1] < CollTree->Bounder.TL.Pos[1]) {
NCollTime = (End[1] - CollTree->Bounder.TL.Pos[1]) / MoveVec[1];
if (NCollTime < CollTime) {
CollTime = NCollTime;
CollPtr = (void *)CollTree;
}
}


EPtr = FirstEdge;
while (EPtr) {

d1 =
EPtr->Content.E->a * pos[0] +
EPtr->Content.E->b * pos[1] + EPtr->Content.E->c;
d2 =
EPtr->Content.E->a * End[0] +
EPtr->Content.E->b * End[1] + EPtr->Content.E->c;
if ((d1 >= (-0.05f)) && (d2 < 0)) {
NCollTime = d1 / (d1 - d2);

ColPoint[0] = pos[0] + NCollTime * MoveVec[0];
ColPoint[1] = pos[1] + NCollTime * MoveVec[1];
if (ColPoint[0] >= EPtr->Content.E->Bounder.TL.Pos[0]
&& ColPoint[0] <= EPtr->Content.E->Bounder.BR.Pos[0]
&& ColPoint[1] >= EPtr->Content.E->Bounder.TL.Pos[1]
&& ColPoint[1] <= EPtr->Content.E->Bounder.BR.Pos[1]
) {
CollTime = NCollTime;
CollPtr = (void *)EPtr;
}
}


EPtr = EPtr->Next;
}


pos[0] += MoveVec[0] * CollTime;
pos[1] += MoveVec[1] * CollTime;
if (!Contact)
AdvanceAnimation(PAnim, 0, false);


if (CollPtr) {
if (CollPtr == (void *)CollTree) {
CollTree = GetCollisionNode(lvl, pos, vec);


EPtr = CollTree->Contents;
while (EPtr && EPtr->Type != EDGE)
EPtr = EPtr->Next;
FirstEdge = EPtr;
} else {

E = ((struct Container *)CollPtr)->Content.E;
r = FixUp(vec, E->a, E->b);

SetAngle(E->a, E->b, &pos[2],
(fabs(vec[0] * E->b - vec[1] * E->a) > 0.5f));
AdvanceAnimation(PAnim,
(vec[0] * E->b -
vec[1] * E->a) *
((KeyFlags & KEYFLAG_FLIP) ? 1.0f : -1.0f),
true);


DoFriction(r, E, vec);
}
} else if (!Contact) {

if (KeyFlags & KEYFLAG_LEFT) {
vec[0] -= 0.05f;
KeyFlags |= KEYFLAG_FLIP;
}
if (KeyFlags & KEYFLAG_RIGHT) {
vec[0] += 0.05f;
KeyFlags &= ~KEYFLAG_FLIP;
}
if (pos[2] > 0) {
pos[2] -= TimeToGo * CollTime * 0.03f;
if (pos[2] < 0)
pos[2] = 0;
}
if (pos[2] < 0) {
pos[2] += TimeToGo * CollTime * 0.03f;
if (pos[2] > 0)
pos[2] = 0;
}
}


TimeToGo -= TimeToGo * CollTime;
}
while (TimeToGo > 0.01f);

return CollTree;
}
#endif































struct QuadTreeNode *RunPhysics(struct Level *lvl, double *pos,
double *vec, double TimeToGo,
struct Animation *PAnim)
{
double MoveVec[2];
struct QuadTreeNode *CollTree = GetCollisionNode(lvl, pos, vec);
double CollTime, NCollTime;
double End[2], ColPoint[2];
void *CollPtr;
double d1, d2;
struct Edge *E;
double r;
int Contact;
struct Container *EPtr, *FirstEdge;


vec[1] += 0.1f;


vec[0] *= 0.997f;
vec[1] *= 0.997f;

EPtr = CollTree->Contents;
while (EPtr && EPtr->Type != EDGE)
EPtr = EPtr->Next;
FirstEdge = EPtr;


do {
Contact = false;


EPtr = FirstEdge;
while (EPtr) {

d1 =
EPtr->Content.E->a * pos[0] +
EPtr->Content.E->b * pos[1] + EPtr->Content.E->c;

if (d1 >= (-0.5f) && d1 <= 0.05f &&
pos[0] >= EPtr->Content.E->Bounder.TL.Pos[0]
&& pos[0] <= EPtr->Content.E->Bounder.BR.Pos[0]
&& pos[1] >= EPtr->Content.E->Bounder.TL.Pos[1]
&& pos[1] <= EPtr->Content.E->Bounder.BR.Pos[1]
) {
pos[0] += EPtr->Content.E->a * (0.05f - d1);
pos[1] += EPtr->Content.E->b * (0.05f - d1);
Contact = true;
}

EPtr = EPtr->Next;
}

MoveVec[0] = TimeToGo * vec[0];
MoveVec[1] = TimeToGo * vec[1];

CollTime = 1.0f;
End[0] = pos[0] + MoveVec[0];
End[1] = pos[1] + MoveVec[1];
CollPtr = NULL;




if (End[0] > CollTree->Bounder.BR.Pos[0]) {
NCollTime = (End[0] - CollTree->Bounder.BR.Pos[0]) / MoveVec[0];
if (NCollTime < CollTime) {
CollTime = NCollTime;
CollPtr = (void *)CollTree;
}
}

if (End[0] < CollTree->Bounder.TL.Pos[0]) {
NCollTime = (End[0] - CollTree->Bounder.TL.Pos[0]) / MoveVec[0];
if (NCollTime < CollTime) {
CollTime = NCollTime;
CollPtr = (void *)CollTree;
}
}

if (End[1] > CollTree->Bounder.BR.Pos[1]) {
NCollTime = (End[1] - CollTree->Bounder.BR.Pos[1]) / MoveVec[1];
if (NCollTime < CollTime) {
CollTime = NCollTime;
CollPtr = (void *)CollTree;
}
}

if (End[1] < CollTree->Bounder.TL.Pos[1]) {
NCollTime = (End[1] - CollTree->Bounder.TL.Pos[1]) / MoveVec[1];
if (NCollTime < CollTime) {
CollTime = NCollTime;
CollPtr = (void *)CollTree;
}
}


EPtr = FirstEdge;
while (EPtr) {

d1 =
EPtr->Content.E->a * pos[0] +
EPtr->Content.E->b * pos[1] + EPtr->Content.E->c;
d2 =
EPtr->Content.E->a * End[0] +
EPtr->Content.E->b * End[1] + EPtr->Content.E->c;
if ((d1 >= (-0.05f)) && (d2 < 0)) {
NCollTime = d1 / (d1 - d2);

ColPoint[0] = pos[0] + NCollTime * MoveVec[0];
ColPoint[1] = pos[1] + NCollTime * MoveVec[1];
if (ColPoint[0] >= EPtr->Content.E->Bounder.TL.Pos[0]
&& ColPoint[0] <= EPtr->Content.E->Bounder.BR.Pos[0]
&& ColPoint[1] >= EPtr->Content.E->Bounder.TL.Pos[1]
&& ColPoint[1] <= EPtr->Content.E->Bounder.BR.Pos[1]
) {
CollTime = NCollTime;
CollPtr = (void *)EPtr;
}
}


EPtr = EPtr->Next;
}


pos[0] += MoveVec[0] * CollTime;
pos[1] += MoveVec[1] * CollTime;
if (!Contact)
AdvanceAnimation(PAnim, 0, false);


if (CollPtr) {
if (CollPtr == (void *)CollTree) {
CollTree = GetCollisionNode(lvl, pos, vec);
EPtr = CollTree->Contents;
while (EPtr && EPtr->Type != EDGE)
EPtr = EPtr->Next;
FirstEdge = EPtr;
} else {

E = ((struct Container *)CollPtr)->Content.E;
r = FixUp(vec, E->a, E->b);

SetAngle(E->a, E->b, &pos[2],
(fabs(vec[0] * E->b - vec[1] * E->a) > 0.5f));
AdvanceAnimation(PAnim,
(vec[0] * E->b -
vec[1] * E->a) *
((KeyFlags & KEYFLAG_FLIP) ? 1.0f : -1.0f),
true);


DoFriction(r, E, vec);
}
} else if (!Contact) {

if (KeyFlags & KEYFLAG_LEFT) {
vec[0] -= 0.05f;
KeyFlags |= KEYFLAG_FLIP;
}
if (KeyFlags & KEYFLAG_RIGHT) {
vec[0] += 0.05f;
KeyFlags &= ~KEYFLAG_FLIP;
}
if (pos[2] > 0) {
pos[2] -= TimeToGo * CollTime * 0.03f;
if (pos[2] < 0)
pos[2] = 0;
}
if (pos[2] < 0) {
pos[2] += TimeToGo * CollTime * 0.03f;
if (pos[2] > 0)
pos[2] = 0;
}
}


TimeToGo -= TimeToGo * CollTime;
}
while (TimeToGo > 0.01f);

return CollTree;
}
