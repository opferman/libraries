

#ifndef __RAYCASTER_H__
#define __RAYCASTER_H__

typedef PVOID HRAYCAST;

#define NO_INTERSECTION ((UINT)-1)
#define EMPTY_CELL      ((DWORD)0)

typedef struct _DRAW_CONTEXT
{
	PVOID   pContext;
	double  RayDistance;
	UINT    VerticleLine;
	UINT    ImageNumber;

	UINT    CellIntersectionX;
	UINT    CellIntersectionY;

	UINT    MapIndexX;
	UINT    MapIndexY;
	UINT    MapIntersectionIndexX;
	UINT    MapIntersectionIndexY;

} DRAW_CONTEXT, *PDRAW_CONTEXT;

typedef void (WINAPI *PFN_DRAW)(HRAYCAST hRayCast, PDRAW_CONTEXT pDrawContext);

typedef struct _RAYCAST_INIT
{

	PDWORD pLevelMap;
	UINT   ResolutionX;
	UINT   ResolutionY;
	UINT   CellResolution;

	PVOID  pContext;
	PFN_DRAW pfnDrawWallSlice;

	double   CellX;
	double   CellY;
	UINT     MapIndexX;
	UINT     MapIndexY;

	UINT CollisionRadius;

	int DirectionAngle;

} RAYCAST_INIT, *PRAYCAST_INIT;


typedef struct _MAP_LOCATION
{
	double   CellX;
	double   CellY;
	UINT     MapIndexX;
	UINT     MapIndexY;
	int      DirectionAngle;

} MAP_LOCATION, *PMAP_LOCATION;

HRAYCAST RayCaster_Init(PRAYCAST_INIT pRayCastInit);
void RayCaster_Turn(HRAYCAST hRayCast, int TurnAngleMod);
DWORD RayCaster_Move(HRAYCAST hRayCast, int NumberSteps);
void RayCaster_GetLocation(HRAYCAST hRayCast, PMAP_LOCATION pMapLocation);
void RayCaster_Cast(HRAYCAST hRayCast, UINT Width, UINT CellSize, UINT ViewAngle);




#endif


