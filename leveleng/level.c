/*******************************************************
 *
 *  Level.c
 *
 *     Level Engine
 *
 *  Copyright (c) 2003, All Rights Reserved
 *
 *******************************************************/
 
 
 
#include <windows.h>
#include <gdi.h>
#include <tile.h>
#include <leveleng.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "levelinternal.h"

typedef struct _LEVEL_INTERNAL
{
    PVOID pLevelContext;
	HTILE hTile;
	PFNEFFECTSCALLBACK pfnEffectsCallback;
	PFNSPRITECOLCB     pfnSpriteCollision;

	TILE_INFO   TileInfo;
	SCREEN_INFO ScreenInfo;
	WORLD_MAP   WorldMap;
	UINT        NumberOfSprites;
	SPRITE_DATA SpriteData[200];
	HTILESPRITE hTileSprite[199];
	PUCHAR      pScreenBuffer;

} LEVEL_INTERNAL, *PLEVEL_INTERNAL;

/*
 * Prototypes
 */
HTILE Level_InitializeTileEngine(PLEVEL_INTERNAL pLevelInternal, PLEVEL_PARAMS pLevelParams);
void WINAPI Level_DrawTileUsingGdi(PVOID pContext, DWORD TileId, UINT TileX, UINT TileY, UINT StartX, UINT StartY);
void Level_DrawSpriteUsingGdi(PVOID pContext, DWORD SpriteId, UINT uiStartX, UINT uiStartY, UINT ScreenX, UINT ScreenY);
void Level_Debug(char *pszFormatString, ...);
void Level_SpriteCollision(HTILESPRITE hTileSprite, PVOID pContext, DWORD dwSpriteId);

 /***********************************************************************
  * Level_Create
  *  
  *    Create Level
  *
  * Parameters
  *
  *
  * Return Value
  *
  ***********************************************************************/
HLEVEL Level_Create(PLEVEL_PARAMS pLevelParams)
{
	PLEVEL_INTERNAL pLevelInternal = NULL;

	pLevelInternal = (PLEVEL_INTERNAL)LocalAlloc(LMEM_ZEROINIT, sizeof(LEVEL_INTERNAL));

	if(pLevelInternal)
	{
		pLevelInternal->pLevelContext = pLevelParams->pLevelContext;
		pLevelInternal->ScreenInfo    = pLevelParams->ScreenInfo;
		pLevelInternal->WorldMap      = pLevelParams->WorldMap;
		pLevelInternal->TileInfo      = pLevelParams->TileInfo;
		pLevelInternal->pfnEffectsCallback  = pLevelParams->pfnEffectsCallback;
		pLevelInternal->pfnSpriteCollision  = pLevelParams->pfnSpriteCollision;

		pLevelInternal->NumberOfSprites  = 1;
		pLevelInternal->SpriteData[0]    = pLevelParams->SpriteData;
		pLevelInternal->SpriteData[0].SpriteIsStanding = TRUE;

		pLevelInternal->hTile         = Level_InitializeTileEngine(pLevelInternal, pLevelParams);

		if(pLevelInternal->hTile == NULL)
		{
			Level_Close((HLEVEL)pLevelInternal);
			pLevelInternal = NULL;
		}
	}

	return pLevelInternal;
}


 /***********************************************************************
  * Level_Close
  *  
  *    Create Level
  *
  * Parameters
  *
  *
  * Return Value
  *
  ***********************************************************************/
void Level_Close(HLEVEL hLevel)
{
}



 /***********************************************************************
  * Level_UpdateScreen
  *  
  *    Create Level
  *
  * Parameters
  *
  *
  * Return Value
  *
  ***********************************************************************/
void Level_UpdateScreen(HLEVEL hLevel)
{
	PLEVEL_INTERNAL pLevelInternal = (HLEVEL)hLevel;
	TILE_SCREEN_POSITION TileScreenPos;

	pLevelInternal->pScreenBuffer = GDI_BeginPaint(pLevelInternal->ScreenInfo.hScreenGdi);
    
	Tile_Draw(pLevelInternal->hTile, pLevelInternal->ScreenInfo.Width, pLevelInternal->ScreenInfo.Height);
    Tile_GetCurrentScreenLocation(pLevelInternal->hTile, &TileScreenPos);
	
	if(pLevelInternal->pfnEffectsCallback)
	{
		pLevelInternal->pfnEffectsCallback((HLEVEL)pLevelInternal, pLevelInternal->pScreenBuffer, pLevelInternal->pLevelContext);
	}

    GDI_EndPaint(pLevelInternal->ScreenInfo.hScreenGdi);
	pLevelInternal->pScreenBuffer = NULL;
}




 /***********************************************************************
  * Level_InitializeTileEngine
  *  
  *    
  *
  * Parameters
  *
  *
  * Return Value
  *
  ***********************************************************************/
HTILE Level_InitializeTileEngine(PLEVEL_INTERNAL pLevelInternal, PLEVEL_PARAMS pLevelParams)
{  
   PTILEINFO pTileInfo;
   TILE_MAP TileMap = {0};
   DWORD Index;
   DWORD NumberOfTiles;
   HTILE hTile = NULL;
   UINT SpriteWidth;
   UINT SpriteHeight;

   /*
    * Tile engine requires structure for each tile
	*/
   NumberOfTiles = pLevelParams->WorldMap.Width*pLevelParams->WorldMap.Height;

   pTileInfo = (PTILEINFO)LocalAlloc(LMEM_ZEROINIT, sizeof(TILEINFO)*NumberOfTiles);

   if(pTileInfo)
   {
	   for(Index = 0; Index < NumberOfTiles; Index++)
	   {
		   pTileInfo[Index].dwTileId    = pLevelParams->WorldMap.pTileMap[Index] & 0xFFFFFF;
		   pTileInfo[Index].dwTileFlags = pLevelParams->WorldMap.pTileMap[Index]>>24;
	   }

	   GDI_GetSize(pLevelParams->SpriteData.hStanding, &TileMap.dwViewWidth, &TileMap.dwViewHeight);

	   for(Index = 0; Index < (pLevelParams->SpriteData.NumberOfAnimations-1); Index++)
	   {
		   GDI_GetSize(pLevelParams->SpriteData.phAnimations[Index], &SpriteWidth, &SpriteHeight);

		   if(SpriteHeight > TileMap.dwViewHeight)
		   {
			   TileMap.dwViewHeight = SpriteHeight;
		   }

		   if(SpriteWidth > TileMap.dwViewWidth)
		   {
			   TileMap.dwViewWidth = SpriteWidth;
		   }
	   }

   
	   TileMap.pTileContext  = (PVOID)pLevelInternal;
	   TileMap.dwResolutionX = pLevelParams->WorldMap.Width;
	   TileMap.dwResolutionY = pLevelParams->WorldMap.Height;
	   TileMap.dwTileSizeX   = pLevelParams->TileInfo.Width;
	   TileMap.dwTileSizeY   = pLevelParams->TileInfo.Height;
	   TileMap.pTileInfo     = pTileInfo;
	   TileMap.pfnDrawTile   = Level_DrawTileUsingGdi;
	   TileMap.pfnDrawSprite = Level_DrawSpriteUsingGdi;

       TileMap.dwCurrentTileX = pLevelParams->TileInfo.StartTileX;
       TileMap.dwCurrentTileY = pLevelParams->TileInfo.StartTileY;

	   hTile = Tile_Init(&TileMap);
   }

   return hTile; 
}



 /***********************************************************************
  * Level_DrawTileUsingGdi
  *  
  *   Draw a Tile
  *
  * Parameters
  *     instance Handle, Window Handle
  * 
  * Return Value
  *     Exit Value
  *
  ***********************************************************************/
void WINAPI Level_DrawTileUsingGdi(PVOID pContext, DWORD TileId, UINT TileX, UINT TileY, UINT StartX, UINT StartY)
{
	PLEVEL_INTERNAL pLevelInternal = (PLEVEL_INTERNAL)pContext;
	PTILE_DESCRIPTION pTileDescription;
	DWORD StartIndexX;
	DWORD StartIndexY;
	DWORD CopySize;

	/*
	 * Future: Do a look up table here for tiles not in a sorted ordering
	 */
	pTileDescription = &pLevelInternal->TileInfo.pTileDescription[TileId];

	StartIndexX = pTileDescription->TileStartOffsetX;
	StartIndexY = pTileDescription->TileStartOffsetY;
    
	/*
	 * Assume everything is in range, for now
	 */
	{
		for(; StartY < pLevelInternal->TileInfo.Height && TileY < pLevelInternal->ScreenInfo.Height; StartY++, TileY++)
		{
			
			CopySize = pLevelInternal->TileInfo.Width - StartX;

			if(CopySize + TileX >= pLevelInternal->ScreenInfo.Width)
			{
				CopySize = CopySize - ((CopySize + TileX) - pLevelInternal->ScreenInfo.Width);
			}

			memcpy(&pLevelInternal->pScreenBuffer[(TileX<<2) + ((TileY<<2)*pLevelInternal->ScreenInfo.Width)], 
				   &pTileDescription->pTileBuffer[((StartIndexX+StartX)<<2) + (((StartIndexY+StartY)<<2)*pTileDescription->TileStride)],
				   CopySize<<2);
		}
	}
	
}


 /***********************************************************************
  * Level_DrawSpriteUsingGdi
  *  
  *   Draw a sprite
  *
  * Parameters
  *     instance Handle, Window Handle
  * 
  * Return Value
  *     Exit Value
  *
  ***********************************************************************/
void Level_DrawSpriteUsingGdi(PVOID pContext, DWORD SpriteId, UINT uiStartX, UINT uiStartY, UINT ScreenX, UINT ScreenY)
{
	PLEVEL_INTERNAL pLevelInternal = (PLEVEL_INTERNAL)pContext;
	DWORD *pSpriteImage;
	DWORD *pScreen;
	UINT IndexX, IndexY;
	UINT SpriteWidth, SpriteHeight;
	DWORD SkipColor;
	HGDI hImageGDI;
	UINT CalculatedLocationX;
	UINT CalculatedLocationY;

	if(pLevelInternal->SpriteData[SpriteId].SpriteIsStanding)
	{
		hImageGDI = pLevelInternal->SpriteData[SpriteId].hStanding;
	}
	else
	{
		hImageGDI = pLevelInternal->SpriteData[SpriteId].phAnimations[ pLevelInternal->SpriteData[SpriteId].CurrentSprite];
	}

	GDI_GetSize(hImageGDI, &SpriteWidth, &SpriteHeight);

	pScreen = (DWORD *)pLevelInternal->pScreenBuffer;
	pSpriteImage = (DWORD *)GDI_BeginPaint(hImageGDI);

	SkipColor = *pSpriteImage;

	for(IndexY = uiStartY; IndexY < SpriteHeight; IndexY++)
	{
		for(IndexX = uiStartX; IndexX < SpriteWidth; IndexX++)
		{
			if(SkipColor != pSpriteImage[IndexX + IndexY*SpriteWidth])
			{
				CalculatedLocationX = ScreenX + (IndexX - uiStartX);
				CalculatedLocationY = ScreenY + (IndexY - uiStartY);

				if(CalculatedLocationX < pLevelInternal->ScreenInfo.Width && CalculatedLocationY < pLevelInternal->ScreenInfo.Height)
				{
					pScreen[(CalculatedLocationX) + (CalculatedLocationY*pLevelInternal->ScreenInfo.Width)] = pSpriteImage[IndexX + IndexY*SpriteWidth];
				}
			}
		}
	}

	GDI_EndPaint(hImageGDI);

}


 /***********************************************************************
  * Level_AddSprite
  *  
  *   Add a Sprite
  *
  * Parameters
  *     instance Handle, Window Handle
  * 
  * Return Value
  *     Exit Value
  *
  ***********************************************************************/
void Level_AddSprite(HLEVEL hLevel, DWORD dwSpriteInstance, PSPRITE_DATA pSpriteData, DWORD StartTileX, DWORD StartTileY)
{
	PLEVEL_INTERNAL pLevelInternal = (HLEVEL)hLevel;
	TILE_SPRITE TileSprite = {0};
	DWORD Index;
	DWORD SpriteHeight, SpriteWidth;

	pLevelInternal->NumberOfSprites++;
	pLevelInternal->SpriteData[dwSpriteInstance]    = *pSpriteData;
	pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding = TRUE;

    TileSprite.pTileContext   = pLevelInternal;
	TileSprite.pfnDrawSprite  = Level_DrawSpriteUsingGdi;
	TileSprite.dwSpriteId     = dwSpriteInstance;

    GDI_GetSize(pSpriteData->hStanding, &TileSprite.dwViewWidth, &TileSprite.dwViewHeight);

	for(Index = 0; Index < (pSpriteData->NumberOfAnimations-1); Index++)
	{
	   GDI_GetSize(pSpriteData->phAnimations[Index], &SpriteWidth, &SpriteHeight);

	   if(SpriteHeight > TileSprite.dwViewHeight)
	   {
		   TileSprite.dwViewHeight = SpriteHeight;
	   }

	   if(SpriteWidth > TileSprite.dwViewWidth)
	   {
		   TileSprite.dwViewWidth = SpriteWidth;
	   }
	}
	
	if(pSpriteData->Flags & FLAG_SPRITE_IGNORE_BLOCKS)
	{
       TileSprite.dwFlags |= TS_FREE;
	}
	  
	TileSprite.dwCurrentTileX = StartTileX;
	TileSprite.dwCurrentTileY = StartTileY;
	TileSprite.pfnSpriteCollision = Level_SpriteCollision;

	pLevelInternal->hTileSprite[dwSpriteInstance - 1] = Tile_CreateTileSprite(pLevelInternal->hTile, &TileSprite);
}



 /***********************************************************************
  * Level_SpriteCollision
  *  
  *   Delete a Sprite
  *
  * Parameters
  *     instance Handle, Window Handle
  * 
  * Return Value
  *     Exit Value
  *
  ***********************************************************************/
void Level_SpriteCollision(HTILESPRITE hTileSprite, PVOID pContext, DWORD dwSpriteId)
{
	PLEVEL_INTERNAL pLevelInternal = (PLEVEL_INTERNAL)pContext;

	pLevelInternal->pfnSpriteCollision((HLEVEL)pLevelInternal, pLevelInternal->pLevelContext, dwSpriteId);
}

 /***********************************************************************
  * Level_DeleteSprite
  *  
  *   Delete a Sprite
  *
  * Parameters
  *     instance Handle, Window Handle
  * 
  * Return Value
  *     Exit Value
  *
  ***********************************************************************/
void Level_DeleteSprite(HLEVEL hLevel, DWORD dwSpriteInstance)
{
	PLEVEL_INTERNAL pLevelInternal = (HLEVEL)hLevel;

	pLevelInternal->NumberOfSprites--;
	memset(&pLevelInternal->SpriteData[dwSpriteInstance], 0, sizeof(SPRITE_DATA));

	Tile_DestroyTileSprite(pLevelInternal->hTileSprite[dwSpriteInstance - 1]);
	pLevelInternal->hTileSprite[dwSpriteInstance - 1] = NULL;
}


 /***********************************************************************
  * Level_MoveSprite
  *  
  *   Draw a Tile
  *
  * Parameters
  *     instance Handle, Window Handle
  * 
  * Return Value
  *     Exit Value
  *
  ***********************************************************************/
void WINAPI Level_GetCurrentTileLocation(HLEVEL hLevel, DWORD dwSpriteId, UINT *pTileX, UINT *pTileY)
{
    PLEVEL_INTERNAL pLevelInternal = (HLEVEL)hLevel;
	TILE_LOCATION TileLocation;

	if(dwSpriteId == 0)
	{
		Tile_GetCurrentTileLocation(pLevelInternal->hTile, &TileLocation);
		*pTileX = TileLocation.dwCurrentTileX;
	    *pTileY = TileLocation.dwCurrentTileY;
	}
	else
	{
		TileSprite_GetCurrentTileLocation(pLevelInternal->hTileSprite[dwSpriteId - 1], pTileX, pTileY);
	}
}

 /***********************************************************************
  * Level_MoveSprite
  *  
  *   Draw a Tile
  *
  * Parameters
  *     instance Handle, Window Handle
  * 
  * Return Value
  *     Exit Value
  *
  ***********************************************************************/
BOOL Level_MoveSprite(HLEVEL hLevel, DWORD dwSpriteInstance, int iDirectionX, int iDirectionY, BOOL bDirectionChange)
{
	PLEVEL_INTERNAL pLevelInternal = (HLEVEL)hLevel;
	BOOL MovementSatisfied = TRUE;

	if(pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding == FALSE)
	{
		pLevelInternal->SpriteData[dwSpriteInstance].CurrentSprite++;

		if(pLevelInternal->SpriteData[dwSpriteInstance].CurrentSprite >= pLevelInternal->SpriteData[dwSpriteInstance].CurrentUppferBounds)
		{
			pLevelInternal->SpriteData[dwSpriteInstance].CurrentSprite = pLevelInternal->SpriteData[dwSpriteInstance].CurrentLowerBounds;
		}
	}

	if(iDirectionX != 0)
	{
		if(iDirectionX < 0)
		{
			if(pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding || bDirectionChange)
			{
				pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding    = FALSE;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentSprite       = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberLeft;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentLowerBounds  = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberLeft;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentUppferBounds = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberRight;

			}
			
			if(dwSpriteInstance == 0)
			{
				MovementSatisfied = Tile_MoveLeft(pLevelInternal->hTile, (DWORD)(-1*iDirectionX));
			}
			else
			{
				MovementSatisfied = TileSprite_MoveLeft(pLevelInternal->hTileSprite[dwSpriteInstance - 1], (DWORD)(-1*iDirectionX));
			}
		}
		else
		{
			if(pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding || bDirectionChange)
			{
				pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding    = FALSE;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentSprite       = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberRight;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentLowerBounds  = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberRight;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentUppferBounds = pLevelInternal->SpriteData[dwSpriteInstance].NumberOfAnimations - 1;

			}

			if(dwSpriteInstance == 0)
			{
				MovementSatisfied = Tile_MoveRight(pLevelInternal->hTile, iDirectionX);
			}
			else
			{
				MovementSatisfied = TileSprite_MoveRight(pLevelInternal->hTileSprite[dwSpriteInstance - 1], iDirectionX);
			}
		}
	}

	if(iDirectionY != 0)
	{
		if(iDirectionY < 0)
		{
			if(pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding || bDirectionChange)
			{
				pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding    = FALSE;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentSprite       = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberUp;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentLowerBounds  = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberUp;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentUppferBounds = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberDown;

			}

			if(dwSpriteInstance == 0)
			{
				MovementSatisfied = Tile_MoveUp(pLevelInternal->hTile, (DWORD)(-1*iDirectionY));
			}
			else
			{
				MovementSatisfied = TileSprite_MoveUp(pLevelInternal->hTileSprite[dwSpriteInstance - 1], (DWORD)(-1*iDirectionY));
			}
		}
		else
		{
			if(pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding || bDirectionChange)
			{
				pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding    = FALSE;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentSprite       = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberDown;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentLowerBounds  = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberDown;
				pLevelInternal->SpriteData[dwSpriteInstance].CurrentUppferBounds = pLevelInternal->SpriteData[dwSpriteInstance].StartNumberLeft;

			}
			
			if(dwSpriteInstance == 0)
			{
				MovementSatisfied = Tile_MoveDown(pLevelInternal->hTile, iDirectionY);
			}
			else
			{
				MovementSatisfied = TileSprite_MoveDown(pLevelInternal->hTileSprite[dwSpriteInstance - 1], iDirectionY);
			}
		}
	}

	if(iDirectionY == 0 && iDirectionX == 0)
	{
		pLevelInternal->SpriteData[dwSpriteInstance].SpriteIsStanding = TRUE;
	}

	return MovementSatisfied;
}

/***********************************************************************
 * Level_Debug
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
 void Level_Debug(char *pszFormatString, ...)
 {
     char DebugString[256];
     va_list vl;

     va_start(vl, pszFormatString);
     vsprintf(DebugString, pszFormatString, vl);
     va_end(vl);

     OutputDebugStringA(DebugString);
 }