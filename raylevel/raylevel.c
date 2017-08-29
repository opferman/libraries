

#include <windows.h>
#include <stdio.h>
#include <raylevel.h>
#include <raycaster.h>



typedef struct _RAYLEVEL
{
    UINT   ResolutionX;
	UINT   ResolutionY;
	UINT   ScreenWidth;
	UINT   ScreenHeight;
	UINT   CellResolution;
	UINT   PointOfViewAngle;
	float  SizeDistanceRatio;

	LIGHTING_TYPE LightingType;
	float         SimpleLightingLumination;
	float         SimpleLightingDistance;

	UINT NumberOfWallGraphics;
	PWALL_GRAPHIC  pWallGraphicList;
	DWORD *pScreenBuffer;

	HRAYCAST hRayCast;


} RAYLEVEL, *PRAYLEVEL;

/******************************************************* 
 * Internal Prototypes
 *******************************************************/
 void RayLevel_Debug(char *pszFormatString, ...);
 void WINAPI RayLevel_DrawLine(HRAYCAST hRayCast, PDRAW_CONTEXT pDrawContext);


/**********************************************************************
 *
 *  RayLevel_Init
 *
 *
 *
 **********************************************************************/
HRAYLEVEL RayLevel_Init(PRAYLEVEL_INIT pRayLevelInit)
{
	RAYCAST_INIT RayCastInit = {0};
	PRAYLEVEL pRayLevel = NULL;
	DWORD SizeOfImageListInBytes;

	pRayLevel = (PRAYLEVEL)LocalAlloc(LMEM_ZEROINIT, sizeof(RAYLEVEL));

    if(pRayLevel)
	{
		RayCastInit.ResolutionX    = pRayLevelInit->ResolutionX;
		RayCastInit.ResolutionY    = pRayLevelInit->ResolutionY;
		RayCastInit.CellResolution = pRayLevelInit->CellResolution;

		RayCastInit.pfnDrawWallSlice = RayLevel_DrawLine;
		RayCastInit.pContext         = (PVOID)pRayLevel;

		RayCastInit.CellX = pRayLevelInit->CellX;
		RayCastInit.CellY = pRayLevelInit->CellY;

		RayCastInit.DirectionAngle  = pRayLevelInit->DirectionAngle;
		RayCastInit.CollisionRadius = pRayLevelInit->CollisionRadius;
		
		RayCastInit.pLevelMap = pRayLevelInit->pLevelMap;
		RayCastInit.MapIndexX = pRayLevelInit->MapIndexX;
		RayCastInit.MapIndexY = pRayLevelInit->MapIndexY;

		pRayLevel->SizeDistanceRatio = pRayLevelInit->SizeDistanceRatio;
		pRayLevel->LightingType      = pRayLevelInit->LightingType;
		pRayLevel->SimpleLightingLumination = pRayLevelInit->SimpleLightingLumination;

		pRayLevel->NumberOfWallGraphics = pRayLevelInit->NumberOfWallGraphics;
		SizeOfImageListInBytes          = pRayLevelInit->NumberOfWallGraphics*sizeof(WALL_GRAPHIC);

		pRayLevel->pWallGraphicList = (PWALL_GRAPHIC)LocalAlloc(LMEM_ZEROINIT, SizeOfImageListInBytes);
		pRayLevel->ScreenWidth      = pRayLevelInit->ScreenWidth;
	    pRayLevel->ScreenHeight     = pRayLevelInit->ScreenHeight;
		pRayLevel->PointOfViewAngle = pRayLevelInit->PointOfViewAngle;
		pRayLevel->CellResolution   = pRayLevelInit->CellResolution;
		pRayLevel->ResolutionX      = pRayLevelInit->ResolutionX;
		pRayLevel->ResolutionY      = pRayLevelInit->ResolutionY;
		pRayLevel->SimpleLightingDistance = pRayLevelInit->SimpleLightingDistance;
		

		if(pRayLevel->pWallGraphicList)
		{
			memcpy(pRayLevel->pWallGraphicList, pRayLevelInit->pWallGraphicList, SizeOfImageListInBytes);

			pRayLevel->hRayCast = RayCaster_Init(&RayCastInit);

			if(pRayLevel->hRayCast == NULL)
			{
				LocalFree(pRayLevel->pWallGraphicList);
				LocalFree(pRayLevel);
			    pRayLevel = NULL;
			}
		}	
		else
		{
			LocalFree(pRayLevel);
			pRayLevel = NULL;
		}
	}

	return (HRAYLEVEL)pRayLevel;
}






/**********************************************************************
 *
 *  RayLevel_Turn
 *
 *
 *
 **********************************************************************/
void RayLevel_Turn(HRAYLEVEL hRayLevel, HRAYSPRITE hRaySprite, int TurnAngleMod)
{
	PRAYLEVEL pRayLevel = (PRAYLEVEL)hRayLevel;

	if(TurnAngleMod)
	{
		if(hRaySprite == MAIN_SPRITE)
		{
			RayCaster_Turn(pRayLevel->hRayCast, TurnAngleMod);
		}
	}
}


/**********************************************************************
 *
 *  RayLevel_Move
 *
 *
 *
 **********************************************************************/
DWORD RayLevel_Move(HRAYLEVEL hRayLevel, HRAYSPRITE hRaySprite, int NumberSteps)
{
	PRAYLEVEL pRayLevel = (PRAYLEVEL)hRayLevel;
	DWORD CellEncountered = EMPTY_CELL;
	
	if(hRaySprite == MAIN_SPRITE)
	{
		CellEncountered = RayCaster_Move(pRayLevel->hRayCast, NumberSteps);
	}

    return  CellEncountered;
}




/**********************************************************************
 *
 *  RayLevel_DrawScene
 *
 *
 *
 **********************************************************************/
void RayLevel_DrawScene(HRAYLEVEL hRayLevel, DWORD *pScreenBuffer)
{
	PRAYLEVEL pRayLevel = (PRAYLEVEL)hRayLevel;

	pRayLevel->pScreenBuffer = pScreenBuffer;
	RayCaster_Cast(pRayLevel->hRayCast, pRayLevel->ScreenWidth, pRayLevel->CellResolution, pRayLevel->PointOfViewAngle);
	pRayLevel->pScreenBuffer = NULL;
}

/***********************************************************************
 * RayLevel_DrawLine
 *  
 *    
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
void WINAPI RayLevel_DrawLine(HRAYCAST hRayCast, PDRAW_CONTEXT pDrawContext)
{
	PRAYLEVEL pRayLevel = (PRAYLEVEL)pDrawContext->pContext;
	UINT ScreenLocationY;
	UINT ScreenLocationYEnd;
	UINT ImageHeightSize;
	UINT ImageLocationX;
	float ImageLocationY;
	float ImageIncrementY;
	UINT ImageIndex;
	DWORD DrawColor;
	
	ImageHeightSize = pRayLevel->ScreenHeight;
	ImageIndex      = pDrawContext->ImageNumber - 1;
	
	if(pDrawContext->RayDistance)
	{
		ImageHeightSize = (UINT)((((float)pRayLevel->CellResolution)/(pDrawContext->RayDistance))*pRayLevel->SizeDistanceRatio); 
	}

	if(ImageHeightSize > pRayLevel->ScreenHeight)
	{
		ImageHeightSize = pRayLevel->ScreenHeight;
	}

	if(pDrawContext->MapIndexX != pDrawContext->MapIntersectionIndexX)
	{
		ImageLocationX = pDrawContext->CellIntersectionY;
	}
	else
	{
		ImageLocationX = pDrawContext->CellIntersectionX;
	}

	if(ImageLocationX >= pRayLevel->pWallGraphicList[ImageIndex].Width)
	{
		ImageLocationX = pRayLevel->pWallGraphicList[ImageIndex].Width - 1;
	}

	ScreenLocationY    = (pRayLevel->ScreenHeight/2) - (ImageHeightSize/2);
	ScreenLocationYEnd = ScreenLocationY + ImageHeightSize;

    ImageLocationY  = 0;
	ImageIncrementY = ((float)pRayLevel->pWallGraphicList[ImageIndex].Height/(float)ImageHeightSize);

	while(ScreenLocationY < ScreenLocationYEnd && ScreenLocationY < pRayLevel->ScreenHeight)
	{
		
		if(ImageLocationY > pRayLevel->pWallGraphicList[ImageIndex].Height)
		{
			ImageLocationY = (float)(pRayLevel->pWallGraphicList[ImageIndex].Height - 1);
		}

		DrawColor =  pRayLevel->pWallGraphicList[ImageIndex].pImageData[ImageLocationX + ((UINT)ImageLocationY*pRayLevel->pWallGraphicList[ImageIndex].Width)];

		if(pRayLevel->LightingType != NoLighting && pDrawContext->RayDistance > pRayLevel->SimpleLightingDistance)
		{
			UCHAR ColorRed, ColorGreen, ColorBlue;
			double Multiplier;

			Multiplier = pRayLevel->SimpleLightingLumination/pDrawContext->RayDistance;

			ColorRed    = (UCHAR)(((DrawColor>>16) & 0xFF)*Multiplier);
			ColorGreen  = (UCHAR)(((DrawColor>>8) & 0xFF)*Multiplier);
			ColorBlue   = (UCHAR)(((DrawColor) & 0xFF)*Multiplier);

			DrawColor = (ColorRed<<16) | (ColorGreen<<8) | (ColorBlue);
		}

		pRayLevel->pScreenBuffer[pDrawContext->VerticleLine + ScreenLocationY*pRayLevel->ScreenWidth] = DrawColor;

   	    ImageLocationY += ImageIncrementY;
  	    ScreenLocationY++;
	}
}




/***********************************************************************
 * RayLevel_Debug
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
 void RayLevel_Debug(char *pszFormatString, ...)
 {
     char DebugString[256];
     va_list vl;

     va_start(vl, pszFormatString);
     vsprintf(DebugString, pszFormatString, vl);
     va_end(vl);

     OutputDebugStringA(DebugString);
 }