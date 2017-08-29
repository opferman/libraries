

#ifndef __GIF_INTERNAL__
#define __GIF_INTERNAL__

#pragma pack(1)
typedef struct _GIF_HEADER {

	  UCHAR Signature[3]; /* 'GIF' */
	  UCHAR Version[3];   /* i.e. 87a, 89a, etc. */

  } GIF_HEADER, *PGIF_HEADER;

#pragma pack(1)
  typedef struct _SCREEN_DESCRIPTOR {

       USHORT ScreenWidth;
	   USHORT ScreenHeight;
	   union {
			   UCHAR  Byte;
			   struct {
				   UCHAR BitsPerPixel:3;
				   UCHAR ReservedBit:1;
				   UCHAR CrBits:3;
				   UCHAR GlobalMapDefined:1;
			   };
	   };

	   UCHAR   ScreenBackgroundColorIndex;
       UCHAR   Reserved;

  } SCREEN_DESCRIPTOR, *PSCREEN_DESCRIPTOR;

#pragma pack(1)
  typedef struct _GIFRGB {

	  UCHAR Red;
	  UCHAR Green;
	  UCHAR Blue;

  } GIFRGB, *PGIFRGB;

#pragma pack(1)
	typedef struct _GLOBAL_COLOR_MAP { /* Defined only if m=1*/

	  GIFRGB   GifRgbIndex[256];

  } GLOBAL_COLOR_MAP, *PGLOBAL_COLOR_MAP;

#pragma pack(1)
  typedef struct _IMAGE_DESCRIPTOR {
	  char ImageSeperator; /* , */
      USHORT ImageStartLeft;
	  USHORT ImageStartTop;
      USHORT ImageWidth;
	  USHORT ImageHeight;
  	   union {
			   UCHAR  Byte;
			   struct {
				   UCHAR BitsPerPixel:3;
				   UCHAR ReservedBit:3;
				   UCHAR ImageIsInterlaced:1;
				   UCHAR UseLocalMap:1;
			   };
	   };

  } IMAGE_DESCRIPTOR, *PIMAGE_DESCRIPTOR;

#pragma pack(1)
  typedef struct _LOCAL_COLOR_MAP {

	  GIFRGB   GifRgbIndex[256];

  } LOCAL_COLOR_MAP, *PLOCAL_COLOR_MAP;

  #pragma pack(1)
  typedef struct _PACKED_BLOCK
  {
	  UCHAR BlockByteCount;
	  UCHAR DataBytes[];
  } PACKED_BLOCK, *PPACKED_BLOCK;

#pragma pack(1)
  typedef struct _RASTER_DATA
  {
	  UCHAR CodeSize;
	  UINT  NumberOfBlocks;
	  PPACKED_BLOCK pPackBlocks[5000];

  } RASTER_DATA, *PRASTER_DATA;

#pragma pack(1)
  typedef struct _IMAGE_DATA {
      PIMAGE_DESCRIPTOR  pImageDescriptor;
      PLOCAL_COLOR_MAP   pLocalColorMap;
	  RASTER_DATA        RasterData;
  } IMAGE_DATA, *PIMAGE_DATA;

#pragma pack(1)
  typedef struct _STRING_TABLE
  {
	  UCHAR DecodeString[4096];
	  UINT  Length;
  } STRING_TABLE, *PSTRING_TABLE;

#pragma pack(1)
  typedef struct _DECODE_STRING_TABLE
  {
	  UINT  ClearCode;
	  UINT  EndOfInformation;
	  UINT  FirstAvailable;
	  UINT  CurrentIndex;
	  UINT  CurrentCodeBits;
	  UINT  Stride;	  
	  STRING_TABLE StringTable[4096];

	  UINT  LastCodeWord;
	  UINT  NewCodeWord;
	  UINT  BitIncrement;
	  UCHAR *pRasterDataBuffer;
	  UINT  RasterDataSize;

	  UINT  CurrentPixel;
	  UINT  ImageWidth;
	  DWORD *pImageBuffer32bpp;
	  PGIFRGB pImagePalette;

	  UINT  ImageX;
	  UINT  ImageY;
	  UINT  ImageStartLeft;
	  
  } DECODE_STRING_TABLE, *PDECODE_STRING_TABLE;

#endif

