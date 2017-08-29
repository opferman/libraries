#ifndef __GIF_H__
#define __GIF_H__

typedef PVOID HGIF;

HGIF WINAPI Gif_Open(char *pszFileName);
void WINAPI Gif_Close(HGIF hGif);
UINT WINAPI Gif_NumberOfImages(HGIF hGif);
UINT WINAPI Gif_GetImageSize(HGIF hGif, UINT Index);
UINT WINAPI Gif_GetImageWidth(HGIF hGif, UINT Index);
UINT WINAPI Gif_GetImageHeight(HGIF hGif, UINT Index);
void WINAPI Gif_GetImage32bpp(HGIF hGif, UINT Index, UCHAR *pImageBuffer32bpp);

#endif

