/*******************************************************
 *
 *  TileDraw.c
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
  *  Tile_Draw
  *
  *     Draw the tiles onto the screen
  *   
  *
  *
  ********************************************************/
void WINAPI Tile_Draw(HTILE hTile, UINT dwMaxX, UINT dwMaxY)
{
    PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;
    SCREEN_RESOLUTION ScreenResolution;
    SCREEN_START_INFO ScreenStartInfo;

    ScreenResolution.ScreenX = dwMaxX;
    ScreenResolution.ScreenY = dwMaxY;

    //
    // Draw Tiles on the Screen
    //
    Tile_InternalDrawTiles(pTileEng, &pTileEng->MainSpriteLocation, &pTileEng->MainTileBox, &ScreenResolution, &ScreenStartInfo);

    //
    // Draw The Main Sprite On Screen
    //
    pTileEng->pfnDrawSprite(pTileEng->pTileContext, 0, 0, 0, pTileEng->TileScreenPosition.ScreenX, pTileEng->TileScreenPosition.ScreenY);

    //
    // Draw the sprites on the screen
    //
    Tile_InternalDrawSprites(pTileEng, &ScreenResolution, &ScreenStartInfo);
}


/***********************************************************************
 * Tile_InternalDrawSprites
 *  
 *    
 *    Draw the tiles on the screen.
 *    
 *
 *
 ***********************************************************************/
void Tile_InternalDrawSprites(PTILE_ENGINE pTileEng, PSCREEN_RESOLUTION pScreenResolution, PSCREEN_START_INFO pScreenStartInfo)
{
    PTILE_SPRITE_ENG pTileSpriteEng;
    DWORD TopLeftTileX;
    DWORD TopLeftTileY;
    DWORD BottomRightTileX;
    DWORD BottomRightTileY;
    DWORD dwDrawStartX;
    DWORD dwDrawStartY;
    DWORD dwScreenX;
    DWORD dwScreenY;
    DWORD dwIndexX;
    DWORD dwIndexY;

    TopLeftTileX = pScreenStartInfo->StartTileX;
    TopLeftTileY = pScreenStartInfo->StartTileY;
    BottomRightTileX = TopLeftTileX + pScreenStartInfo->TilesX;
    BottomRightTileY = TopLeftTileY + pScreenStartInfo->TilesY;

    pTileSpriteEng = pTileEng->pSprites;

    while(pTileSpriteEng)
    {
        if(pTileSpriteEng->SpriteLocation.dwCurrentTileX >= TopLeftTileX &&
            pTileSpriteEng->SpriteLocation.dwCurrentTileY >= TopLeftTileY &&
            pTileSpriteEng->SpriteLocation.dwCurrentTileX <= BottomRightTileX &&
            pTileSpriteEng->SpriteLocation.dwCurrentTileY <= BottomRightTileY)
        {
            dwIndexX = pTileSpriteEng->SpriteLocation.dwCurrentTileX - TopLeftTileX;
            dwIndexY = pTileSpriteEng->SpriteLocation.dwCurrentTileY - TopLeftTileY;

            dwScreenX = dwIndexX ? ((pTileEng->dwTileSizeX - pScreenStartInfo->TileIndexX) + (dwIndexX - 1)*pTileEng->dwTileSizeX) : 0;
            dwScreenY = dwIndexY ? ((pTileEng->dwTileSizeY - pScreenStartInfo->TileIndexY) + (dwIndexY - 1)*pTileEng->dwTileSizeY) : 0;

            if(dwScreenX != 0)
            {
                dwScreenX += pTileSpriteEng->SpriteLocation.dwCurrentTileIndexX;
                dwDrawStartX = 0;
            }
            else
            {
                if(pScreenStartInfo->TileIndexX > pTileSpriteEng->SpriteLocation.dwCurrentTileIndexX)
                {
                    dwDrawStartX = pScreenStartInfo->TileIndexX - pTileSpriteEng->SpriteLocation.dwCurrentTileIndexX;
                }
                else
                {
                    dwScreenX    = pTileSpriteEng->SpriteLocation.dwCurrentTileIndexX - pScreenStartInfo->TileIndexX;
                    dwDrawStartX = 0;
                }
            }

            if(dwScreenY != 0)
            {
                dwScreenY += pTileSpriteEng->SpriteLocation.dwCurrentTileIndexY;
                dwDrawStartY = 0;
            }
            else
            {
                if(pScreenStartInfo->TileIndexY > pTileSpriteEng->SpriteLocation.dwCurrentTileIndexY)
                {
                    dwDrawStartY = pScreenStartInfo->TileIndexY - pTileSpriteEng->SpriteLocation.dwCurrentTileIndexY;
                }
                else
                {
                    dwScreenY    = pTileSpriteEng->SpriteLocation.dwCurrentTileIndexY - pScreenStartInfo->TileIndexY;
                    dwDrawStartY = 0;
                }
            }

            pTileSpriteEng->pfnDrawSprite(pTileSpriteEng->pTileContext, pTileSpriteEng->dwSpriteId, dwDrawStartX, dwDrawStartY, dwScreenX, dwScreenY);
        }

        pTileSpriteEng = pTileSpriteEng->pNextSprite;
    }
}

/***********************************************************************
 * Tile_InternalDrawTiles
 *  
 *    
 *    Draw the tiles on the screen.
 *    
 *
 *
 ***********************************************************************/
void Tile_InternalDrawTiles(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, PSCREEN_RESOLUTION pScreenResolution, PSCREEN_START_INFO pScreenStartInfo)
{
    DWORD dwIndexX;
    DWORD dwIndexY;
    DWORD dwScreenX;
    DWORD dwScreenY;
    PTILEINFO pTileMap;

    Tile_CenterScreenOnSprite(pTileEng, &pTileEng->MainSpriteLocation, &pTileEng->MainTileBox, pScreenResolution, pScreenStartInfo);
    
    for(dwIndexY = 0; dwIndexY < pScreenStartInfo->TilesY && dwIndexY < pTileEng->dwResolutionY; dwIndexY++)
    {
        for(dwIndexX = 0; dwIndexX < pScreenStartInfo->TilesX && dwIndexX < pTileEng->dwResolutionX; dwIndexX++)
        {
            pTileMap = &pTileEng->pTileMap[pScreenStartInfo->StartTileX + dwIndexX + (pScreenStartInfo->StartTileY + dwIndexY)*pTileEng->dwResolutionX];

            dwScreenX = dwIndexX ? ((pTileEng->dwTileSizeX - pScreenStartInfo->TileIndexX) + (dwIndexX - 1)*pTileEng->dwTileSizeX) : 0;
            dwScreenY = dwIndexY ? ((pTileEng->dwTileSizeY - pScreenStartInfo->TileIndexY) + (dwIndexY - 1)*pTileEng->dwTileSizeY) : 0;

            if(pSpriteLocation->dwCurrentTileY == (pScreenStartInfo->StartTileY + dwIndexY) && pSpriteLocation->dwCurrentTileX == (pScreenStartInfo->StartTileX + dwIndexX))
            {
                pTileEng->TileScreenPosition.ScreenX = dwScreenX + pSpriteLocation->dwCurrentTileIndexX;
                pTileEng->TileScreenPosition.ScreenY = dwScreenY + pSpriteLocation->dwCurrentTileIndexY;
            }
                        
            pTileEng->pfnDrawTile(pTileEng->pTileContext, pTileMap->dwTileId, dwScreenX, dwScreenY, (dwIndexX ? 0 : pScreenStartInfo->TileIndexX), (dwIndexY ? 0 : pScreenStartInfo->TileIndexY));
        }
    }

}


/***********************************************************************
 * Tile_CenterScreenOnSprite
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
void Tile_CenterScreenOnSprite(PTILE_ENGINE pTileEngine, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, PSCREEN_RESOLUTION pScreenResolution, PSCREEN_START_INFO pScreenStartInfo)
{
    SCREEN_START ScreenStartX;
    SCREEN_START ScreenStartY;

    ScreenStartX.TileSize         = pTileEngine->dwTileSizeX;
    ScreenStartX.TileResolution   = pTileEngine->dwResolutionX;
    ScreenStartX.ScreenResolution = pScreenResolution->ScreenX;
    ScreenStartX.SpriteIndex      = pSpriteLocation->dwCurrentTileIndexX;
    ScreenStartX.SpriteTile       = pSpriteLocation->dwCurrentTileX;

    Tile_FindTileStartPosition(&ScreenStartX);

    ScreenStartY.TileSize         = pTileEngine->dwTileSizeY;
    ScreenStartY.TileResolution   = pTileEngine->dwResolutionY;
    ScreenStartY.ScreenResolution = pScreenResolution->ScreenY;
    ScreenStartY.SpriteIndex      = pSpriteLocation->dwCurrentTileIndexY;
    ScreenStartY.SpriteTile       = pSpriteLocation->dwCurrentTileY;

    Tile_FindTileStartPosition(&ScreenStartY);

    pScreenStartInfo->TilesX = ScreenStartX.NumberOfTiles;
    pScreenStartInfo->TilesY = ScreenStartY.NumberOfTiles;

    pScreenStartInfo->StartTileX = ScreenStartX.StartTile;
    pScreenStartInfo->StartTileY = ScreenStartY.StartTile;

    pScreenStartInfo->TileIndexX = ScreenStartX.StartTileIndex;
    pScreenStartInfo->TileIndexY = ScreenStartY.StartTileIndex;
}



/***********************************************************************
 * Tile_FindTileStartPosition
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
void Tile_FindTileStartPosition(PSCREEN_START pScreenStart)
{
    DWORD TilesToDraw ;
    DWORD HalfResolution;
    DWORD TempScreenRes;
    int   CurrentTile;


    //
    // Determine the starting tile
    //
    HalfResolution = (pScreenStart->ScreenResolution>>1) - pScreenStart->SpriteIndex;
   
    CurrentTile = pScreenStart->SpriteTile;
    pScreenStart->StartTileIndex = 0;

    do {

        CurrentTile--;
        
        if(HalfResolution >= pScreenStart->TileSize)
        {
           HalfResolution -= pScreenStart->TileSize;
        }
        else
        {
           pScreenStart->StartTileIndex = pScreenStart->TileSize - HalfResolution;
           HalfResolution = 0;
        }
    
    } while(HalfResolution && CurrentTile >= 0);
    
    //
    // Determine if the tiles are in the valid tile range
    //
    if(CurrentTile >= 0)
    {
        pScreenStart->StartTile = CurrentTile;
    }
    else
    {
        pScreenStart->StartTile      = 0;
        CurrentTile                  = 0;
        pScreenStart->StartTileIndex = 0;
    }

    //
    // Determine the number of tiles to draw
    //

    TempScreenRes  = pScreenStart->ScreenResolution;
    TempScreenRes -= (pScreenStart->TileSize - pScreenStart->StartTileIndex);
    pScreenStart->NumberOfTiles = 1;

    while(TempScreenRes)
    {
        pScreenStart->NumberOfTiles++;

        if(TempScreenRes >= pScreenStart->TileSize)
        {
            TempScreenRes -= pScreenStart->TileSize;
        }
        else
        {
            TempScreenRes = 0;
        }
    }

    //
    // Determine if the tiles fit on screen
    //
    if(pScreenStart->TileResolution <  (pScreenStart->StartTile + pScreenStart->NumberOfTiles))
    {
        TempScreenRes  = pScreenStart->ScreenResolution;
        pScreenStart->NumberOfTiles = 0;
        pScreenStart->StartTileIndex = 0;

        while(TempScreenRes)
        {
            pScreenStart->NumberOfTiles++;

            if(TempScreenRes >= pScreenStart->TileSize)
            {
                TempScreenRes -= pScreenStart->TileSize;
            }
            else
            {
                pScreenStart->StartTileIndex =  pScreenStart->TileSize - TempScreenRes;
                TempScreenRes = 0;
            }
        }

        if(pScreenStart->NumberOfTiles > pScreenStart->TileResolution)
        {
            pScreenStart->StartTile = 0;
        }
        else
        {
            pScreenStart->StartTile = pScreenStart->TileResolution - pScreenStart->NumberOfTiles;
        }

    }
}

 