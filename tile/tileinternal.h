


#ifndef __TILE_INTERNAL__
#define __TILE_INTERNAL__




typedef struct _TILE_SPRITE_ENG;


typedef struct _SCREEN_RESOLUTION
{
	DWORD ScreenX;
	DWORD ScreenY;

} SCREEN_RESOLUTION, *PSCREEN_RESOLUTION;

typedef struct _SCREEN_START
{
	DWORD TileResolution;
	DWORD ScreenResolution;
	DWORD SpriteIndex;
	DWORD SpriteTile;
	DWORD TileSize;

	DWORD NumberOfTiles;
	DWORD StartTile;
	DWORD StartTileIndex;

} SCREEN_START, *PSCREEN_START;

typedef struct _SCREEN_START_INFO
{
	DWORD TilesX;
	DWORD TilesY;

	DWORD StartTileX;
	DWORD StartTileY;

	DWORD TileIndexX;
	DWORD TileIndexY;

} SCREEN_START_INFO, *PSCREEN_START_INFO;

 typedef struct _TILE_BOX
{
	DWORD dwLeftTile;
	DWORD dwTopTile;
	DWORD dwRightTile;
	DWORD dwBottomTile;

	DWORD dwPosLeftX;
	DWORD dwPosTopY;
	DWORD dwPosRightX;
	DWORD dwPosBottomY;


} TILE_BOX, *PTILE_BOX;

 typedef struct _SPRITE_LOCATION_DATA
{
	DWORD dwCurrentTileIndexY;
	DWORD dwCurrentTileIndexX;
	
	DWORD dwCurrentTileX;
	DWORD dwCurrentTileY;

	DWORD dwCurrentViewWidth;
	DWORD dwCurrentViewHeight;

} SPRITE_LOCATION_DATA, *PSPRITE_LOCATION_DATA;


 typedef struct _TILE_ENGINE {

    PVOID             pTileContext;
    PFNDRAWTILE       pfnDrawTile;
	PFNSPRITECALLBACK pfnDrawSprite;

	DWORD dwResolutionX;
	DWORD dwResolutionY;

	DWORD dwTileSizeX;
	DWORD dwTileSizeY;

	SPRITE_LOCATION_DATA MainSpriteLocation;
	TILE_BOX             MainTileBox;

	TILE_SCREEN_POSITION TileScreenPosition;

    PTILEINFO pTileMap;

	struct _TILE_SPRITE_ENG *pSprites;

 } TILE_ENGINE, *PTILE_ENGINE;


 typedef struct _TILE_SPRITE_ENG
{
	PTILE_ENGINE       pTileEng;

	DWORD              dwSpriteId;
	DWORD              dwFlags;

    PVOID              pTileContext;
    PFNSPRITECALLBACK  pfnDrawSprite;
	PFNSPRITECOLLISION pfnSpriteCollision;
	
	SPRITE_LOCATION_DATA SpriteLocation;
	TILE_BOX             TileBox;
	
	struct _TILE_SPRITE_ENG *pNextSprite;
	struct _TILE_SPRITE_ENG *pPrevSprite;

} TILE_SPRITE_ENG, *PTILE_SPRITE_ENG;



void Tile_Debug(char *pszFormatString, ...);
void Tile_CreateTileBox(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocationData, PTILE_BOX pTileBox);
BOOL WINAPI Tile_GenericMoveUp(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, DWORD dwPosition);
BOOL WINAPI Tile_GenericMoveDown(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, DWORD dwPosition);
BOOL WINAPI Tile_GenericMoveLeft(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, DWORD dwPosition);
BOOL WINAPI Tile_GenericMoveRight(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, DWORD dwPosition);
BOOL Tile_IsMoveUpBlocked(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox);
BOOL Tile_IsMoveDownBlocked(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox);
BOOL Tile_IsMoveLeftBlocked(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox);
BOOL Tile_IsMoveRightBlocked(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox);
void Tile_CenterScreenOnSprite(PTILE_ENGINE pTileEngine, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, PSCREEN_RESOLUTION pScreenResolution, PSCREEN_START_INFO pScreenStartInfo);

void Tile_CollisionDetectionMainSprite(PTILE_ENGINE pTileEng);
void Tile_CollisionDetectionMainSpriteSpecifySprite(PTILE_ENGINE pTileEng, PTILE_SPRITE_ENG pSprite);
void Tile_CollisionDetectionSpritesSpecifySprite(PTILE_ENGINE pTileEng, PTILE_SPRITE_ENG pSprite);

void Tile_InternalDrawTiles(PTILE_ENGINE pTileEng, PSPRITE_LOCATION_DATA pSpriteLocation, PTILE_BOX pTileBox, PSCREEN_RESOLUTION pScreenResolution, PSCREEN_START_INFO pScreenStartInfo);
void Tile_InternalDrawSprites(PTILE_ENGINE pTileEng, PSCREEN_RESOLUTION pScreenResolution, PSCREEN_START_INFO pScreenStartInfo);
void Tile_FindTileStartPosition(PSCREEN_START pScreenStart);


#endif 


