

#ifndef __RAYLEVEL_H__
#define __RAYLEVEL_H__

typedef PVOID HRAYLEVEL;
typedef PVOID HRAYSPRITE;

#define MAIN_SPRITE   ((HRAYSPRITE)0)

#define NO_INTERSECTION ((UINT)-1)
#define EMPTY_CELL      ((DWORD)0)



typedef struct _WALL_GRAPHIC
{
	DWORD *pImageData;
	UINT  Height;
	UINT  Width;
	UINT  ImageNumber;

} WALL_GRAPHIC, *PWALL_GRAPHIC;

typedef enum _LIGHTING_TYPE
{
	NoLighting,
	SimpleLighting
} LIGHTING_TYPE, *PLIGHTING_TYPE;

typedef struct _RAYLEVEL_INIT
{
	PDWORD pLevelMap;
	UINT   ResolutionX;
	UINT   ResolutionY;
	UINT   CellResolution;
	UINT   ScreenWidth;
	UINT   ScreenHeight;

	double   CellX;
	double   CellY;
	UINT     MapIndexX;
	UINT     MapIndexY;

	UINT CollisionRadius;

	int DirectionAngle;

	UINT NumberOfWallGraphics;
	PWALL_GRAPHIC  pWallGraphicList;

	LIGHTING_TYPE LightingType;
	float         SimpleLightingLumination;
	float         SimpleLightingDistance;

	float SizeDistanceRatio;
	UINT  PointOfViewAngle;

} RAYLEVEL_INIT, *PRAYLEVEL_INIT;

HRAYLEVEL RayLevel_Init(PRAYLEVEL_INIT pRayLevelInit);
void RayLevel_Turn(HRAYLEVEL hRayLevel, HRAYSPRITE hRaySprite, int TurnAngleMod);
DWORD RayLevel_Move(HRAYLEVEL hRayLevel, HRAYSPRITE hRaySprite, int NumberSteps);
void RayLevel_DrawScene(HRAYLEVEL hRayLevel, DWORD *pScreenBuffer);


#endif


