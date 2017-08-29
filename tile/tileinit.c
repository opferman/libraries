/*******************************************************
 *
 *  TileInit.c
 *
 *     Tile Engine
 *
 *  Copyright (c) 2003,2007 All Rights Reserved
 *
 *******************************************************/


#include <windows.h>
#include <tile.h>
#include <stdarg.h>
#include <stdio.h>
#include "tileinternal.h"


 /********************************************************
  *  Tile_Init
  *
  *     Initialize The Tile Engine
  *   
  *
  *
  ********************************************************/
HTILE WINAPI Tile_Init(PTILE_MAP pTileMap)
{
	PTILE_ENGINE pTileEng = NULL;

	if(pTileEng = (PTILE_ENGINE)LocalAlloc(LMEM_ZEROINIT, sizeof(TILE_ENGINE)))
	{
		pTileEng->dwResolutionX = pTileMap->dwResolutionX;
		pTileEng->dwResolutionY = pTileMap->dwResolutionY;
		pTileEng->dwTileSizeX   = pTileMap->dwTileSizeX;
		pTileEng->dwTileSizeY   = pTileMap->dwTileSizeY;
		pTileEng->pfnDrawTile   = pTileMap->pfnDrawTile;
                pTileEng->pTileContext  = pTileMap->pTileContext;
		pTileEng->pfnDrawSprite = pTileMap->pfnDrawSprite;

		pTileEng->MainSpriteLocation.dwCurrentTileX  = pTileMap->dwCurrentTileX;
		pTileEng->MainSpriteLocation.dwCurrentTileY  = pTileMap->dwCurrentTileY;

		pTileEng->MainSpriteLocation.dwCurrentViewWidth  = pTileMap->dwViewWidth;
		pTileEng->MainSpriteLocation.dwCurrentViewHeight = pTileMap->dwViewHeight;

		Tile_CreateTileBox(pTileEng, &pTileEng->MainSpriteLocation, &pTileEng->MainTileBox);

		if(pTileEng->pTileMap = (PTILEINFO)LocalAlloc(LMEM_ZEROINIT, sizeof(TILEINFO)*pTileEng->dwResolutionY*pTileEng->dwResolutionX))
		{
			memcpy(pTileEng->pTileMap, pTileMap->pTileInfo, sizeof(TILEINFO)*pTileEng->dwResolutionY*pTileEng->dwResolutionX);
		}
		else
		{
			Tile_UnInit((HTILE)pTileEng);
			pTileEng = NULL;
		}
	}

    return (HTILE)pTileEng;
}



 /********************************************************
  *  Tile_UnInit
  *
  *     Free the tile context
  *   
  *
  *
  ********************************************************/
void WINAPI Tile_UnInit(HTILE hTile)
{
	PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;

	if(pTileEng)
	{
		while(pTileEng->pSprites)
		{
			Tile_DestroyTileSprite((HTILESPRITE)pTileEng->pSprites);
		}

		if(pTileEng->pTileMap)
		{
			LocalFree(pTileEng->pTileMap);
		}

		LocalFree(pTileEng);
	}

}


/***********************************************************************
 * Tile_CreateTileSprite
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
HTILESPRITE Tile_CreateTileSprite(HTILE hTile, PTILE_SPRITE pTileSprite)
{
	PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;
	PTILE_SPRITE_ENG pTileSpriteEng = NULL;

	if(pTileSpriteEng = (PTILE_SPRITE_ENG)LocalAlloc(LMEM_ZEROINIT, sizeof(TILE_SPRITE_ENG)))
	{
		pTileSpriteEng->pNextSprite = pTileEng->pSprites;
		
		if(pTileEng->pSprites)
		{
			pTileEng->pSprites->pPrevSprite = pTileSpriteEng;
		}

		pTileEng->pSprites = pTileSpriteEng;
	

		pTileSpriteEng->dwSpriteId    = pTileSprite->dwSpriteId;
		pTileSpriteEng->pTileContext  = pTileSprite->pTileContext;
		pTileSpriteEng->pfnDrawSprite = pTileSprite->pfnDrawSprite;
		
		pTileSpriteEng->pfnSpriteCollision = pTileSprite->pfnSpriteCollision;
		
		pTileSpriteEng->SpriteLocation.dwCurrentTileX = pTileSprite->dwCurrentTileX;
		pTileSpriteEng->SpriteLocation.dwCurrentTileY = pTileSprite->dwCurrentTileY;

		pTileSpriteEng->SpriteLocation.dwCurrentViewWidth  = pTileSprite->dwViewWidth;
		pTileSpriteEng->SpriteLocation.dwCurrentViewHeight = pTileSprite->dwViewHeight;
		pTileSpriteEng->dwFlags      = pTileSprite->dwFlags;
		pTileSpriteEng->pTileEng     = pTileEng;

		Tile_CreateTileBox(pTileEng, &pTileSpriteEng->SpriteLocation, &pTileSpriteEng->TileBox);
	}

    return (HTILESPRITE)pTileSpriteEng;
}


/***********************************************************************
 * Tile_DestroyTileSprite
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
void Tile_DestroyTileSprite(HTILESPRITE hTileSprite)
{
	PTILE_SPRITE_ENG pTileSpriteEng = (PTILE_SPRITE_ENG)hTileSprite;

	if(pTileSpriteEng->pNextSprite)
	{
		pTileSpriteEng->pNextSprite->pPrevSprite = pTileSpriteEng->pPrevSprite;
	}

	if(pTileSpriteEng->pPrevSprite)
	{
		pTileSpriteEng->pPrevSprite->pNextSprite = pTileSpriteEng->pNextSprite;
	}

	if(pTileSpriteEng->pTileEng->pSprites == pTileSpriteEng)
	{
		pTileSpriteEng->pTileEng->pSprites = pTileSpriteEng->pNextSprite;
	}

	LocalFree(pTileSpriteEng);
}




/***********************************************************************
 * Tile_Debug
 *  
 *    Debug 
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
 void Tile_Debug(char *pszFormatString, ...)
 {
     char DebugString[256];
     va_list vl;

     va_start(vl, pszFormatString);
     vsprintf(DebugString, pszFormatString, vl);
     va_end(vl);

     OutputDebugStringA(DebugString);
 }