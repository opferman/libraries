/*******************************************************
 *
 *  TileUtil.c
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
 * Tile_SetCurrentTileLocation
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
void WINAPI Tile_SetCurrentTileLocation(HTILE hTile, PTILE_LOCATION pTileLocation)
{
    PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;

    if(pTileLocation->dwCurrentTileX < pTileEng->dwResolutionX)
    {
        pTileEng->MainSpriteLocation.dwCurrentTileX = pTileLocation->dwCurrentTileX;
    }

    if(pTileLocation->dwCurrentTileY < pTileEng->dwResolutionY)
    {
        pTileEng->MainSpriteLocation.dwCurrentTileY = pTileLocation->dwCurrentTileY;
    }

    if(pTileLocation->dwCurrentTileIndexX < pTileEng->dwTileSizeX)
    {
        pTileEng->MainSpriteLocation.dwCurrentTileIndexX = pTileLocation->dwCurrentTileIndexX;
    }

    if(pTileLocation->dwCurrentTileIndexY < pTileEng->dwTileSizeY)
    {
        pTileEng->MainSpriteLocation.dwCurrentTileIndexY = pTileLocation->dwCurrentTileIndexY;
    }
}



/***********************************************************************
 * Tile_GetCurrentTileLocation
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
void WINAPI Tile_GetCurrentTileLocation(HTILE hTile, PTILE_LOCATION pTileLocation)
{
    PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;

    pTileLocation->dwCurrentTileX = pTileEng->MainSpriteLocation.dwCurrentTileX;
    pTileLocation->dwCurrentTileY = pTileEng->MainSpriteLocation.dwCurrentTileY;

    pTileLocation->dwCurrentTileIndexX = pTileEng->MainSpriteLocation.dwCurrentTileIndexX;
    pTileLocation->dwCurrentTileIndexY = pTileEng->MainSpriteLocation.dwCurrentTileIndexY;
}


/***********************************************************************
 * TileSprite_GetCurrentTileLocation
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
void WINAPI TileSprite_GetCurrentTileLocation(HTILESPRITE hTileSprite, UINT *pTileX, UINT *pTileY)
{
    PTILE_SPRITE_ENG pTileSpriteEng = (PTILE_SPRITE_ENG)hTileSprite;

    *pTileX = pTileSpriteEng->SpriteLocation.dwCurrentTileX;
    *pTileY = pTileSpriteEng->SpriteLocation.dwCurrentTileY;
}


/***********************************************************************
 * Tile_GetCurrentTileLocation
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
void WINAPI Tile_GetCurrentScreenLocation(HTILE hTile, PTILE_SCREEN_POSITION pTileScreenPosition)
{
    PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;

    *pTileScreenPosition = pTileEng->TileScreenPosition;
}


/***********************************************************************
 * Tile_Modify
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
void WINAPI Tile_Modify(HTILE hTile, UINT TileX, UINT TileY, PTILEINFO pNewTileInfo)
{
    PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;

    pTileEng->pTileMap[TileX + pTileEng->dwResolutionX*TileY] = *pNewTileInfo;
}


/***********************************************************************
 * Tile_CreateTileBox
 *  
 *    
 *    Determine the location boundary of the sprite.
 *    
 *
 *
 ***********************************************************************/
void Tile_CreateTileBox(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocationData, PTILE_BOX pTileBox)
{
    pTileBox->dwLeftTile = pSpriteLocationData->dwCurrentTileX;
    pTileBox->dwTopTile  = pSpriteLocationData->dwCurrentTileY;
    pTileBox->dwPosLeftX = pSpriteLocationData->dwCurrentTileIndexX;
    pTileBox->dwPosTopY  = pSpriteLocationData->dwCurrentTileIndexY;

    pTileBox->dwRightTile  = pTileBox->dwLeftTile;
    pTileBox->dwBottomTile = pTileBox->dwTopTile;
    pTileBox->dwPosRightX  = pTileBox->dwPosLeftX + pSpriteLocationData->dwCurrentViewWidth;
    pTileBox->dwPosBottomY = pTileBox->dwPosTopY + pSpriteLocationData->dwCurrentViewHeight;

    while(pTileBox->dwPosRightX > pTileEng->dwTileSizeX)
    {
        pTileBox->dwPosRightX = pTileBox->dwPosRightX - pTileEng->dwTileSizeX;
        pTileBox->dwRightTile++;
    }

    while(pTileBox->dwPosBottomY > pTileEng->dwTileSizeY)
    {
        pTileBox->dwPosBottomY = pTileBox->dwPosBottomY - pTileEng->dwTileSizeY;
        pTileBox->dwBottomTile++;
    } 
}








