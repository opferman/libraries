/*******************************************************
 *
 *  Gif.c
 *
 *     GIF Engine
 *
 *  Copyright (c) 2003, All Rights Reserved
 *
 *******************************************************/
 
 
 
#include <windows.h>
#include <gif.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include "gifinternal.h"
 

#define HANDLE_IS_VALID(x) (x != INVALID_HANDLE_VALUE && x != NULL)
#define GET_NEXT_POINTER(x, y, z) (z)((char *)x + y)

#define DEBUGPRINT  
#define DEBUGPRINT2 

#define CREATE_RGB(x) (x.Red<<16 | x.Green<<8 | x.Blue)
#define Gif_IsClearCode(x, y) (x->ClearCode == y)
#define Gif_IsEndOfImageCode(x, y) (x->EndOfInformation == y)
#define CREATE_BIT_MASK(x) ((1<<x)-1)
#define Gif_GetPaletteColorByIndexSpecifyPalette(x, y)  CREATE_RGB(x[y])
#define CODE_TO_INDEX(x,y) (x - y->FirstAvailable)
#define INDEX_TO_CODE(x,y) (x + y->FirstAvailable)

/*********************************************************
 * Internal Functions
 *********************************************************/
 typedef struct _GIF_INTERNAL {

     HANDLE hGifFile;
     HANDLE hMemoryMapping;
     PVOID  pStartOfGif;

     PGIF_HEADER        pGifHeader;
     PSCREEN_DESCRIPTOR pScreenDescriptor;
     PGLOBAL_COLOR_MAP  pGlobalColorMap;
     
     UINT               NumberOfImages;
     IMAGE_DATA         ImageData[256]; /* Support up to 256 Images, Dynamic support could be added later  */

 } GIF_INTERNAL, *PGIF_INTERNAL;



void WINAPI Gif_DisplayDebugInformation(PGIF_INTERNAL pGifInternal);
void WINAPI Gif_Debug(char *pszFormatString, ...);
BOOL WINAPI Gif_OpenAndValidateFile(PGIF_INTERNAL pGifInternal, char *pszFileName);
BOOL WINAPI Gif_ParseFile(PGIF_INTERNAL pGifInternal);
UINT WINAPI Gif_ParsePackedBlock(PGIF_INTERNAL pGifInternal, PPACKED_BLOCK *pPackedBlocks, UINT *pOffset);
void WINAPI Gif_CloseFile(PGIF_INTERNAL pGifInternal);
void WINAPI Gif_InitializeStringTable(PGIF_INTERNAL pGifInternal, PIMAGE_DATA pImageData, PDECODE_STRING_TABLE pDecodeStringTable);
DWORD WINAPI Gif_GetPaletteColorByIndex(PGIF_INTERNAL pGifInternal, UINT ImageIndex, UINT ColorIndex);
void WINAPI Gif_SetBackgroundColor(PGIF_INTERNAL pGifInternal, UINT ImageIndex, UCHAR *pImageBuffer32bpp);
void WINAPI Gif_Decode(PGIF_INTERNAL pGifInternal, PIMAGE_DATA pImageData, UINT Stride, UCHAR *pImageBuffer32bpp);
BOOL WINAPI Gif_DecodePackedBlock(PGIF_INTERNAL pGifInternal, PIMAGE_DATA pImageData, PDECODE_STRING_TABLE pDecodeStringTable);
UINT WINAPI Gif_RetrieveCodeWord(PDECODE_STRING_TABLE pDecodeStringTable, UINT *pBitIncrement, UCHAR *pPackedBlockBytes);
BOOL WINAPI Gif_ProcessNewCode(PDECODE_STRING_TABLE pDecodeStringTable, UINT LastCodeWord, UINT NewCodeWord);
BOOL WINAPI Gif_AddNewEntry(PDECODE_STRING_TABLE pDecodeStringTable, UINT LastCodeWord, UINT NewCodeWord);


 /********************************************************
  *  Gif_Open
  *
  *     Open The Gif
  *   
  *
  *
  ********************************************************/
HGIF WINAPI Gif_Open(char *pszFileName)
{
    PGIF_INTERNAL pGifInternal = NULL;
    BOOL bFileIsGif;
    BOOL bFileParseSuccessful;

    pGifInternal = (PGIF_INTERNAL)LocalAlloc(LMEM_ZEROINIT, sizeof(GIF_INTERNAL));

    if(pGifInternal)
    {
        bFileIsGif = Gif_OpenAndValidateFile(pGifInternal, pszFileName);

        if(bFileIsGif)
        {
            bFileParseSuccessful = Gif_ParseFile(pGifInternal);

            if(bFileParseSuccessful == FALSE)
            {
                Gif_CloseFile(pGifInternal);
                LocalFree(pGifInternal);
                pGifInternal = NULL;
            }
        }
        else
        {
            LocalFree(pGifInternal);
            pGifInternal = NULL;
        }		
    }

    return pGifInternal;
}


 /********************************************************
  *  Gif_Close
  *
  *     Close the GIF
  *   
  *
  *
  ********************************************************/
void WINAPI Gif_Close(HGIF hGif)
{
    PGIF_INTERNAL pGifInternal = (PGIF_INTERNAL)hGif;
    Gif_CloseFile(pGifInternal);
    LocalFree(pGifInternal);
}


 /********************************************************
  *  Gif_CloseFile
  *
  *     Close the GIF
  *   
  *
  *
  ********************************************************/
void WINAPI Gif_CloseFile(PGIF_INTERNAL pGifInternal)
{

    if(pGifInternal->pStartOfGif)
    {
        UnmapViewOfFile(pGifInternal->pStartOfGif);
        pGifInternal->pStartOfGif = NULL;
    }
    if(HANDLE_IS_VALID(pGifInternal->hMemoryMapping))
    {
        CloseHandle(pGifInternal->hMemoryMapping);
        pGifInternal->hMemoryMapping = NULL;
    }

    if(HANDLE_IS_VALID(pGifInternal->hGifFile))
    {
        CloseHandle(pGifInternal->hGifFile);
        pGifInternal->hGifFile = NULL;
    }
}


 /********************************************************
  *  Gif_OpenAndValidateFile
  *
  *     
  *   
  *
  *
  ********************************************************/
BOOL WINAPI Gif_OpenAndValidateFile(PGIF_INTERNAL pGifInternal, char *pszFileName)
{
    BOOL bFileIsValid = FALSE;

    pGifInternal->hGifFile = CreateFile(pszFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

    if(HANDLE_IS_VALID(pGifInternal->hGifFile))
    {
        pGifInternal->hMemoryMapping = CreateFileMapping(pGifInternal->hGifFile, NULL, PAGE_READONLY, 0, 0, NULL);

        if(HANDLE_IS_VALID(	pGifInternal->hMemoryMapping))
        {
            pGifInternal->pStartOfGif = MapViewOfFile(pGifInternal->hMemoryMapping, FILE_MAP_READ, 0, 0, 0);

            if(pGifInternal->pStartOfGif)
            {
                pGifInternal->pGifHeader = (PGIF_HEADER)pGifInternal->pStartOfGif;
                bFileIsValid = TRUE;
                if(pGifInternal->pGifHeader->Signature[0] != 'G' || 
                   pGifInternal->pGifHeader->Signature[1] != 'I' ||
                   pGifInternal->pGifHeader->Signature[2] != 'F')
                {
                    bFileIsValid = FALSE;
                    Gif_CloseFile(pGifInternal);
                }
            }
            else
            {
                Gif_CloseFile(pGifInternal);
            }
        }
        else
        {
            Gif_CloseFile(pGifInternal);
        }
    }

    return bFileIsValid;
}

/***********************************************************************
 * Gif_ParseFile
 *  
 *    Gif_ParseFile 
 *
 *    
 *
 * Parameters
 *     Gif Internal
 *
 * Return Value
 *     Nothing
 *
 ***********************************************************************/
BOOL WINAPI Gif_ParseFile(PGIF_INTERNAL pGifInternal)
{
    UINT CurrentOffset;
    UINT CurrentRasterBlock;
    BOOL bFileParseSuccessful = TRUE;
    BOOL bMoreImages;
    BOOL bMoreBlocks;

    CurrentOffset = sizeof(GIF_HEADER);
    pGifInternal->pScreenDescriptor = GET_NEXT_POINTER(pGifInternal->pStartOfGif, CurrentOffset, PSCREEN_DESCRIPTOR);
    CurrentOffset += sizeof(SCREEN_DESCRIPTOR);

    if(pGifInternal->pScreenDescriptor->GlobalMapDefined)
    {
        pGifInternal->pGlobalColorMap = GET_NEXT_POINTER(pGifInternal->pStartOfGif, CurrentOffset, PGLOBAL_COLOR_MAP);
        CurrentOffset += (UINT)(3*pow(2, ((int)pGifInternal->pScreenDescriptor->BitsPerPixel + 1)));
    }

    do {
        UINT CurrentIndex = pGifInternal->NumberOfImages;

        /*
         * Remove Extension Data
         */
        while(*((UCHAR *)pGifInternal->pStartOfGif + CurrentOffset) != ';' && *((UCHAR *)pGifInternal->pStartOfGif + CurrentOffset) != ',')
        {
            CurrentOffset += 2;
            Gif_ParsePackedBlock(pGifInternal, NULL, &CurrentOffset);
        }

        if(*((UCHAR *)pGifInternal->pStartOfGif + CurrentOffset) == ',')
        {
            pGifInternal->ImageData[CurrentIndex].pImageDescriptor = GET_NEXT_POINTER(pGifInternal->pStartOfGif, CurrentOffset, PIMAGE_DESCRIPTOR);
            CurrentOffset += sizeof(IMAGE_DESCRIPTOR);
           
            if(pGifInternal->ImageData[CurrentIndex].pImageDescriptor->UseLocalMap)
            {
                pGifInternal->ImageData[CurrentIndex].pLocalColorMap = GET_NEXT_POINTER(pGifInternal->pStartOfGif, CurrentOffset, PLOCAL_COLOR_MAP);
                CurrentOffset += (UINT)(3*pow(2, pGifInternal->ImageData[CurrentIndex].pImageDescriptor->BitsPerPixel + 1));
            }
                        
            CurrentRasterBlock = 0;
            bMoreBlocks = TRUE;

            pGifInternal->ImageData[CurrentIndex].RasterData.CodeSize = *((UCHAR *)pGifInternal->pStartOfGif + CurrentOffset);
            CurrentOffset++;			
            pGifInternal->ImageData[CurrentIndex].RasterData.NumberOfBlocks = Gif_ParsePackedBlock(pGifInternal, pGifInternal->ImageData[CurrentIndex].RasterData.pPackBlocks, &CurrentOffset);
        }

        if(*((UCHAR *)pGifInternal->pStartOfGif + CurrentOffset) == ';')
        {
            bMoreImages = FALSE;
        }

        pGifInternal->NumberOfImages++;
    } while(bMoreImages);   

#if 0
    Gif_DisplayDebugInformation(pGifInternal);
#endif

    return bFileParseSuccessful;
}


/***********************************************************************
 * Gif_ParsePackedBlock
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     Offset
 *
 ***********************************************************************/
UINT WINAPI Gif_ParsePackedBlock(PGIF_INTERNAL pGifInternal, PPACKED_BLOCK *pPackedBlocks, UINT *pOffset)
{
    BOOL bMoreBlocks = TRUE;
    PPACKED_BLOCK pPackedBlock;
    UINT CurrentRasterBlock = 0;


    do {

        pPackedBlock = GET_NEXT_POINTER(pGifInternal->pStartOfGif, (*pOffset), PPACKED_BLOCK);
        (*pOffset) += pPackedBlock->BlockByteCount + 1;

        if(pPackedBlocks)
        {
             *pPackedBlocks = pPackedBlock;
             pPackedBlocks++;
        }

        if(pPackedBlock->BlockByteCount == 0)
        {
            bMoreBlocks = FALSE;
        }

        CurrentRasterBlock++;			
    } while(bMoreBlocks);

    return CurrentRasterBlock;
}


/***********************************************************************
 * Gif_DisplayDebugInformation
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     Nothing
 *
 ***********************************************************************/
void WINAPI Gif_DisplayDebugInformation(PGIF_INTERNAL pGifInternal)
{
    UINT Index;

    DEBUGPRINT2("Signature (%c%c%c%c%c%c)\n", pGifInternal->pGifHeader->Signature[0], pGifInternal->pGifHeader->Signature[1], pGifInternal->pGifHeader->Signature[2],pGifInternal->pGifHeader->Version[0], pGifInternal->pGifHeader->Version[1], pGifInternal->pGifHeader->Version[2]);
    DEBUGPRINT2("Resolution (%i, %i)\n", pGifInternal->pScreenDescriptor->ScreenWidth, pGifInternal->pScreenDescriptor->ScreenHeight);
    DEBUGPRINT2("Bits Per Pixel (%i)\n", (int)(pGifInternal->pScreenDescriptor->BitsPerPixel + 1));
    DEBUGPRINT2("Global Color Map Defined (%i)\n", (int)(pGifInternal->pScreenDescriptor->GlobalMapDefined));
    DEBUGPRINT2("Background Color Index (%i)\n", pGifInternal->pScreenDescriptor->ScreenBackgroundColorIndex);

    Index = 0;
    do {
        DEBUGPRINT2("\nImage # %i\n", Index);
        DEBUGPRINT2(" Image Signature (%c)\n", pGifInternal->ImageData[Index].pImageDescriptor->ImageSeperator);
        DEBUGPRINT2(" Image Position (%i, %i)\n", pGifInternal->ImageData[Index].pImageDescriptor->ImageStartLeft, pGifInternal->ImageData[Index].pImageDescriptor->ImageStartTop);
        DEBUGPRINT2(" Image Size (%i, %i)\n", pGifInternal->ImageData[Index].pImageDescriptor->ImageWidth, pGifInternal->ImageData[Index].pImageDescriptor->ImageHeight);
        DEBUGPRINT2(" Bits Per Pixel (%i)\n", pGifInternal->ImageData[Index].pImageDescriptor->BitsPerPixel + 1);
        DEBUGPRINT2(" Image is Interlaced (%i)\n", pGifInternal->ImageData[Index].pImageDescriptor->ImageIsInterlaced);
        DEBUGPRINT2(" Image uses Local Map (%i)\n", pGifInternal->ImageData[Index].pImageDescriptor->UseLocalMap);
        DEBUGPRINT2(" Raster Data contains (%i) blocks\n", pGifInternal->ImageData[Index].RasterData.NumberOfBlocks);

        Index++;
    } while(Index < pGifInternal->NumberOfImages);

}

/***********************************************************************
 * Gif_NumberOfImages
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     Nothing
 *
 ***********************************************************************/
UINT WINAPI Gif_NumberOfImages(HGIF hGif)
{
    PGIF_INTERNAL pGifInternal = (PGIF_INTERNAL)hGif;

    return pGifInternal->NumberOfImages;
}



/***********************************************************************
 * Gif_NumberOfImages
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     Nothing
 *
 ***********************************************************************/
UINT WINAPI Gif_GetImageSize(HGIF hGif, UINT Index)
{
    PGIF_INTERNAL pGifInternal = (PGIF_INTERNAL)hGif;
    UINT SizeCalculation;

    SizeCalculation = pGifInternal->pScreenDescriptor->ScreenWidth*pGifInternal->pScreenDescriptor->ScreenHeight*4;

    return SizeCalculation;
}

/***********************************************************************
 * Gif_GetImageWidth
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     Nothing
 *
 ***********************************************************************/
UINT WINAPI Gif_GetImageWidth(HGIF hGif, UINT Index)
{
    PGIF_INTERNAL pGifInternal = (PGIF_INTERNAL)hGif;

    return pGifInternal->pScreenDescriptor->ScreenWidth;
}


/***********************************************************************
 * Gif_GetImageHeight
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     Nothing
 *
 ***********************************************************************/
UINT WINAPI Gif_GetImageHeight(HGIF hGif, UINT Index)
{
    PGIF_INTERNAL pGifInternal = (PGIF_INTERNAL)hGif;

    return pGifInternal->pScreenDescriptor->ScreenHeight;
}



/***********************************************************************
 * Gif_GetImage32bpp
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     Nothing
 *                                                   
 ***********************************************************************/
void WINAPI Gif_GetImage32bpp(HGIF hGif, UINT Index, UCHAR *pImageBuffer32bpp)
{
    PGIF_INTERNAL pGifInternal = (PGIF_INTERNAL)hGif;
    PIMAGE_DATA pImageData;
    UINT Stride;
    UINT Width;
    UINT StartOffset;

    if(Index < pGifInternal->NumberOfImages)
    {
        pImageData = &pGifInternal->ImageData[Index];
        Gif_SetBackgroundColor(pGifInternal, Index, pImageBuffer32bpp);

        StartOffset = pImageData->pImageDescriptor->ImageStartLeft + (pImageData->pImageDescriptor->ImageStartTop*pGifInternal->pScreenDescriptor->ScreenWidth);
        pImageBuffer32bpp += (StartOffset*4);

        Stride = (pGifInternal->pScreenDescriptor->ScreenWidth - pImageData->pImageDescriptor->ImageWidth);

        Gif_Decode(pGifInternal, pImageData, Stride, pImageBuffer32bpp);
    }
} 


/***********************************************************************
 * Gif_InitializeStringTable
 *  
 *    Gif_InitializeStringTable 
 *
 *    
 *
 * Parameters
 *     Gif_InitializeStringTable
 *
 * Return Value
 *     Nothing
 *
 ***********************************************************************/
void WINAPI Gif_InitializeStringTable(PGIF_INTERNAL pGifInternal, PIMAGE_DATA pImageData, PDECODE_STRING_TABLE pDecodeStringTable)
{
    pDecodeStringTable->ClearCode        = (UINT)(pow(2, pImageData->RasterData.CodeSize));
    pDecodeStringTable->EndOfInformation = pDecodeStringTable->ClearCode + 1;
    pDecodeStringTable->FirstAvailable   = pDecodeStringTable->ClearCode + 2;
    pDecodeStringTable->CurrentIndex     = 0;
    pDecodeStringTable->CurrentCodeBits  = pImageData->RasterData.CodeSize + 1;
    pDecodeStringTable->ImageWidth       = pImageData->pImageDescriptor->ImageWidth;
    pDecodeStringTable->LastCodeWord     = pDecodeStringTable->ClearCode;
}

/***********************************************************************
 * Gif_Debug
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
 void Gif_Debug(char *pszFormatString, ...)
 {
     char DebugString[256];
     va_list vl;

     va_start(vl, pszFormatString);
     vsprintf(DebugString, pszFormatString, vl);
     va_end(vl);

     OutputDebugStringA(DebugString);
 }


/***********************************************************************
 * Gif_GetPaletteColorByIndex
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     
 *
 ***********************************************************************/
 DWORD WINAPI Gif_GetPaletteColorByIndex(PGIF_INTERNAL pGifInternal, UINT ImageIndex, UINT ColorIndex)
 {
     DWORD Color32Bit = 0;

     if(pGifInternal->ImageData[ImageIndex].pImageDescriptor->UseLocalMap)
     {
         Color32Bit = CREATE_RGB(pGifInternal->ImageData[ImageIndex].pLocalColorMap->GifRgbIndex[ColorIndex]);
     }
     else
     {
         Color32Bit = CREATE_RGB(pGifInternal->pGlobalColorMap->GifRgbIndex[ColorIndex]);

     }

     return Color32Bit;
 }


/***********************************************************************
 * Gif_SetBackgroundColor
 *  
 *     

 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     
 *
 ***********************************************************************/
 void WINAPI Gif_SetBackgroundColor(PGIF_INTERNAL pGifInternal, UINT ImageIndex, UCHAR *pImageBuffer32bpp)
 {
     DWORD BackgroundColor;
     UINT Index;

     BackgroundColor = Gif_GetPaletteColorByIndex(pGifInternal, ImageIndex, pGifInternal->pScreenDescriptor->ScreenBackgroundColorIndex);

     Index = 0;
     while(Index < ((UINT)pGifInternal->pScreenDescriptor->ScreenWidth*(UINT)pGifInternal->pScreenDescriptor->ScreenHeight))
     {
         *((DWORD *)pImageBuffer32bpp) = BackgroundColor;
         pImageBuffer32bpp += 4;
         Index++;
     }

 }





/***********************************************************************
 * Gif_Decode
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     
 *
 ***********************************************************************/
 void WINAPI Gif_Decode(PGIF_INTERNAL pGifInternal, PIMAGE_DATA pImageData, UINT Stride, UCHAR *pImageBuffer32bpp)
 {
     PDECODE_STRING_TABLE pDecodeStringTable = NULL;
     UINT Index = 0;
     UCHAR *pRasterDataBuffer;

     pDecodeStringTable = (PDECODE_STRING_TABLE)LocalAlloc(LMEM_ZEROINIT, sizeof(DECODE_STRING_TABLE));

     if(pDecodeStringTable)
     {
         pDecodeStringTable->ImageX            = pImageData->pImageDescriptor->ImageStartLeft;
         pDecodeStringTable->ImageStartLeft    = pImageData->pImageDescriptor->ImageStartLeft;
         pDecodeStringTable->ImageY            = pImageData->pImageDescriptor->ImageStartTop;
         pDecodeStringTable->pImageBuffer32bpp = (DWORD *)pImageBuffer32bpp;
         pDecodeStringTable->Stride            = Stride;

         if(pImageData->pImageDescriptor->UseLocalMap)
         {
             pDecodeStringTable->pImagePalette = pImageData->pLocalColorMap->GifRgbIndex;
         }
         else
         {
             pDecodeStringTable->pImagePalette = pGifInternal->pGlobalColorMap->GifRgbIndex;
         }

         Gif_InitializeStringTable(pGifInternal, pImageData, pDecodeStringTable);
         pDecodeStringTable->BitIncrement = 0;
         pDecodeStringTable->RasterDataSize = 0;
                  
         for(Index = 0; Index < pImageData->RasterData.NumberOfBlocks; Index++)
         {
             pDecodeStringTable->RasterDataSize += pImageData->RasterData.pPackBlocks[Index]->BlockByteCount;
         }

         pDecodeStringTable->pRasterDataBuffer = (PCHAR)LocalAlloc(LMEM_ZEROINIT, pDecodeStringTable->RasterDataSize);

         if(pDecodeStringTable->pRasterDataBuffer)
         {
             pRasterDataBuffer = pDecodeStringTable->pRasterDataBuffer;
             for(Index = 0; Index < pImageData->RasterData.NumberOfBlocks; Index++)
             {
                memcpy(pRasterDataBuffer, &pImageData->RasterData.pPackBlocks[Index]->DataBytes[0], pImageData->RasterData.pPackBlocks[Index]->BlockByteCount);
                pRasterDataBuffer += pImageData->RasterData.pPackBlocks[Index]->BlockByteCount;
             }

             Gif_DecodePackedBlock(pGifInternal, pImageData, pDecodeStringTable);
             LocalFree(pDecodeStringTable->pRasterDataBuffer);
         }

         LocalFree(pDecodeStringTable);
     }
 }



 /***********************************************************************
 * Gif_Decode
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     
 *
 ***********************************************************************/
BOOL WINAPI Gif_DecodePackedBlock(PGIF_INTERNAL pGifInternal, PIMAGE_DATA pImageData, PDECODE_STRING_TABLE pDecodeStringTable)
{
    UINT Index;
    UINT EndOfBlock;
    BOOL ContinueProcessing = TRUE;
    UCHAR *pPackedBlockBytes;
    
    EndOfBlock = pDecodeStringTable->RasterDataSize;
    pPackedBlockBytes = pDecodeStringTable->pRasterDataBuffer;

    for(Index = 0; Index < EndOfBlock && ContinueProcessing;)
    {
        pDecodeStringTable->NewCodeWord = Gif_RetrieveCodeWord(pDecodeStringTable, &pDecodeStringTable->BitIncrement, pPackedBlockBytes);

        if(Gif_IsClearCode(pDecodeStringTable, pDecodeStringTable->NewCodeWord))
        {
            Gif_InitializeStringTable(pGifInternal, pImageData, pDecodeStringTable);
        }
        else
        {
            if(Gif_IsEndOfImageCode(pDecodeStringTable, pDecodeStringTable->NewCodeWord))
            {
                DEBUGPRINT2(" EOI Command\n");
                ContinueProcessing = FALSE;
            }
            else
            {
                if(Gif_ProcessNewCode(pDecodeStringTable, pDecodeStringTable->LastCodeWord, pDecodeStringTable->NewCodeWord))
                {
                    Gif_InitializeStringTable(pGifInternal, pImageData, pDecodeStringTable);
                }
                else
                {
                    pDecodeStringTable->LastCodeWord = pDecodeStringTable->NewCodeWord;
                }
            }
        }

        while(pDecodeStringTable->BitIncrement >= 8)
        {
            pPackedBlockBytes++;
            Index++;
            pDecodeStringTable->BitIncrement = pDecodeStringTable->BitIncrement - 8;
        }
        
    }

    DEBUGPRINT2(" Processed %i of %i\n", Index, EndOfBlock);

    return ContinueProcessing;
}



 /***********************************************************************
 * Gif_ProcessNewCode
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     
 *
 ***********************************************************************/
BOOL WINAPI Gif_ProcessNewCode(PDECODE_STRING_TABLE pDecodeStringTable, UINT LastCodeWord, UINT NewCodeWord)
{
    DWORD Pixel;
    BOOL ReinitializeStringTable = FALSE;

    if(NewCodeWord < pDecodeStringTable->ClearCode)
    {
        Pixel = Gif_GetPaletteColorByIndexSpecifyPalette(pDecodeStringTable->pImagePalette, NewCodeWord);
        pDecodeStringTable->pImageBuffer32bpp[pDecodeStringTable->CurrentPixel] = Pixel;
        pDecodeStringTable->CurrentPixel++;

        if(pDecodeStringTable->CurrentPixel >= pDecodeStringTable->ImageWidth)
        {
            pDecodeStringTable->pImageBuffer32bpp += pDecodeStringTable->Stride + pDecodeStringTable->ImageWidth;
            pDecodeStringTable->CurrentPixel = 0;
        }

        if(LastCodeWord != pDecodeStringTable->ClearCode)
        {
            ReinitializeStringTable = Gif_AddNewEntry(pDecodeStringTable, LastCodeWord, NewCodeWord);			
        }
    }
    else
    {	
        UINT PixelIndex;
        
        ReinitializeStringTable = Gif_AddNewEntry(pDecodeStringTable, LastCodeWord, NewCodeWord);		

        for(PixelIndex = 0; PixelIndex < pDecodeStringTable->StringTable[CODE_TO_INDEX(NewCodeWord, pDecodeStringTable)].Length; PixelIndex++)
        {
            Pixel = Gif_GetPaletteColorByIndexSpecifyPalette(pDecodeStringTable->pImagePalette, pDecodeStringTable->StringTable[CODE_TO_INDEX(NewCodeWord, pDecodeStringTable)].DecodeString[PixelIndex]);
            pDecodeStringTable->pImageBuffer32bpp[pDecodeStringTable->CurrentPixel] = Pixel;
            pDecodeStringTable->CurrentPixel++;
            pDecodeStringTable->ImageX++;

            
            if(pDecodeStringTable->CurrentPixel >= pDecodeStringTable->ImageWidth)
            {
                pDecodeStringTable->pImageBuffer32bpp += pDecodeStringTable->Stride + pDecodeStringTable->ImageWidth;
                pDecodeStringTable->CurrentPixel = 0;
                
                pDecodeStringTable->ImageX = pDecodeStringTable->ImageStartLeft;
                pDecodeStringTable->ImageY++;
            }
            
        }
    }

    return ReinitializeStringTable;
}


 /***********************************************************************
 * Gif_ProcessNewCode
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     
 *
 ***********************************************************************/
BOOL WINAPI Gif_AddNewEntry(PDECODE_STRING_TABLE pDecodeStringTable, UINT LastCodeWord, UINT NewCodeWord)
{
    STRING_TABLE FrontString;
    STRING_TABLE BackString;
    BOOL ReinitializeStringTable = FALSE;

    if(NewCodeWord < pDecodeStringTable->ClearCode || CODE_TO_INDEX(NewCodeWord,pDecodeStringTable) < pDecodeStringTable->CurrentIndex)
    {
        if(NewCodeWord < pDecodeStringTable->ClearCode)
        {
            BackString.DecodeString[0] = (UCHAR)NewCodeWord;
            BackString.Length          = 1;
        }
        else
        {
            BackString.DecodeString[0] = pDecodeStringTable->StringTable[CODE_TO_INDEX(NewCodeWord, pDecodeStringTable)].DecodeString[0];
            BackString.Length          = 1;
        }

        if(LastCodeWord < pDecodeStringTable->ClearCode)
        {
            FrontString.DecodeString[0] = (UCHAR)LastCodeWord;
            FrontString.Length          = 1;
        }
        else
        {
            memcpy(&FrontString, &pDecodeStringTable->StringTable[CODE_TO_INDEX(LastCodeWord, pDecodeStringTable)], sizeof(STRING_TABLE));
        }
    }
    else
    {
        if(LastCodeWord < pDecodeStringTable->ClearCode)
        {
            BackString.DecodeString[0] = (UCHAR)LastCodeWord;
            BackString.Length          = 1;

            FrontString.DecodeString[0] = (UCHAR)LastCodeWord;
            FrontString.Length          = 1;
        }
        else
        {
            BackString.DecodeString[0] = pDecodeStringTable->StringTable[CODE_TO_INDEX(LastCodeWord, pDecodeStringTable)].DecodeString[0];
            BackString.Length          = 1;

            memcpy(&FrontString, &pDecodeStringTable->StringTable[CODE_TO_INDEX(LastCodeWord, pDecodeStringTable)], sizeof(STRING_TABLE));
        }
    }

    memcpy(pDecodeStringTable->StringTable[pDecodeStringTable->CurrentIndex].DecodeString, FrontString.DecodeString, FrontString.Length);
    memcpy(pDecodeStringTable->StringTable[pDecodeStringTable->CurrentIndex].DecodeString + FrontString.Length, BackString.DecodeString, BackString.Length);
    pDecodeStringTable->StringTable[pDecodeStringTable->CurrentIndex].Length = FrontString.Length + BackString.Length;
    
    if(pDecodeStringTable->StringTable[pDecodeStringTable->CurrentIndex].Length >= 4096)
    {
        DebugBreak();
    }

    if(INDEX_TO_CODE(pDecodeStringTable->CurrentIndex, pDecodeStringTable) == 4096)
    {
        ReinitializeStringTable = TRUE;
        DEBUGPRINT2(" Re-Initialize String Table %i %i\n", pDecodeStringTable->CurrentIndex, INDEX_TO_CODE(pDecodeStringTable->CurrentIndex, pDecodeStringTable));
    }

    pDecodeStringTable->CurrentIndex++; 

    if(INDEX_TO_CODE(pDecodeStringTable->CurrentIndex, pDecodeStringTable) == (UINT)pow(2, pDecodeStringTable->CurrentCodeBits))
    {
        if(pDecodeStringTable->CurrentCodeBits < 12)
        {
            pDecodeStringTable->CurrentCodeBits++;
        }
        DEBUGPRINT2(" Code Bits Increase %i\n", pDecodeStringTable->CurrentCodeBits);
    }

    return ReinitializeStringTable;
}




 /***********************************************************************
 * Gif_RetrieveCodeWord
 *  
 *     
 *
 *    
 *
 * Parameters
 *     
 *
 * Return Value
 *     
 *
 ***********************************************************************/
UINT WINAPI Gif_RetrieveCodeWord(PDECODE_STRING_TABLE pDecodeStringTable, UINT *pBitIncrement, UCHAR *pPackedBlockBytes)
{
    UINT CodeWord;

    CodeWord = (UINT)(*((UINT *)pPackedBlockBytes) >> *pBitIncrement) & CREATE_BIT_MASK(pDecodeStringTable->CurrentCodeBits);

    *pBitIncrement += pDecodeStringTable->CurrentCodeBits;

    return CodeWord;
}


 