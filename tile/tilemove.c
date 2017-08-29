/*******************************************************
 *
 *  TileMove.c
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
  *  Tile_MoveUp
  *
  *     Move the current position up.
  *   
  *
  *
  ********************************************************/
BOOL WINAPI Tile_MoveUp(HTILE hTile, DWORD dwPosition)
{
	PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;
	BOOL SpriteCanMove;

	SpriteCanMove = Tile_GenericMoveUp(pTileEng, &pTileEng->MainSpriteLocation, &pTileEng->MainTileBox, dwPosition);

	Tile_CollisionDetectionMainSprite(pTileEng);

	return SpriteCanMove;
}



/********************************************************
  *  Tile_MoveDown
  *
  *     Move the current position down.
  *   
  *
  *
  ********************************************************/
BOOL WINAPI Tile_MoveDown(HTILE hTile, DWORD dwPosition)
{
	PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;
	BOOL SpriteCanMove;

	SpriteCanMove = Tile_GenericMoveDown(pTileEng, &pTileEng->MainSpriteLocation, &pTileEng->MainTileBox, dwPosition);

	Tile_CollisionDetectionMainSprite(pTileEng);

	return SpriteCanMove;
}



 /********************************************************
  *  Tile_MoveLeft
  *
  *     Move the current position left.
  *   
  *
  *
  ********************************************************/
BOOL WINAPI Tile_MoveLeft(HTILE hTile, DWORD dwPosition)
{
	PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;
	BOOL SpriteCanMove;

	SpriteCanMove = Tile_GenericMoveLeft(pTileEng, &pTileEng->MainSpriteLocation, &pTileEng->MainTileBox, dwPosition);

	Tile_CollisionDetectionMainSprite(pTileEng);

	return SpriteCanMove;

}

 /********************************************************
  *  Tile_MoveRight
  *
  *     Move the current position right.
  *   
  *
  *
  ********************************************************/
BOOL WINAPI Tile_MoveRight(HTILE hTile, DWORD dwPosition)
{
	PTILE_ENGINE pTileEng = (PTILE_ENGINE)hTile;
	BOOL SpriteCanMove;

	SpriteCanMove = Tile_GenericMoveRight(pTileEng, &pTileEng->MainSpriteLocation, &pTileEng->MainTileBox, dwPosition);

	Tile_CollisionDetectionMainSprite(pTileEng);

	return SpriteCanMove;
}




/***********************************************************************
 * TileSprite_MoveUp
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
BOOL WINAPI TileSprite_MoveUp(HTILESPRITE hTileSprite, DWORD dwPosition)
{
	PTILE_SPRITE_ENG pTileSpriteEng = (PTILE_SPRITE_ENG)hTileSprite;
	PTILE_ENGINE pTileEng =  pTileSpriteEng->pTileEng;
	BOOL SpriteCanMove;

	SpriteCanMove = Tile_GenericMoveUp(pTileEng, &pTileSpriteEng->SpriteLocation, &pTileSpriteEng->TileBox, dwPosition);

	Tile_CollisionDetectionMainSpriteSpecifySprite(pTileEng, pTileSpriteEng);

	return SpriteCanMove;
}


/***********************************************************************
 * TileSprite_MoveDown
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
BOOL WINAPI TileSprite_MoveDown(HTILESPRITE hTileSprite, DWORD dwPosition)
{
	PTILE_SPRITE_ENG pTileSpriteEng = (PTILE_SPRITE_ENG)hTileSprite;
	PTILE_ENGINE pTileEng =  pTileSpriteEng->pTileEng;
	BOOL SpriteCanMove;

	SpriteCanMove = Tile_GenericMoveDown(pTileEng, &pTileSpriteEng->SpriteLocation, &pTileSpriteEng->TileBox, dwPosition);

	Tile_CollisionDetectionMainSpriteSpecifySprite(pTileEng, pTileSpriteEng);

	return SpriteCanMove;
}

/***********************************************************************
 * TileSprite_MoveLeft
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
BOOL WINAPI TileSprite_MoveLeft(HTILESPRITE hTileSprite, DWORD dwPosition)
{
	PTILE_SPRITE_ENG pTileSpriteEng = (PTILE_SPRITE_ENG)hTileSprite;
	PTILE_ENGINE pTileEng = pTileSpriteEng->pTileEng;
	BOOL SpriteCanMove;

	SpriteCanMove = Tile_GenericMoveLeft(pTileEng, &pTileSpriteEng->SpriteLocation, &pTileSpriteEng->TileBox, dwPosition);

	Tile_CollisionDetectionMainSpriteSpecifySprite(pTileEng, pTileSpriteEng);

	return SpriteCanMove;
}


/***********************************************************************
 * TileSprite_MoveRight
 *  
 *    
 *    Find the current start position of what tiles to start drawing.
 *    
 *
 *
 ***********************************************************************/
BOOL WINAPI TileSprite_MoveRight(HTILESPRITE hTileSprite, DWORD dwPosition)
{
	PTILE_SPRITE_ENG pTileSpriteEng = (PTILE_SPRITE_ENG)hTileSprite;
	PTILE_ENGINE pTileEng = pTileSpriteEng->pTileEng;
	BOOL SpriteCanMove;

	SpriteCanMove = Tile_GenericMoveRight(pTileEng, &pTileSpriteEng->SpriteLocation, &pTileSpriteEng->TileBox, dwPosition);

	Tile_CollisionDetectionMainSpriteSpecifySprite(pTileEng, pTileSpriteEng);

	return SpriteCanMove;
}




/********************************************************
  *  Tile_GenericMoveUp
  *
  *     Move the current position up.
  *   
  *
  *
  ********************************************************/
BOOL WINAPI Tile_GenericMoveUp(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, DWORD dwPosition)
{
	BOOL MoveIsComplete    = FALSE;
	BOOL NextTileIsBlocked = FALSE;
   
	while(!MoveIsComplete)
	{
		NextTileIsBlocked = Tile_IsMoveUpBlocked(pTileEng, pSpriteLocation, pTileBox);

		if(dwPosition > pSpriteLocation->dwCurrentTileIndexY)
		{
			if(NextTileIsBlocked)
			{
				pSpriteLocation->dwCurrentTileIndexY = 0;
				MoveIsComplete = TRUE;
			}
			else
			{
				dwPosition  = dwPosition - pSpriteLocation->dwCurrentTileIndexY;
				pSpriteLocation->dwCurrentTileIndexY = (pTileEng->dwTileSizeY - 1);
				pSpriteLocation->dwCurrentTileY--;
			}
		}
		else
		{
			pSpriteLocation->dwCurrentTileIndexY -= dwPosition;
			dwPosition = 0;
			MoveIsComplete = TRUE;			
		}

		Tile_CreateTileBox(pTileEng, pSpriteLocation, pTileBox);
	}

	return !NextTileIsBlocked;
}



/********************************************************
  *  Tile_GenericMoveDown
  *
  *     Move the current position up.
  *   
  *
  *
  ********************************************************/
BOOL WINAPI Tile_GenericMoveDown(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, DWORD dwPosition)
{
	BOOL MoveIsComplete    = FALSE;
	BOOL NextTileIsBlocked = FALSE;
   
	while(!MoveIsComplete)
	{
		NextTileIsBlocked = Tile_IsMoveDownBlocked(pTileEng, pSpriteLocation, pTileBox);

		if((pTileBox->dwPosBottomY + dwPosition) >= pTileEng->dwTileSizeY && NextTileIsBlocked)
		{
			if(pTileBox->dwPosBottomY + 1 >= pTileEng->dwTileSizeY)
			{
				dwPosition = 0;
			}
			else
			{
				dwPosition = pTileEng->dwTileSizeY - pTileBox->dwPosBottomY - 1;
			}
		}

		if((pSpriteLocation->dwCurrentTileIndexY + dwPosition) >= pTileEng->dwTileSizeY)
		{
			dwPosition  = dwPosition - (pTileEng->dwTileSizeY - pSpriteLocation->dwCurrentTileIndexY);
			pSpriteLocation->dwCurrentTileIndexY = 0;
			pSpriteLocation->dwCurrentTileY++;
		}
		else
		{
			pSpriteLocation->dwCurrentTileIndexY += dwPosition;
			dwPosition = 0;
			MoveIsComplete = TRUE;			
		}

		Tile_CreateTileBox(pTileEng, pSpriteLocation, pTileBox);
	}

	return !NextTileIsBlocked;
}

/********************************************************
  *  Tile_GenericMoveLeft
  *
  *     Move the current position left.
  *   
  *
  *
  ********************************************************/
BOOL WINAPI Tile_GenericMoveLeft(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, DWORD dwPosition)
{
	BOOL MoveIsComplete    = FALSE;
	BOOL NextTileIsBlocked = FALSE;
   
	while(!MoveIsComplete)
	{
		NextTileIsBlocked = Tile_IsMoveLeftBlocked(pTileEng, pSpriteLocation, pTileBox);

		if(dwPosition > pSpriteLocation->dwCurrentTileIndexX)
		{
			if(NextTileIsBlocked)
			{
				pSpriteLocation->dwCurrentTileIndexX = 0;
				MoveIsComplete = TRUE;
			}
			else
			{
				dwPosition  = dwPosition - pSpriteLocation->dwCurrentTileIndexX;
				pSpriteLocation->dwCurrentTileIndexX = (pTileEng->dwTileSizeX - 1);
				pSpriteLocation->dwCurrentTileX--;
			}
		}
		else
		{
			pSpriteLocation->dwCurrentTileIndexX -= dwPosition;
			dwPosition = 0;
			MoveIsComplete = TRUE;			
		}

		Tile_CreateTileBox(pTileEng, pSpriteLocation, pTileBox);
	}

	return !NextTileIsBlocked;
}




/********************************************************
  *  Tile_GenericMoveRight
  *
  *     Move the current position right.
  *   
  *
  *
  ********************************************************/
BOOL WINAPI Tile_GenericMoveRight(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, DWORD dwPosition)
{
	BOOL MoveIsComplete    = FALSE;
	BOOL NextTileIsBlocked = FALSE;
   
	while(!MoveIsComplete)
	{
		NextTileIsBlocked = Tile_IsMoveRightBlocked(pTileEng, pSpriteLocation, pTileBox);

		if((pTileBox->dwPosRightX + dwPosition) >= pTileEng->dwTileSizeX && NextTileIsBlocked)
		{
			if(pTileBox->dwPosRightX + 1 >= pTileEng->dwTileSizeX)
			{
				dwPosition = 0;
			}
			else
			{
				dwPosition = pTileEng->dwTileSizeX - pTileBox->dwPosRightX - 1;
			}
		}

		if((pSpriteLocation->dwCurrentTileIndexX + dwPosition) >= pTileEng->dwTileSizeX)
		{
			dwPosition  = dwPosition - (pTileEng->dwTileSizeX - pSpriteLocation->dwCurrentTileIndexX);
			pSpriteLocation->dwCurrentTileIndexX = 0;
			pSpriteLocation->dwCurrentTileX++;
		}
		else
		{
			pSpriteLocation->dwCurrentTileIndexX += dwPosition;
			dwPosition = 0;
			MoveIsComplete = TRUE;			
		}

		Tile_CreateTileBox(pTileEng, pSpriteLocation, pTileBox);
	}

	return !NextTileIsBlocked;
}




/***********************************************************************
 * Tile_IsMoveDownBlocked
 *  
 *    
 *    Check if the tiles down are blocked.
 *    
 *
 *
 ***********************************************************************/
BOOL Tile_IsMoveDownBlocked(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox)
{
	BOOL  NextTileIsBlocked = TRUE;
	UINT  StartTile;

	if(pTileBox->dwBottomTile < (pTileEng->dwResolutionY - 1))
	{
		NextTileIsBlocked = FALSE;
		for(StartTile = pTileBox->dwLeftTile; StartTile <= pTileBox->dwRightTile && NextTileIsBlocked == FALSE; StartTile++)
		{
			if((pTileEng->pTileMap[StartTile + ((pTileBox->dwBottomTile + 1)*pTileEng->dwResolutionX)].dwTileFlags & TF_BLOCKED) != 0)
			{
				NextTileIsBlocked = TRUE;
			}
		}
	}

	return NextTileIsBlocked;
}



/***********************************************************************
 * Tile_IsMoveUpBlocked
 *  
 *    
 *    Check if the tiles down are blocked.
 *    
 *
 *
 ***********************************************************************/
BOOL Tile_IsMoveUpBlocked(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox)
{
	BOOL  NextTileIsBlocked = TRUE;
	UINT  StartTile;

	if(pTileBox->dwTopTile > 0)
	{
		NextTileIsBlocked = FALSE;
		for(StartTile = pTileBox->dwLeftTile; StartTile <= pTileBox->dwRightTile && NextTileIsBlocked == FALSE; StartTile++)
		{
			if((pTileEng->pTileMap[StartTile + ((pTileBox->dwTopTile - 1)*pTileEng->dwResolutionX)].dwTileFlags & TF_BLOCKED) != 0)
			{
				NextTileIsBlocked = TRUE;
			}
		}
	}

	return NextTileIsBlocked;
}



/***********************************************************************
 * Tile_IsMoveLeftBlocked
 *  
 *    
 *    Check if the tiles down are blocked.
 *    
 *
 *
 ***********************************************************************/
BOOL Tile_IsMoveLeftBlocked(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox)
{
	BOOL  NextTileIsBlocked = TRUE;
	UINT  StartTile;

	if(pTileBox->dwLeftTile > 0)
	{
		NextTileIsBlocked = FALSE;
		for(StartTile = pTileBox->dwTopTile; StartTile <= pTileBox->dwBottomTile && NextTileIsBlocked == FALSE; StartTile++)
		{
			if((pTileEng->pTileMap[(pTileBox->dwLeftTile-1) + (StartTile*pTileEng->dwResolutionX)].dwTileFlags & TF_BLOCKED) != 0)
			{
				NextTileIsBlocked = TRUE;
			}
		}
	}

	return NextTileIsBlocked;
}



/***********************************************************************
 * Tile_IsMoveRightBlocked
 *  
 *    
 *    Check if the tiles down are blocked.
 *    
 *
 *
 ***********************************************************************/
BOOL Tile_IsMoveRightBlocked(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox)
{
	BOOL  NextTileIsBlocked = TRUE;
	UINT  StartTile;

	if(pTileBox->dwRightTile < (pTileEng->dwResolutionX - 1))
	{
		NextTileIsBlocked = FALSE;
		for(StartTile = pTileBox->dwTopTile; StartTile <= pTileBox->dwBottomTile && NextTileIsBlocked == FALSE; StartTile++)
		{
			if((pTileEng->pTileMap[(pTileBox->dwRightTile+1) + (StartTile*pTileEng->dwResolutionX)].dwTileFlags & TF_BLOCKED) != 0)
			{
				NextTileIsBlocked = TRUE;
			}
		}
	}

	return NextTileIsBlocked;
}

