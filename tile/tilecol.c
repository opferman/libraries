/*******************************************************
 *
 *  TileCol.c
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



/***********************************************************************
 * Tile_CollisionDetectionMainSprite
 *  
 *    
 *    Determine if there are any collisons on the main sprite
 *    
 *
 *
 ***********************************************************************/
void Tile_CollisionDetectionMainSprite(PTILE_ENGINE pTileEng)
{
    PTILE_SPRITE_ENG pSpriteWalker, pTempSprite;

	pSpriteWalker = pTileEng->pSprites;

	while(pSpriteWalker)
	{
		if(pSpriteWalker->TileBox.dwLeftTile <= pTileEng->MainTileBox.dwRightTile &&
			pSpriteWalker->TileBox.dwRightTile >= pTileEng->MainTileBox.dwLeftTile)
		{
			if(pSpriteWalker->TileBox.dwTopTile <= pTileEng->MainTileBox.dwBottomTile &&
				pSpriteWalker->TileBox.dwBottomTile >= pTileEng->MainTileBox.dwTopTile)
			{
				/*
				 * Use Temp Sprite in case the collision code deletes the sprite
				 */
				pTempSprite = pSpriteWalker;
				pSpriteWalker = pSpriteWalker->pNextSprite;
				pTempSprite->pfnSpriteCollision((HTILESPRITE)pTempSprite, pTempSprite->pTileContext, pTempSprite->dwSpriteId);
			}
			else
			{
				pSpriteWalker = pSpriteWalker->pNextSprite;
			}
		}
		else
		{
			pSpriteWalker = pSpriteWalker->pNextSprite;
		}
	}
}	



/***********************************************************************
 * Tile_CollisionDetectionMainSprite
 *  
 *    
 *    Determine if there are any collisons on the main sprite
 *    
 *
 *
 ***********************************************************************/
void Tile_CollisionDetectionMainSpriteSpecifySprite(PTILE_ENGINE pTileEng, PTILE_SPRITE_ENG pSprite)
{
	if(pSprite->TileBox.dwLeftTile <= pTileEng->MainTileBox.dwRightTile &&
		pSprite->TileBox.dwRightTile >= pTileEng->MainTileBox.dwLeftTile)
	{
		if(pSprite->TileBox.dwTopTile <= pTileEng->MainTileBox.dwBottomTile &&
			pSprite->TileBox.dwBottomTile >= pTileEng->MainTileBox.dwTopTile)
		{
		    pSprite->pfnSpriteCollision((HTILESPRITE)pSprite, pSprite->pTileContext, pSprite->dwSpriteId);
		}
	}
}	