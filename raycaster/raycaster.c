

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <raycaster.h>

#define FLOAT_TYPE float

#define RC_NORMALIZE_ANGLE(x) \
	        while(x < 0) { x += 360; }\
			while(x >= 360) { x -= 360; } 

#define RC_CONVERT_ANGLE_TO_RADIANS(x) (FLOAT_TYPE)((x)*3.14/180.0)


typedef enum _MOVE_TYPE
{
	MoveBackward,
	MoveForward
} MOVE_TYPE, *PMOVE_TYPE;


typedef enum _GLOBALS_STATE
{
	GlobalsUninitialized,
	GlobalsInitializing,
	GlobalsInitialized
} GLOBALS_STATE, *PGLOBALS_STATE;

/*
 * Map Level Data
 */
typedef struct _LEVEL_DATA
{
	PDWORD pLevelMap;

	UINT   ResolutionX;
	UINT   ResolutionY;

	UINT   CellResolution;

	PVOID  pContext;
	PFN_DRAW pfnDrawWallSlice;

} LEVEL_DATA, *PLEVEL_DATA;


/*
 * Location In Level
 */
typedef struct _LEVEL_LOCATION
{
	FLOAT_TYPE   CellX;
	FLOAT_TYPE   CellY;

	FLOAT_TYPE   fIncrementX;
	FLOAT_TYPE   fIncrementY;

	UINT     MapIndexX;
	UINT     MapIndexY;

	FLOAT_TYPE   fLastIncrementX;
	FLOAT_TYPE   fLastIncrementY;

	BOOL XIsBlocked;
	BOOL YIsBlocked;

	UINT     MapIntersectionX;
    UINT     MapIntersectionY;

	int      DirectionAngle;

	UINT CollisionRadius;

#ifndef USE_TRIG_TABLE 
	FLOAT_TYPE   DirectionRadians;
#endif

} LEVEL_LOCATION, *PLEVEL_LOCATION;


typedef struct _RAYCASTER
{
    LEVEL_DATA     LevelData;
    LEVEL_LOCATION LevelLocation;

} RAYCASTER, *PRAYCASTER;


/******************************************************* 
 * Globals
 *******************************************************/
#ifdef USE_TRIG_TABLE 
GLOBALS_STATE volatile  g_GloalsState = GlobalsUninitialized;
double g_COS[360];
double g_SIN[360];
#endif

/******************************************************* 
 * Internal Prototypes
 *******************************************************/
DWORD RayCaster_MoveInternal(PRAYCASTER pRayCaster, PLEVEL_LOCATION pLevelLocation, MOVE_TYPE MoveType);
DWORD RayCaster_CheckCollision(PRAYCASTER pRayCaster, PLEVEL_LOCATION pLevelLocation);
void RayCaster_Debug(char *pszFormatString, ...);
#ifdef USE_TRIG_TABLE 
void RayCaster_InitializeGlobals(void);
#endif

/**********************************************************************
 *
 *  RayCaster_Init
 *
 *
 *
 **********************************************************************/
HRAYCAST RayCaster_Init(PRAYCAST_INIT pRayCastInit)
{
	PRAYCASTER pRayCaster = NULL;
	DWORD SizeOfMapInBytes;

#ifdef USE_TRIG_TABLE 
	if(InterlockedCompareExchange((LONG volatile *)&g_GloalsState, GlobalsUninitialized, GlobalsInitializing) == GlobalsUninitialized)
	{
		RayCaster_InitializeGlobals();
		g_GloalsState = GlobalsInitialized;
	}
	else
	{
   	    while(g_GloalsState != GlobalsInitialized)
     	{ 
	       Sleep(1);
 	    }  
	}
#endif

	pRayCaster = (PRAYCASTER)LocalAlloc(LMEM_ZEROINIT, sizeof(RAYCASTER));

    if(pRayCaster)
	{
		pRayCaster->LevelData.ResolutionX = pRayCastInit->ResolutionX;
		pRayCaster->LevelData.ResolutionY = pRayCastInit->ResolutionY;
		pRayCaster->LevelData.CellResolution  = pRayCastInit->CellResolution;

		pRayCaster->LevelData.pfnDrawWallSlice = pRayCastInit->pfnDrawWallSlice;
		pRayCaster->LevelData.pContext         = pRayCastInit->pContext;

		pRayCaster->LevelLocation.CellX     = (FLOAT_TYPE)pRayCastInit->CellX;
		pRayCaster->LevelLocation.CellY     = (FLOAT_TYPE)pRayCastInit->CellY;
		pRayCaster->LevelLocation.MapIndexX = pRayCastInit->MapIndexX;
		pRayCaster->LevelLocation.MapIndexY = pRayCastInit->MapIndexY;
		pRayCaster->LevelLocation.CollisionRadius = pRayCastInit->CollisionRadius;

		pRayCaster->LevelLocation.DirectionAngle   = pRayCastInit->DirectionAngle;

		RC_NORMALIZE_ANGLE(pRayCaster->LevelLocation.DirectionAngle);

#ifdef USE_TRIG_TABLE
		pRayCaster->LevelLocation.fIncrementX      = g_COS[pRayCaster->LevelLocation.DirectionAngle];
	    pRayCaster->LevelLocation.fIncrementY      =  g_SIN[pRayCaster->LevelLocation.DirectionAngle];
#else
		pRayCaster->LevelLocation.DirectionRadians = RC_CONVERT_ANGLE_TO_RADIANS(pRayCaster->LevelLocation.DirectionAngle);

		pRayCaster->LevelLocation.fIncrementX      = (FLOAT_TYPE)cos(pRayCaster->LevelLocation.DirectionRadians);
	    pRayCaster->LevelLocation.fIncrementY      = (FLOAT_TYPE)sin(pRayCaster->LevelLocation.DirectionRadians);
#endif

		SizeOfMapInBytes = sizeof(DWORD)*pRayCastInit->ResolutionX*pRayCastInit->ResolutionY;
		
		pRayCaster->LevelData.pLevelMap = (PDWORD)LocalAlloc(LMEM_ZEROINIT, SizeOfMapInBytes);
		
		if(pRayCaster->LevelData.pLevelMap)
		{
			memcpy(pRayCaster->LevelData.pLevelMap, pRayCastInit->pLevelMap, SizeOfMapInBytes);
		}	
		else
		{
			LocalFree(pRayCaster);
			pRayCaster = NULL;
		}
	}

	return (HRAYCAST)pRayCaster;
}

#ifdef USE_TRIG_TABLE 
/**********************************************************************
 *
 *  RayCaster_InitializeGlobals
 *
 *
 *
 **********************************************************************/
void RayCaster_InitializeGlobals(void)
{
	UINT Angle;

	for(Angle = 0; Angle < 360; Angle++)
	{
		 g_COS[Angle] = cos(RC_CONVERT_ANGLE_TO_RADIANS(Angle));
		 g_SIN[Angle] = sin(RC_CONVERT_ANGLE_TO_RADIANS(Angle));
	}
}
#endif



/**********************************************************************
 *
 *  RayCaster_Turn
 *
 *
 *
 **********************************************************************/
void RayCaster_Turn(HRAYCAST hRayCast, int TurnAngleMod)
{
	PRAYCASTER pRayCaster = (PRAYCASTER)hRayCast;

	if(TurnAngleMod)
	{
		pRayCaster->LevelLocation.DirectionAngle += TurnAngleMod;
		RC_NORMALIZE_ANGLE(pRayCaster->LevelLocation.DirectionAngle);

#ifdef USE_TRIG_TABLE
		pRayCaster->LevelLocation.fIncrementX      = g_COS[pRayCaster->LevelLocation.DirectionAngle];
	    pRayCaster->LevelLocation.fIncrementY      =  g_SIN[pRayCaster->LevelLocation.DirectionAngle];
#else
		pRayCaster->LevelLocation.DirectionRadians = RC_CONVERT_ANGLE_TO_RADIANS(pRayCaster->LevelLocation.DirectionAngle);

		pRayCaster->LevelLocation.fIncrementX      = (FLOAT_TYPE)cos(pRayCaster->LevelLocation.DirectionRadians);
	    pRayCaster->LevelLocation.fIncrementY      = (FLOAT_TYPE)sin(pRayCaster->LevelLocation.DirectionRadians);
#endif

	}
}


/**********************************************************************
 *
 *  RayCaster_Move
 *
 *
 *
 **********************************************************************/
DWORD RayCaster_Move(HRAYCAST hRayCast, int NumberSteps)
{
	PRAYCASTER pRayCaster = (PRAYCASTER)hRayCast;
	DWORD CellEncountered = EMPTY_CELL;
		
	while(NumberSteps < 0 && CellEncountered == EMPTY_CELL)
	{
     	RayCaster_MoveInternal(pRayCaster, &pRayCaster->LevelLocation, MoveBackward);
    	CellEncountered = RayCaster_CheckCollision(pRayCaster, &pRayCaster->LevelLocation);
		NumberSteps++;
	}

	while(NumberSteps > 0 && CellEncountered == EMPTY_CELL)
	{
		RayCaster_MoveInternal(pRayCaster, &pRayCaster->LevelLocation, MoveForward);
		CellEncountered = RayCaster_CheckCollision(pRayCaster, &pRayCaster->LevelLocation);
		NumberSteps--;
	}

    return  CellEncountered;
}


/**********************************************************************
 *
 *  RayCaster_GetLocation
 *
 *
 *
 **********************************************************************/
void RayCaster_GetLocation(HRAYCAST hRayCast, PMAP_LOCATION pMapLocation)
{
	PRAYCASTER pRayCaster = (PRAYCASTER)hRayCast;

	pMapLocation->CellX          = pRayCaster->LevelLocation.CellX;
	pMapLocation->CellY          = pRayCaster->LevelLocation.CellY;
	pMapLocation->MapIndexX      = pRayCaster->LevelLocation.MapIndexX;
	pMapLocation->MapIndexY      = pRayCaster->LevelLocation.MapIndexY;
	pMapLocation->DirectionAngle = pRayCaster->LevelLocation.DirectionAngle;
}


/**********************************************************************
 *
 *  RayCaster_Cast
 *
 *
 *
 **********************************************************************/
void RayCaster_Cast(HRAYCAST hRayCast, UINT Width, UINT CellSize, UINT ViewAngle)
{
	PRAYCASTER pRayCaster = (PRAYCASTER)hRayCast;
	LEVEL_LOCATION LevelWalker;
	FLOAT_TYPE AngleIncrement;
	FLOAT_TYPE CurrentAngle;
	FLOAT_TYPE LocalViewAngle;
	int    RayIndex;
	DRAW_CONTEXT DrawContext;
	DWORD CellCollision;
	FLOAT_TYPE RayDistanceX;
	FLOAT_TYPE RayDistanceY;
	

#ifdef USE_TRIG_TABLE
	CurrentAngle      = (FLOAT_TYPE)pRayCaster->LevelLocation.DirectionAngle - (((FLOAT_TYPE)ViewAngle)/2.0);
	AngleIncrement    = ((FLOAT_TYPE)ViewAngle/(FLOAT_TYPE)Width);
	LocalViewAngle    = -1*ViewAngle/2;
#else
	CurrentAngle      = RC_CONVERT_ANGLE_TO_RADIANS(ViewAngle);
	AngleIncrement    = RC_CONVERT_ANGLE_TO_RADIANS((FLOAT_TYPE)ViewAngle/(FLOAT_TYPE)Width);
    LocalViewAngle    = (FLOAT_TYPE)(-1*CurrentAngle/2);

	CurrentAngle      = (FLOAT_TYPE)(pRayCaster->LevelLocation.DirectionRadians - ((CurrentAngle)/2.0));
#endif

	DrawContext.pContext = pRayCaster->LevelData.pContext;

	for(RayIndex = (int)Width - 1; RayIndex >= 0; CurrentAngle += AngleIncrement, RayIndex--, LocalViewAngle += AngleIncrement)
	{
#ifdef USE_TRIG_TABLE
		RC_NORMALIZE_ANGLE(CurrentAngle);
		RC_NORMALIZE_ANGLE(LocalViewAngle);
#endif
		memcpy(&LevelWalker, &pRayCaster->LevelLocation, sizeof(LEVEL_LOCATION));

		DrawContext.VerticleLine = RayIndex;

#ifdef USE_TRIG_TABLE
		LevelWalker.fIncrementY = g_SIN[(UINT)CurrentAngle];
		LevelWalker.fIncrementX = g_COS[(UINT)CurrentAngle];
#else
		LevelWalker.fIncrementY = (FLOAT_TYPE)sin(CurrentAngle);
		LevelWalker.fIncrementX = (FLOAT_TYPE)cos(CurrentAngle);
		LevelWalker.DirectionRadians = CurrentAngle;
#endif
		RayDistanceX = 0;
		RayDistanceY = 0;

		do {
			CellCollision = RayCaster_MoveInternal(pRayCaster, &LevelWalker, MoveForward);

			RayDistanceX += LevelWalker.fLastIncrementX;
			RayDistanceY += LevelWalker.fLastIncrementY;

		} while(CellCollision == EMPTY_CELL);

		DrawContext.ImageNumber = CellCollision;

#ifdef USE_TRIG_TABLE
		DrawContext.RayDistance = sqrt(RayDistanceX*RayDistanceX + RayDistanceY*RayDistanceY)*g_COS[(UINT)LocalViewAngle];
#else
		DrawContext.RayDistance = (FLOAT_TYPE)sqrt(RayDistanceX*RayDistanceX + RayDistanceY*RayDistanceY)*cos(LocalViewAngle);
#endif
		if(DrawContext.RayDistance < 0)
		{
			DrawContext.RayDistance = DrawContext.RayDistance*-1;
		}

	    DrawContext.MapIndexX = LevelWalker.MapIndexX;
	    DrawContext.MapIndexY = LevelWalker.MapIndexY;
	    DrawContext.MapIntersectionIndexX = LevelWalker.MapIntersectionX;
	    DrawContext.MapIntersectionIndexY = LevelWalker.MapIntersectionY;

		RayDistanceX += pRayCaster->LevelLocation.CellX;
		RayDistanceY += pRayCaster->LevelLocation.CellY;
		
		if(RayDistanceX < 0)
		{
			RayDistanceX = -1*RayDistanceX;
		}

    	if(RayDistanceY < 0)
		{
			RayDistanceY = -1*RayDistanceY;
		}

		DrawContext.CellIntersectionX =	((UINT)floor(RayDistanceX)) % CellSize;
		DrawContext.CellIntersectionY = ((UINT)floor(RayDistanceY)) % CellSize;

		pRayCaster->LevelData.pfnDrawWallSlice(hRayCast, &DrawContext);
	}
}


/**********************************************************************
 *
 *  RayCaster_CheckCollision
 *
 *
 *
 **********************************************************************/
DWORD RayCaster_CheckCollision(PRAYCASTER pRayCaster, PLEVEL_LOCATION pLevelLocation)
{
     DWORD CellEncountered = EMPTY_CELL;
	 DWORD CellEncounteredTemp;
	 FLOAT_TYPE CollisionCheckX;
	 FLOAT_TYPE CollisionCheckY;
	 UINT Angle;
	 FLOAT_TYPE Radians;
	 BOOL ReCheck;
	 DWORD MapIndexY;
	 DWORD MapIndexX;
		 
 	 MapIndexX = pLevelLocation->MapIndexX;
	 MapIndexY = pLevelLocation->MapIndexY;

	 for(Angle = 0; Angle < 360;)
	 {
		 ReCheck = FALSE;
		 Radians = RC_CONVERT_ANGLE_TO_RADIANS(Angle);
		 CollisionCheckX = (FLOAT_TYPE)(cos(Radians)*pLevelLocation->CollisionRadius + pLevelLocation->CellX);
		 CollisionCheckY = (FLOAT_TYPE)(sin(Radians)*pLevelLocation->CollisionRadius + pLevelLocation->CellY);

#if DBG
		 RayCaster_Debug("(%1.2f, %1.2f) %1.2f, %1.2f, %i\n", pLevelLocation->CellX, pLevelLocation->CellY, CollisionCheckX, CollisionCheckY, Angle);
#endif

		 if(CollisionCheckX < 0)
		 {
			 CellEncounteredTemp = pRayCaster->LevelData.pLevelMap[MapIndexX - 1 + (MapIndexY*pRayCaster->LevelData.ResolutionX)];

		     if(CellEncounteredTemp != EMPTY_CELL)
			 {
				 pLevelLocation->CellX = (FLOAT_TYPE)pLevelLocation->CollisionRadius;
				 ReCheck = TRUE;
			 }
		 }
		 else
		 {
			 if(CollisionCheckX > (pRayCaster->LevelData.CellResolution - 1))
			 {
				 CellEncounteredTemp = pRayCaster->LevelData.pLevelMap[MapIndexX + 1 + (MapIndexY*pRayCaster->LevelData.ResolutionX)];

				 if(CellEncounteredTemp != EMPTY_CELL)
				 {
					 pLevelLocation->CellX = (FLOAT_TYPE)(pRayCaster->LevelData.CellResolution - 1) - pLevelLocation->CollisionRadius;
					 ReCheck = TRUE;
				 }
			 }
		 }

		 if(ReCheck == FALSE)
		 {
			 if(CollisionCheckY < 0)
			 {
				 CellEncounteredTemp = pRayCaster->LevelData.pLevelMap[MapIndexX + ((MapIndexY-1)*pRayCaster->LevelData.ResolutionX)];

				 if(CellEncounteredTemp != EMPTY_CELL)
				 {
					 pLevelLocation->CellY = (FLOAT_TYPE)pLevelLocation->CollisionRadius;
					 ReCheck = TRUE;
				 }
			 }
			 else
			 {
				 if(CollisionCheckY > (pRayCaster->LevelData.CellResolution - 1))
				 {
					 CellEncounteredTemp = pRayCaster->LevelData.pLevelMap[MapIndexX + ((MapIndexY+1)*pRayCaster->LevelData.ResolutionX)];

					 if(CellEncounteredTemp != EMPTY_CELL)
					 {
						 pLevelLocation->CellY = (FLOAT_TYPE)(pRayCaster->LevelData.CellResolution - 1) - pLevelLocation->CollisionRadius;
						 ReCheck = TRUE;
					 }
				 }
			 }
		}

		 if(ReCheck == FALSE)
		 { 
			 Angle += 45;
		 }
		 else
		 {
			 if(CellEncountered == EMPTY_CELL)
			 {
				 CellEncountered = CellEncounteredTemp;
			 }

			 Angle = 0;
		 }
	 }
#if DBG
	 RayCaster_Debug("\n\n");
#endif

	 return CellEncountered;
}

/**********************************************************************
 *
 *  RayCaster_MoveInternal
 *
 *
 *
 **********************************************************************/
DWORD RayCaster_MoveInternal(PRAYCASTER pRayCaster, PLEVEL_LOCATION pLevelLocation, MOVE_TYPE MoveType)
{
	DWORD CellEncountered = EMPTY_CELL;
	FLOAT_TYPE NewCellX;
	FLOAT_TYPE NewCellY;
	FLOAT_TYPE DistanceXEndOfCell;
	FLOAT_TYPE DistanceYEndOfCell;
	FLOAT_TYPE DistanceXNewCell;
	FLOAT_TYPE DistanceYNewCell;
	BOOL NewCellXIsNextCell;
	BOOL NewCellYIsNextCell;
	UINT NewMapIndexX;
	UINT NewMapIndexY;

	pLevelLocation->MapIntersectionX = NO_INTERSECTION;
	pLevelLocation->MapIntersectionY = NO_INTERSECTION;

	if(MoveType == MoveForward)
	{
		NewCellX = pLevelLocation->CellX + pLevelLocation->fIncrementX;
		NewCellY = pLevelLocation->CellY + pLevelLocation->fIncrementY;
	}
	else
	{
		NewCellX = pLevelLocation->CellX - pLevelLocation->fIncrementX;
		NewCellY = pLevelLocation->CellY - pLevelLocation->fIncrementY;
	}	

	NewCellXIsNextCell = (NewCellX < 0 || NewCellX > (pRayCaster->LevelData.CellResolution-1)) ? TRUE : FALSE;
	NewCellYIsNextCell = (NewCellY < 0 || NewCellY > (pRayCaster->LevelData.CellResolution-1)) ? TRUE : FALSE;

	if(NewCellXIsNextCell || NewCellYIsNextCell)
	{
		NewMapIndexX = pLevelLocation->MapIndexX;
		NewMapIndexY = pLevelLocation->MapIndexY;

		if(NewCellXIsNextCell)
		{
			if(NewCellX < 0)
			{
				NewCellX = (pRayCaster->LevelData.CellResolution - 1) + NewCellX;
				NewMapIndexX--;

				DistanceXEndOfCell = pLevelLocation->CellX;
				DistanceXNewCell   = (pRayCaster->LevelData.CellResolution - 1) - NewCellX;
			}
			else
			{
				NewCellX = NewCellX - (pRayCaster->LevelData.CellResolution - 1);
				NewMapIndexX++;

				DistanceXEndOfCell = (pRayCaster->LevelData.CellResolution - 1) - pLevelLocation->CellX;
				DistanceXNewCell = NewCellX;
			}
		}

		if(NewCellYIsNextCell)
		{
			if(NewCellY < 0)
			{
				NewCellY = (pRayCaster->LevelData.CellResolution - 1) + NewCellY;
				NewMapIndexY--;

				DistanceYEndOfCell = pLevelLocation->CellY;
				DistanceYNewCell   = (pRayCaster->LevelData.CellResolution - 1) - NewCellY;
			}
			else
			{
				NewCellY = NewCellY - (pRayCaster->LevelData.CellResolution - 1);
				NewMapIndexY++;
				
				DistanceYEndOfCell = (pRayCaster->LevelData.CellResolution - 1) - pLevelLocation->CellY;
				DistanceYNewCell = NewCellY;
			}
		}

		CellEncountered = pRayCaster->LevelData.pLevelMap[NewMapIndexX + (NewMapIndexY*pRayCaster->LevelData.ResolutionX)];

		if(CellEncountered == EMPTY_CELL)
		{
			pLevelLocation->MapIndexX = NewMapIndexX;
			pLevelLocation->MapIndexY = NewMapIndexY;

			if(NewCellXIsNextCell)
			{
				pLevelLocation->fLastIncrementX = DistanceXNewCell + DistanceXEndOfCell;
			}
			else
			{
				pLevelLocation->fLastIncrementX = NewCellX - pLevelLocation->CellX;
			}

			if(NewCellYIsNextCell)
			{
				pLevelLocation->fLastIncrementY = DistanceYNewCell + DistanceYEndOfCell;
			}
			else
			{
				pLevelLocation->fLastIncrementY = NewCellY - pLevelLocation->CellY;
			}

			pLevelLocation->CellX = NewCellX;
			pLevelLocation->CellY = NewCellY;

		}
		else
		{
			pLevelLocation->MapIntersectionX = NewMapIndexX;
			pLevelLocation->MapIntersectionY = NewMapIndexY;

			pLevelLocation->XIsBlocked = NewCellXIsNextCell;
	        pLevelLocation->YIsBlocked = NewCellYIsNextCell;

			if(NewCellXIsNextCell)
			{
				pLevelLocation->fLastIncrementX = DistanceXEndOfCell;
			}
			else
			{
				pLevelLocation->fLastIncrementX = NewCellX - pLevelLocation->CellX;
			}

			if(NewCellYIsNextCell)
			{
				pLevelLocation->fLastIncrementY = DistanceYEndOfCell;
			}
			else
			{
				pLevelLocation->fLastIncrementY = NewCellY - pLevelLocation->CellY;
			}
		}
	}
	else
	{
		pLevelLocation->fLastIncrementX = NewCellX - pLevelLocation->CellX;
		pLevelLocation->fLastIncrementY = NewCellY - pLevelLocation->CellY;

		pLevelLocation->CellX = NewCellX;
		pLevelLocation->CellY = NewCellY;
	}

    return CellEncountered;
}



/***********************************************************************
 * RayCaster_Debug
 *  
 *    Debug Shit
 *
 *    
 *
 * Parameters
 *     Debug
 *
 * Return Value
 *     Nothing
 *
 ***********************************************************************/
 void RayCaster_Debug(char *pszFormatString, ...)
 {
     char DebugString[256];
     va_list vl;

     va_start(vl, pszFormatString);
     vsprintf(DebugString, pszFormatString, vl);
     va_end(vl);

     OutputDebugStringA(DebugString);
 }