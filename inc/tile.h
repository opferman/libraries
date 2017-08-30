/***********************************************************************
 * Tile.h
 *  
 *    Tile Scroller API
 *
 *
 * Toby Opferman Copyright (c) 2003
 *
 ***********************************************************************/


#ifndef __TILE_H__
#define __TILE_H__


typedef PVOID HTILE;
typedef PVOID HTILESPRITE;


typedef void (WINAPI *PFNDRAWTILE)(PVOID pContext, DWORD dwTileId, UINT dwTileX, UINT dwTileY, UINT dwStartX, UINT dwStartY);
typedef void (WINAPI *PFNSPRITECALLBACK)(PVOID pContext, DWORD SpriteId, UINT uiStartX, UINT uiStartY, UINT ScreenX, UINT ScreenY);
typedef void (WINAPI *PFNTILECALLBACK)(PVOID pContext, DWORD dwTileCallBackId);
typedef void (WINAPI *PFNSPRITECOLLISION)(HTILESPRITE hTileSprite, PVOID pContext, DWORD dwSpriteId);

#define TF_CALLBACK  0x1  /* Moving On This Tile Gets A Call Back */
#define TF_BLOCKED   0x2  /* This Tile is Blocked From Movement   */

#define TS_CALLBACK  0x1  /* Moving On This Sprite Gets A Call Back                                */
#define TS_BLOCKED   0x2  /* This Sprite is Blocked From Movement, Callback is called on collision */
#define TS_FREE      0x4  /* This sprite ignores all blocked tiles */

typedef struct _TILEINFO {
    
    DWORD dwTileId;

    DWORD dwTileFlags;

    DWORD dwTileCallBackId;
    PVOID pTileCallbackContext;
    PFNTILECALLBACK pfnTileCallback;

} TILEINFO, *PTILEINFO;

typedef struct _TILE_MAP {

    PVOID             pTileContext;
    PFNDRAWTILE       pfnDrawTile;
    PFNSPRITECALLBACK pfnDrawSprite;

    DWORD dwResolutionX;
    DWORD dwResolutionY;

    DWORD dwTileSizeX;
    DWORD dwTileSizeY;

    DWORD dwCurrentTileX;
    DWORD dwCurrentTileY;

    DWORD dwViewWidth;
    DWORD dwViewHeight;

    PTILEINFO pTileInfo;

} TILE_MAP, *PTILE_MAP;



typedef struct _TILE_SCREEN_POSITION {

    DWORD ScreenX;
    DWORD ScreenY;

} TILE_SCREEN_POSITION, *PTILE_SCREEN_POSITION;


typedef struct _TILE_SPRITE 
{
    DWORD dwSpriteId;
    PVOID             pTileContext;
    PFNSPRITECALLBACK pfnDrawSprite;
    PFNSPRITECOLLISION pfnSpriteCollision;

    DWORD dwCurrentTileX;
    DWORD dwCurrentTileY;

    DWORD dwViewWidth;
    DWORD dwViewHeight;

    DWORD dwFlags;

} TILE_SPRITE, *PTILE_SPRITE;

typedef struct _TILE_LOCATION
{
    DWORD dwCurrentTileIndexX;
    DWORD dwCurrentTileIndexY;

    DWORD dwCurrentTileX;
    DWORD dwCurrentTileY;

} TILE_LOCATION, *PTILE_LOCATION;


#ifdef __cplusplus
extern "C" {
#endif

HTILE WINAPI Tile_Init(PTILE_MAP pTileMap);
void WINAPI Tile_UnInit(HTILE hTile);

BOOL WINAPI Tile_MoveUp(HTILE hTile, DWORD dwPosition);
BOOL WINAPI Tile_MoveDown(HTILE hTile, DWORD dwPosition);
BOOL WINAPI Tile_MoveLeft(HTILE hTile, DWORD dwPosition);
BOOL WINAPI Tile_MoveRight(HTILE hTile, DWORD dwPosition);
void WINAPI Tile_Draw(HTILE hTile, UINT dwMaxX, UINT dwMaxY);
void WINAPI Tile_GetCurrentScreenLocation(HTILE hTile, PTILE_SCREEN_POSITION pTileScreenPosition);
void WINAPI Tile_SetCurrentTileLocation(HTILE hTile, PTILE_LOCATION pTileLocation);
void WINAPI Tile_Modify(HTILE hTile, UINT TileX, UINT TileY, PTILEINFO pNewTileInfo);
BOOL WINAPI Tile_MoveUp(HTILE hTile, DWORD dwPosition);
BOOL WINAPI Tile_MoveDown(HTILE hTile, DWORD dwPosition);
BOOL WINAPI Tile_MoveLeft(HTILE hTile, DWORD dwPosition);
BOOL WINAPI Tile_MoveRight(HTILE hTile, DWORD dwPosition);
void WINAPI Tile_GetCurrentTileLocation(HTILE hTile, PTILE_LOCATION pTileLocation);

HTILESPRITE Tile_CreateTileSprite(HTILE hTile, PTILE_SPRITE pTileSprite);
BOOL WINAPI TileSprite_MoveUp(HTILESPRITE hTileSprite, DWORD dwPosition);
BOOL WINAPI TileSprite_MoveDown(HTILESPRITE hTileSprite, DWORD dwPosition);
BOOL WINAPI TileSprite_MoveLeft(HTILESPRITE hTileSprite, DWORD dwPosition);
BOOL WINAPI TileSprite_MoveRight(HTILESPRITE hTileSprite, DWORD dwPosition);
void WINAPI TileSprite_GetCurrentTileLocation(HTILESPRITE hTileSprite, UINT *pTileX, UINT *pTileY);
void Tile_DestroyTileSprite(HTILESPRITE hTileSprite);




#ifdef __cplusplus
}
#endif



#endif

 