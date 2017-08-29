/***********************************************************************
 * 3DS Library
 *  
 *   
 *
 *
 * Toby Opferman 
 *
 ***********************************************************************/


#include <windows.h>
#include <gdi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <3d.h>
#include <3ds.h>

#define CHUNK_MAIN     0x4d4d
#define CHUNK_EDIT3DS  0x3D3D
#define CHUNK_OBJECT   0x4000
#define CHUNK_MESH     0x4100
#define CHUNK_VERTICES 0x4110
#define CHUNK_FACES    0x4120

#pragma pack(1)
typedef struct _CHUNK_HEADER {
  unsigned short ChunkID;
  unsigned long ChunkLength;
} CHUNK_HEADER, *PCHUNK_HEADER;

typedef struct _POLY_FACE {
   ULONG IndexA;
   ULONG IndexB;
   ULONG IndexC;
   ULONG FaceInfo;
} POLY_FACE, *PPOLY_FACE;


#define MAX_OBJECTS 50

typedef struct _MESH_OBJECTS {
   
   UINT NumberOfVertices;
   PTD_POINT pVertices;
   
   ULONG NumberOfPolys;
   
   PPOLY_FACE pPolyFaces;
   
} MESH_OBJECTS, *PMESH_OBJECTS;

typedef struct _INTERNAL_3DS {
   H3D h3D;
   UINT NumberOfObjects;
   MESH_OBJECTS MeshObjects[MAX_OBJECTS];
   
   THREE_D_OBJECT ThreeDObject[MAX_OBJECTS];
   
} INTERNAL_3DS, *PINTERNAL_3DS;

/*
 * Internal Prototypes
 */
BOOL ThreeDS_Read3dsFile(PINTERNAL_3DS pInternal3ds, unsigned char *pszFileName);
BOOL ThreeDS_Read3dsFile_ReadMain(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize);
BOOL ThreeDS_Read3dsFile_Read3ds(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize);
BOOL ThreeDS_Read3dsFile_ReadObject(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize);
BOOL ThreeDS_Read3dsFile_Read3DsChunk(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize);
BOOL ThreeDS_Read3dsFile_ReadObject(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize);
BOOL ThreeDS_Read3dsFile_ReadMesh(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize);
BOOL ThreeDS_Read3dsFile_ReadVertices(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize);
BOOL ThreeDS_Read3dsFile_ReadPolys(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize);
void ThreeDS_PopulateObject(PINTERNAL_3DS pInternal3ds);
	
 /***********************************************************************
  * ThreeDS_Init
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
H3DS ThreeDS_Init(H3D h3D)
{
    PINTERNAL_3DS pInternal3ds;
	
	pInternal3ds = LocalAlloc(LMEM_ZEROINIT, sizeof(INTERNAL_3DS));
	
	if(pInternal3ds)
	{
	    pInternal3ds->h3D = h3D;
        ThreeDS_Read3dsFile(pInternal3ds, "skeleton.3DS");	
		ThreeDS_PopulateObject(pInternal3ds);
	}
	
	return pInternal3ds;
}


 /***********************************************************************
  * ThreeDS_Close
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
void ThreeDS_PopulateObject(PINTERNAL_3DS pInternal3ds)
{
  UINT Index;
  UINT Index2;
  
  for(Index = 0; Index < pInternal3ds->NumberOfObjects; Index++)
  {
      pInternal3ds->ThreeDObject[Index].NumberOfVertices = pInternal3ds->MeshObjects[Index].NumberOfVertices;
	  pInternal3ds->ThreeDObject[Index].NumberOfFaces    = pInternal3ds->MeshObjects[Index].NumberOfPolys;
	  pInternal3ds->ThreeDObject[Index].pTriFaces = LocalAlloc(LMEM_ZEROINIT, sizeof(TRI_FACE)*pInternal3ds->ThreeDObject[Index].NumberOfFaces);
	  pInternal3ds->ThreeDObject[Index].pVertices = LocalAlloc(LMEM_ZEROINIT, sizeof(TD_POINT)*pInternal3ds->ThreeDObject[Index].NumberOfVertices);
	  
	  for(Index2 = 0; Index2 < pInternal3ds->MeshObjects[Index].NumberOfVertices; Index2++)
	  {
	     pInternal3ds->ThreeDObject[Index].pVertices[Index2] = pInternal3ds->MeshObjects[Index].pVertices[Index2];
	  }
	  
	  for(Index2 = 0; Index2 < pInternal3ds->MeshObjects[Index].NumberOfPolys; Index2++)
	  {
	     pInternal3ds->ThreeDObject[Index].pTriFaces[Index2].VertexIndex[0] = pInternal3ds->MeshObjects[Index].pPolyFaces[Index2].IndexA;
		 pInternal3ds->ThreeDObject[Index].pTriFaces[Index2].VertexIndex[1] = pInternal3ds->MeshObjects[Index].pPolyFaces[Index2].IndexB;
		 pInternal3ds->ThreeDObject[Index].pTriFaces[Index2].VertexIndex[2] = pInternal3ds->MeshObjects[Index].pPolyFaces[Index2].IndexC;
		 pInternal3ds->ThreeDObject[Index].pTriFaces[Index2].Color.PixelColor = 0xFFFFFF;
	  }	  
  }
  
}

 /***********************************************************************
  * ThreeDS_Close
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
void ThreeDS_Close(H3DS h3Ds)
{
    PINTERNAL_3DS pInternal3ds= (PINTERNAL_3DS)h3Ds;
}


 /***********************************************************************
  * ThreeDS_Read3dsFile
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
BOOL ThreeDS_Read3dsFile(PINTERNAL_3DS pInternal3ds, unsigned char *pszFileName)
{
    HANDLE hFile;
	DWORD BytesRead;
	BOOL bComplete;
	unsigned char *p3DsFileContents;
	DWORD FileSize;
	
	bComplete = FALSE;
	
	hFile = CreateFile(pszFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if(hFile != NULL && hFile != INVALID_HANDLE_VALUE)
	{
	    FileSize = GetFileSize(hFile, NULL);
		
	    if(FileSize != 0)
		{
			p3DsFileContents = LocalAlloc(LMEM_ZEROINIT, FileSize);
			if(p3DsFileContents)
			{
			   if(ReadFile(hFile, p3DsFileContents, FileSize, &BytesRead, NULL))
			   {
					bComplete = ThreeDS_Read3dsFile_ReadMain(pInternal3ds, p3DsFileContents, BytesRead);
			   }
			}
		}	    
	
	    CloseHandle(hFile);    
	}
	
	return bComplete;
}

 /***********************************************************************
  * ThreeDS_Draw
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
BOOL ThreeDS_Draw(H3DS h3Ds)
{
   PINTERNAL_3DS pInternal3ds= (PINTERNAL_3DS)h3Ds;
   UINT IndexObject;
   UINT IndexVertice;
   TD_POINT WorldView = {0};
   TD_PIXEL TdPixel;
   ULONG IndexA;
   ULONG IndexB;
   ULONG IndexC;
   
   TdPixel.Color.PixelColor = 0xFFFFFF;
#if 0   
   for(IndexObject = 0; IndexObject < pInternal3ds->NumberOfObjects; IndexObject++)
   {
		for(IndexVertice = 0; IndexVertice < pInternal3ds->MeshObjects[IndexObject].NumberOfVertices; IndexVertice++)
		{	
		    TdPixel.Point = pInternal3ds->MeshObjects[IndexObject].pVertices[IndexVertice];
			ThreeD_PlotPixel(pInternal3ds->h3D, &TdPixel, &WorldView);
		}
   }
#endif
   
   for(IndexObject = 0; IndexObject < pInternal3ds->NumberOfObjects; IndexObject++)
   {
		for(IndexVertice = 0; IndexVertice < pInternal3ds->MeshObjects[IndexObject].NumberOfPolys; IndexVertice++)
		{	
			IndexA = pInternal3ds->MeshObjects[IndexObject].pPolyFaces[IndexVertice].IndexA;
			IndexB = pInternal3ds->MeshObjects[IndexObject].pPolyFaces[IndexVertice].IndexB;
			IndexC = pInternal3ds->MeshObjects[IndexObject].pPolyFaces[IndexVertice].IndexC;
			
			ThreeD_DrawLine(pInternal3ds->h3D, &pInternal3ds->MeshObjects[IndexObject].pVertices[IndexA], &pInternal3ds->MeshObjects[IndexObject].pVertices[IndexB], &TdPixel.Color, &WorldView);
			ThreeD_DrawLine(pInternal3ds->h3D, &pInternal3ds->MeshObjects[IndexObject].pVertices[IndexB], &pInternal3ds->MeshObjects[IndexObject].pVertices[IndexC], &TdPixel.Color, &WorldView);
			ThreeD_DrawLine(pInternal3ds->h3D, &pInternal3ds->MeshObjects[IndexObject].pVertices[IndexC], &pInternal3ds->MeshObjects[IndexObject].pVertices[IndexA], &TdPixel.Color, &WorldView);
		}
   }
   
   return TRUE;
}

 /***********************************************************************
  * ThreeDS_Draw
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
BOOL ThreeDS_Draw2(H3DS h3Ds)
{
   PINTERNAL_3DS pInternal3ds= (PINTERNAL_3DS)h3Ds;
   UINT IndexObject;
   UINT IndexVertice;
   TD_POINT WorldView = {0};
   TD_PIXEL TdPixel;
   
   TdPixel.Color.PixelColor = 0xFFFFFF;

   for(IndexObject = 0; IndexObject < pInternal3ds->NumberOfObjects; IndexObject++)
   {
		for(IndexVertice = 0; IndexVertice < pInternal3ds->MeshObjects[IndexObject].NumberOfVertices; IndexVertice++)
		{	
		    TdPixel.Point = pInternal3ds->MeshObjects[IndexObject].pVertices[IndexVertice];
			ThreeD_PlotPixel(pInternal3ds->h3D, &TdPixel, &WorldView);
		}
   }

   return TRUE;
}


 /***********************************************************************
  * ThreeDS_Draw
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
BOOL ThreeDS_Draw3(H3DS h3Ds)
{
   PINTERNAL_3DS pInternal3ds= (PINTERNAL_3DS)h3Ds;
   UINT IndexObject;

   for(IndexObject = 0; IndexObject < pInternal3ds->NumberOfObjects; IndexObject++)
   {
         ThreeD_DrawObjectSolid(pInternal3ds->h3D, &pInternal3ds->ThreeDObject[IndexObject], TRUE);
   }

   return TRUE;
}


 /***********************************************************************
  * ThreeDS_Read3dsFile_ReadMain
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
void ThreeDS_Rotate(PVOID Context, PTD_ROTATION pTdRotation)
{
   PINTERNAL_3DS pInternal3ds= (PINTERNAL_3DS)Context;
   UINT IndexObject;

   for(IndexObject = 0; IndexObject < pInternal3ds->NumberOfObjects; IndexObject++)
   {
      pInternal3ds->ThreeDObject[IndexObject].LocalRotation = *pTdRotation;
   }
}


 /***********************************************************************
  * ThreeDS_World
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
void ThreeDS_World(PVOID Context, PTD_POINT pWorld)
{
   PINTERNAL_3DS pInternal3ds = (PINTERNAL_3DS)Context;
   UINT IndexObject;

   for(IndexObject = 0; IndexObject < pInternal3ds->NumberOfObjects; IndexObject++)
   {
      pInternal3ds->ThreeDObject[IndexObject].WorldCoordinates = *pWorld;
   }
}

 /***********************************************************************
  * ThreeDS_Read3dsFile_ReadMain
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
BOOL ThreeDS_Read3dsFile_ReadMain(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize)
{
   BOOL bReadComplete;   
   BOOL bFoundEdit3Ds;
   UINT Index;
   PCHUNK_HEADER pChunkHeader;
   
   bReadComplete = FALSE;
   bFoundEdit3Ds = FALSE;
   pChunkHeader = (PCHUNK_HEADER)pEntire3dsFile;
   
   if(pChunkHeader->ChunkID == CHUNK_MAIN)
   {
       pEntire3dsFile += sizeof(CHUNK_HEADER);
	   pChunkHeader = (PCHUNK_HEADER)pEntire3dsFile;
	   
       while(pEntire3dsFile < (pEntire3dsFile + dwSize) && bFoundEdit3Ds == FALSE)
	   {
	      if(pChunkHeader->ChunkID == CHUNK_EDIT3DS)
		  {
		      bFoundEdit3Ds = TRUE;
			  bReadComplete = ThreeDS_Read3dsFile_Read3DsChunk(pInternal3ds, pEntire3dsFile, pChunkHeader->ChunkLength);
		  }
		  else
		  {
			  ThreeD_Debug("Chunk Id 0x%0x, Chunk Size %i\n", pChunkHeader->ChunkID, pChunkHeader->ChunkLength);
			  pEntire3dsFile = pEntire3dsFile + pChunkHeader->ChunkLength;
			  pChunkHeader = (PCHUNK_HEADER)pEntire3dsFile;
		  }
	   }
   }  
   
   return bReadComplete;
}






 /***********************************************************************
  * ThreeDS_Read3dsFile_Read3DsChunk
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
BOOL ThreeDS_Read3dsFile_Read3DsChunk(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize)
{
   BOOL bReadComplete; 
   UINT Index;
   PCHUNK_HEADER pChunkHeader;
   unsigned char *pEndOfBuffer;
   
   pEndOfBuffer = pEntire3dsFile + dwSize;
	
   bReadComplete = FALSE;
   pEntire3dsFile += sizeof(CHUNK_HEADER);
   pChunkHeader = (PCHUNK_HEADER)pEntire3dsFile;
	   
   while(pChunkHeader->ChunkLength && pEntire3dsFile < pEndOfBuffer)
   {
	  ThreeD_Debug("Chunk Id 0x%0x, Chunk Size %i\n", pChunkHeader->ChunkID, pChunkHeader->ChunkLength);
	  
	  if(pChunkHeader->ChunkID == CHUNK_OBJECT)
	  {
	      bReadComplete = ThreeDS_Read3dsFile_ReadObject(pInternal3ds, pEntire3dsFile, pChunkHeader->ChunkLength);
	  }
	  
	  pEntire3dsFile = pEntire3dsFile + pChunkHeader->ChunkLength;
	  pChunkHeader = (PCHUNK_HEADER)pEntire3dsFile;
   }
   
   return bReadComplete;
}

 /***********************************************************************
  * ThreeDS_Read3dsFile_ReadObject
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
BOOL ThreeDS_Read3dsFile_ReadObject(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize)
{
   BOOL bReadComplete; 
   UINT Index;
   PCHUNK_HEADER pChunkHeader;
   unsigned char *pEndOfBuffer;
   char szObjectName[256];

   pEndOfBuffer = pEntire3dsFile + dwSize;
   pEntire3dsFile = pEntire3dsFile + sizeof(CHUNK_HEADER);
   
   strcpy(szObjectName, pEntire3dsFile);
   
   ThreeD_Debug("Object (%s)\n", szObjectName);
   pEntire3dsFile = pEntire3dsFile + strlen(szObjectName) + 1;
	
   bReadComplete = FALSE;
   pChunkHeader = (PCHUNK_HEADER)pEntire3dsFile;
		
   while(pChunkHeader->ChunkLength && pEntire3dsFile < pEndOfBuffer)
   {
	  ThreeD_Debug("Chunk Id 0x%0x, Chunk Size %i\n", pChunkHeader->ChunkID, pChunkHeader->ChunkLength);
	  
	  if(pChunkHeader->ChunkID == CHUNK_MESH)
	  {
	      bReadComplete = ThreeDS_Read3dsFile_ReadMesh(pInternal3ds, pEntire3dsFile, pChunkHeader->ChunkLength);
	  }
	  
	  pEntire3dsFile = pEntire3dsFile + pChunkHeader->ChunkLength;
	  pChunkHeader = (PCHUNK_HEADER)pEntire3dsFile;
   }
   
   return bReadComplete;
}


 /***********************************************************************
  * ThreeDS_Read3dsFile_ReadMesh
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
BOOL ThreeDS_Read3dsFile_ReadMesh(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize)
{
   BOOL bReadComplete; 
   UINT Index;
   PCHUNK_HEADER pChunkHeader;
   unsigned char *pEndOfBuffer;
   char szObjectName[256];

   pEndOfBuffer = pEntire3dsFile + dwSize;
   pEntire3dsFile = pEntire3dsFile + sizeof(CHUNK_HEADER);
	
   bReadComplete = FALSE;
   pChunkHeader = (PCHUNK_HEADER)pEntire3dsFile;
		
   while(pChunkHeader->ChunkLength && pEntire3dsFile < pEndOfBuffer)
   {
	  ThreeD_Debug("Chunk Id 0x%0x, Chunk Size %i\n", pChunkHeader->ChunkID, pChunkHeader->ChunkLength);
	  
	  if(pChunkHeader->ChunkID == CHUNK_VERTICES)
	  {
	      bReadComplete = ThreeDS_Read3dsFile_ReadVertices(pInternal3ds, pEntire3dsFile, pChunkHeader->ChunkLength);
	  }
	  else
	  {
	     if(pChunkHeader->ChunkID == CHUNK_FACES)
		 {
		     bReadComplete = ThreeDS_Read3dsFile_ReadPolys(pInternal3ds, pEntire3dsFile, pChunkHeader->ChunkLength);
		 }
	  }
	  
	  pEntire3dsFile = pEntire3dsFile + pChunkHeader->ChunkLength;
	  pChunkHeader = (PCHUNK_HEADER)pEntire3dsFile;
   }
   
   pInternal3ds->NumberOfObjects++;
   
   if(pInternal3ds->NumberOfObjects == MAX_OBJECTS)
   {
       ThreeD_Debug("Maximum Objects Hit\n");
   }
   
   return bReadComplete;
}

 /***********************************************************************
  * ThreeDS_Read3dsFile_ReadVertices
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
BOOL ThreeDS_Read3dsFile_ReadVertices(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize)
{
   BOOL bReadComplete; 
   UINT Index;
   PCHUNK_HEADER pChunkHeader;
   unsigned char *pEndOfBuffer;
   char szObjectName[256];
   unsigned short NumberOfVertices;
   float X, Y, Z;

   pEndOfBuffer = pEntire3dsFile + dwSize;
   pEntire3dsFile = pEntire3dsFile + sizeof(CHUNK_HEADER);
   
   NumberOfVertices = *((unsigned short *)pEntire3dsFile);
   
   pEntire3dsFile += sizeof(unsigned short);
   ThreeD_Debug("Vertices: %i\n", NumberOfVertices);
	
   bReadComplete = TRUE;
   
   pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].NumberOfVertices = NumberOfVertices;
   pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pVertices = (PTD_POINT)LocalAlloc(LMEM_ZEROINIT, sizeof(TD_POINT)*NumberOfVertices);
   
   if(pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pVertices)
   {
       Index = 0;
	   while(NumberOfVertices > 0)
	   {
	      X = *((float *)pEntire3dsFile) ;
		  pEntire3dsFile = pEntire3dsFile + sizeof(float);
		  
		  Y = *((float *)pEntire3dsFile) ;
		  pEntire3dsFile = pEntire3dsFile + sizeof(float);
		  
		  Z = *((float *)pEntire3dsFile) ;
		  pEntire3dsFile = pEntire3dsFile + sizeof(float);
		  
		 // ThreeD_Debug("(%f, %f, %f)\n", X, Y, Z);
		  
		  pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pVertices[Index].x = X;
		  pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pVertices[Index].y = Y;
		  pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pVertices[Index].z = Z;
		  Index++;
		  
		  NumberOfVertices--;
	   }
    }
   
   return bReadComplete;
}



 /***********************************************************************
  * ThreeDS_Read3dsFile_ReadPolys
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
BOOL ThreeDS_Read3dsFile_ReadPolys(PINTERNAL_3DS pInternal3ds, unsigned char *pEntire3dsFile, DWORD dwSize)
{
   BOOL bReadComplete; 
   UINT Index;
   PCHUNK_HEADER pChunkHeader;
   unsigned char *pEndOfBuffer;
   char szObjectName[256];
   unsigned short NumberOfPolys;
   float X, Y, Z;

   pEndOfBuffer = pEntire3dsFile + dwSize;
   pEntire3dsFile = pEntire3dsFile + sizeof(CHUNK_HEADER);
   
   NumberOfPolys = *((unsigned short *)pEntire3dsFile);
   
   pEntire3dsFile += sizeof(unsigned short);
   ThreeD_Debug("Polys: %i\n", NumberOfPolys);
	
   bReadComplete = TRUE;
   
   pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].NumberOfPolys = NumberOfPolys;
   
   pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pPolyFaces = (PPOLY_FACE)LocalAlloc(LMEM_ZEROINIT, sizeof(POLY_FACE)*NumberOfPolys);
   
   if( pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pPolyFaces)
   {
       Index = 0;
	   while(NumberOfPolys > 0)
	   {
	      pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pPolyFaces[Index].IndexA = *((unsigned short *)pEntire3dsFile) ;
		  pEntire3dsFile = pEntire3dsFile + sizeof(unsigned short);
		  
		  pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pPolyFaces[Index].IndexB = *((unsigned short *)pEntire3dsFile) ;
		  pEntire3dsFile = pEntire3dsFile + sizeof(unsigned short);
		  
		  pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pPolyFaces[Index].IndexC = *((unsigned short *)pEntire3dsFile) ;
		  pEntire3dsFile = pEntire3dsFile + sizeof(unsigned short);
		  
		  pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pPolyFaces[Index].FaceInfo = *((unsigned short *)pEntire3dsFile) ;
		  pEntire3dsFile = pEntire3dsFile + sizeof(unsigned short);
		  
		  //ThreeD_Debug("(%i, %i, %i) Info(%i)\n", pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pPolyFaces[Index].IndexA, pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pPolyFaces[Index].IndexB,pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pPolyFaces[Index].IndexC, pInternal3ds->MeshObjects[pInternal3ds->NumberOfObjects].pPolyFaces[Index].FaceInfo);
		  
		  Index++;
		  NumberOfPolys--;
	   }
    }
   
   return bReadComplete;
}
