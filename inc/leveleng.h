/***********************************************************************
 * LevelEng.h
 *  
 *    Level Engine API
 *
 *
 * Toby Opferman Copyright (c) 2003
 *
 ***********************************************************************/


#ifndef __LEVELENG_H__
#define __LEVELENG_H__

#define INVALID_SPRITE_INDEX (0xFFFFFFFF)

typedef PVOID HLEVEL;

typedef void (*PFNEFFECTSCALLBACK)(HLEVEL hLevel, char *pScreenBuffer, PVOID pContext);
typedef void (*PFNSPRITECOLCB)(HLEVEL hLevel, PVOID pContext, DWORD dwSpriteId);

typedef enum _MOVEMENT_DIRECTION
{
	MoveDown,
	MoveUp,
	MoveLeft,
	MoveRight
} MOVEMENT_DIRECTION, *PMOVEMENT_DIRECTION;

#define FLAG_SPRITE_IGNORE_BLOCKS  0x1

typedef struct _SPRITE_DATA
{
	HGDI hStanding;
	HGDI *phAnimations;

	UINT StartNumberUp;
	UINT StartNumberDown;
	UINT StartNumberLeft;
	UINT StartNumberRight;
	UINT NumberOfAnimations;

	BOOL SpriteIsStanding;
	UINT CurrentSprite;
	UINT CurrentLowerBounds;
	UINT CurrentUppferBounds;

	UINT AnimationPauseCount;
	UINT AnimationPauses;
	MOVEMENT_DIRECTION MovementDirection;
	DWORD Flags;

} SPRITE_DATA, *PSPRITE_DATA;

typedef struct _TILE_DESCRIPTION
{
	DWORD  TileIndex;
	HGDI   hTileGdi;
	PUCHAR pTileBuffer;

	DWORD TileStartOffsetX;
	DWORD TileStartOffsetY;
	DWORD TileStride;
	
} TILE_DESCRIPTION, *PTILE_DESCRIPTION;

typedef struct _TILE_INFO
{
	DWORD Width;
	DWORD Height;
	DWORD NumberOfTiles;
	DWORD StartTileX;
	DWORD StartTileY;

	PTILE_DESCRIPTION pTileDescription;
	
} TILE_INFO, *PTILE_INFO;

typedef struct _SCREEN_INFO
{
	DWORD Width;
	DWORD Height;
	HGDI  hScreenGdi;
	
} SCREEN_INFO, *PSCREEN_INFO;

typedef struct _WORLD_MAP
{
	DWORD Width;
	DWORD Height;
	DWORD *pTileMap;
	
} WORLD_MAP, *PWORLD_MAP;

typedef struct _LEVEL_PARAMS
{
    PVOID pLevelContext;
	PFNEFFECTSCALLBACK pfnEffectsCallback;
	PFNSPRITECOLCB     pfnSpriteCollision;

	TILE_INFO   TileInfo;
	SCREEN_INFO ScreenInfo;
	WORLD_MAP   WorldMap;
	SPRITE_DATA SpriteData;

} LEVEL_PARAMS, *PLEVEL_PARAMS;

HLEVEL Level_Create(PLEVEL_PARAMS pLevelParams);
void Level_Close(HLEVEL hLevel);
void Level_UpdateScreen(HLEVEL hLevel);

DWORD Level_ShowSprite(HLEVEL hLevel, DWORD dwSpriteId, DWORD dwTileX, DWORD dwTileY);
void Level_HideSprite(HLEVEL hLevel, DWORD dwSpriteInstance);
BOOL Level_MoveSprite(HLEVEL hLevel, DWORD dwSpriteInstance, int iDirectionX, int iDirectionY, BOOL bDirectionChange);

void Level_AddSprite(HLEVEL hLevel, DWORD dwSpriteInstance, PSPRITE_DATA pSpriteData, DWORD StartTileX, DWORD StartTileY);
void Level_DeleteSprite(HLEVEL hLevel, DWORD dwSpriteInstance);

void WINAPI Level_GetCurrentTileLocation(HLEVEL hLevel, DWORD dwSpriteId, UINT *pTileX, UINT *pTileY);

#endif

