#ifndef __3DS_H__
#define __3DS_H__


typedef PVOID H3DS;

H3DS ThreeDS_Init(H3D h3D);
void ThreeDS_Close(H3DS h3Ds);
BOOL ThreeDS_Draw(H3DS h3Ds);
BOOL ThreeDS_Draw2(H3DS h3Ds);
BOOL ThreeDS_Draw3(H3DS h3Ds);
void ThreeDS_Rotate(PVOID Context, PTD_ROTATION pTdRotation);
void ThreeDS_World(PVOID Context, PTD_POINT pWorld);
#endif

