/***********************************************************************
 * Toby's 3D Library
 *
 *   This library implements a simple 3D implementation.
 *
 ***********************************************************************/

#include <windows.h>
#include <gdi.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <3d.h>

/*
 * Internal Structure
 */
typedef struct _INTERNAL_3D {
   
   HGDI hScreenBuffer;
   ULONG ScreenHeight;
   double HalfScreenHeight;
   ULONG ScreenWidth;
   double HalfScreenWidth;
   DWORD *pScreenBuffer;
   
   double *pZBuffer;
   ULONG ZBufferSize;
   
   TD_POINT ViewPoint;
   double CameraX_Radians;
   double CameraY_Radians;
   double CameraZ_Radians;
   double Aspect;
   double ViewDistance;

} INTERNAL_3D, *PINTERNAL_3D;


/*
 * Internal Prototypes
 */
void ThreeD_Debug(char *pszFormatString, ...);
TD_PIXEL_STATUS ThreeD_Convert3Dto2D(PINTERNAL_3D pInternal3d, PTD_POINT pTdPoint, PTD_POINT pPixelWorld, PTD_POINT_2D pTdPoint2d, PTD_POINT pTdCamera);
TD_PIXEL_STATUS ThreeD_PlotPixel2DConverted(PINTERNAL_3D pInternal3d, int X, int Y, double Z, DWORD PixelColor);
void ThreeD_DrawLineInternal(PINTERNAL_3D pInternal3d, PTD_POINT_2D pTdPointA, PTD_POINT_2D pTdPointB, PTD_POINT pTdCameraA, PTD_POINT pTdCameraB, DWORD PixelColor);	
BOOL ThreeD_IsFaceVisible(PINTERNAL_3D pInternal3d, PTD_POINT pPointA, PTD_POINT pPointB, PTD_POINT pPointC);
void ThreeD_RotateInternal(PINTERNAL_3D pInternal3d, PTD_POINT pPoint, PTD_ROTATION pLocalRotation);
void ThreeD_DrawFlatToporBottomTriangle(PINTERNAL_3D pInternal3d, PTD_POINT_2D pTdPointA, PTD_POINT_2D pTdPointB, PTD_POINT_2D pTdPointC, PTD_POINT pTdCameraA, PTD_POINT pTdCameraB, PTD_POINT pTdCameraC, DWORD PixelColor);	
void ThreeD_DrawFlatToporBottomTriangleTexture(PINTERNAL_3D pInternal3d, PTD_POINT_2D pTdPointA, PTD_POINT_2D pTdPointB, PTD_POINT_2D pTdPointC, PTD_POINT pTdCameraA, PTD_POINT pTdCameraB, PTD_POINT pTdCameraC, PPTEXTURE_INFO pTextureInfo);
void ThreeD_DrawFlatToporBottomTriangleTextureEx(PINTERNAL_3D pInternal3d, PTD_POINT pTdCameraA, PTD_POINT pTdCameraB, PTD_POINT pTdCameraC, PPTEXTURE_INFO pTextureInfo);
TD_PIXEL_STATUS ThreeD_PlotWithCamera(PINTERNAL_3D pInternal3d, PTD_POINT pTdCamera, PTD_POINT_2D pTdPoint2d, DWORD Pixel);
void ThreeD_DrawFlatToporBottomTriangleTextureEx2(PINTERNAL_3D pInternal3d, PTD_POINT_2D pTdPointA, PTD_POINT_2D pTdPointB, PTD_POINT_2D pTdPointC, PTD_POINT pTdCameraA, PTD_POINT pTdCameraB, PTD_POINT pTdCameraC, PPTEXTURE_INFO pTextureInfo);
BOOL ThreeD_Get3DFrom2D(PINTERNAL_3D pInternal3d, PTD_POINT pTdCamera, PTD_POINT_2D pTdPoint2d);

 /***********************************************************************
  * ThreeD_Init
  *  
  *    Initialize the 3D engine.
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
H3D ThreeD_Init(HGDI hScreenBuffer, ULONG Flags)
{
    PINTERNAL_3D pInternal3d = NULL;
    
    pInternal3d = LocalAlloc(LMEM_ZEROINIT, sizeof(INTERNAL_3D));
    
    if(pInternal3d)
    {
       pInternal3d->hScreenBuffer = hScreenBuffer;
       
       GDI_GetSize(pInternal3d->hScreenBuffer, &pInternal3d->ScreenWidth, &pInternal3d->ScreenHeight);
       
       pInternal3d->HalfScreenWidth  = pInternal3d->ScreenWidth/2.0;
       pInternal3d->HalfScreenHeight = pInternal3d->ScreenHeight/2.0;
       
       pInternal3d->Aspect = (double)pInternal3d->ScreenWidth / (double)pInternal3d->ScreenHeight;

       pInternal3d->pScreenBuffer = (DWORD *)GDI_BeginPaint(pInternal3d->hScreenBuffer);
       
       if(Flags & T3D_FLAG_ZBUFFER)
       {
            pInternal3d->ZBufferSize = pInternal3d->ScreenWidth*pInternal3d->ScreenHeight;
            
            pInternal3d->pZBuffer = LocalAlloc(LMEM_ZEROINIT, sizeof(double)*pInternal3d->ZBufferSize);
       }
       
    }
    
    return pInternal3d;
}


 /***********************************************************************
  * ThreeD_Close
  *  
  *    Close the 3D Engine
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
void ThreeD_Close(H3D h3D)
{
   PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
   
   if(pInternal3d)
   {
       if(pInternal3d->pZBuffer)
       {
           LocalFree(pInternal3d->pZBuffer);
       }
       
       LocalFree(pInternal3d);
   }
}

 /***********************************************************************
  * ThreeD_GetScreenHeight
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
ULONG ThreeD_GetScreenHeight(H3D h3D)
{
   PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
   
   return pInternal3d->ScreenHeight;
}

 /***********************************************************************
  * ThreeD_GetScreenWidth
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
ULONG ThreeD_GetScreenWidth(H3D h3D)
{
   PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
   
   return pInternal3d->ScreenWidth;
}



 /***********************************************************************
  * ThreeD_SetViewPoint
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
void ThreeD_SetViewPoint(H3D h3D, PTD_POINT pViewPoint)
{
   PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
   
   pInternal3d->ViewPoint.x = pViewPoint->x;
   pInternal3d->ViewPoint.y = pViewPoint->y;
   pInternal3d->ViewPoint.z = pViewPoint->z;
}



 /***********************************************************************
  * ThreeD_GetAspectRatio
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
double ThreeD_GetAspectRatio(H3D h3D)
{
   PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
   
   return pInternal3d->Aspect;
}


 /***********************************************************************
  * ThreeD_SetAspectRatio
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
void ThreeD_SetAspectRatio(H3D h3D, double AspectRatio)
{
   PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
   
   pInternal3d->Aspect = AspectRatio;
}


 /***********************************************************************
  * ThreeD_SetViewDistance
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
void ThreeD_SetViewDistance(H3D h3D, double ViewDistance)
{
   PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
   
   pInternal3d->ViewDistance = ViewDistance;
}

 /***********************************************************************
  * ThreeD_SetCameraRotation
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
void ThreeD_SetCameraRotation(H3D h3D, double X_Radians, double Y_Radians, double Z_Radians)
{
   PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
  
   pInternal3d->CameraX_Radians = X_Radians;
   pInternal3d->CameraY_Radians = Y_Radians;
   pInternal3d->CameraZ_Radians = Z_Radians;
}

 /***********************************************************************
  * ThreeD_ClearScreen
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
void ThreeD_ClearScreen(H3D h3D)
{
   PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
   UINT Index;
   
   if(pInternal3d->pZBuffer)
   {
      for(Index = 0; Index < pInternal3d->ZBufferSize; Index++)
      {
         pInternal3d->pZBuffer[Index] = 1000000;
      }
   }
   
   GDI_ClearVideoBuffer(pInternal3d->hScreenBuffer);
}


 /***********************************************************************
  * ThreeD_DrawLine
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
void ThreeD_DrawLine(H3D h3D, PTD_POINT pTdPointA, PTD_POINT pTdPointB, PPIXEL_COLOR pLineColor, PTD_POINT pPixelWorld)
{
     PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
     TD_POINT_2D TdPointA;
     TD_POINT_2D TdPointB;
     TD_POINT TdCameraA;
     TD_POINT TdCameraB;

     ThreeD_Convert3Dto2D(pInternal3d, pTdPointA, pPixelWorld, &TdPointA, &TdCameraA);
     ThreeD_Convert3Dto2D(pInternal3d, pTdPointB, pPixelWorld, &TdPointB, &TdCameraB);
     
     ThreeD_DrawLineInternal(pInternal3d, &TdPointA, &TdPointB, &TdCameraA, &TdCameraB, pLineColor->PixelColor);
}



 /***********************************************************************
  * ThreeD_Convert3Dto2D
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
TD_PIXEL_STATUS ThreeD_Convert3Dto2D(PINTERNAL_3D pInternal3d, PTD_POINT pTdPoint, PTD_POINT pPixelWorld, PTD_POINT_2D pTdPoint2d, PTD_POINT pTdCamera)
{
    TD_PIXEL_STATUS PixelStatus;
    double WorldX;
    double WorldY;
    double WorldZ;
    double CameraX;
    double CameraY;
    double CameraZ;
    double RotatedCx;
    double RotatedCy;
    double RotatedCz;
    double Projected2DX_Raw;
    double Projected2DY_Raw;
    
    // Plot the World Coordinates from Local Coordinates
    if(pPixelWorld != NULL)
    {
        WorldX = pTdPoint->x + pPixelWorld->x;
        WorldY = pTdPoint->y + pPixelWorld->y;
        WorldZ = pTdPoint->z + pPixelWorld->z;
    }
   
    // Next need to transform the image onto the Camera
    CameraX = WorldX - pInternal3d->ViewPoint.x;
    CameraY = WorldY - pInternal3d->ViewPoint.y;
    CameraZ = WorldZ - pInternal3d->ViewPoint.z;
    
    // Rotate the Camera
    if(pInternal3d->CameraX_Radians)
    {
        RotatedCy = ROTATE_X_NEW_Y(CameraX, CameraY, CameraZ, pInternal3d->CameraX_Radians);
        RotatedCz = ROTATE_X_NEW_Z(CameraX, CameraY, CameraZ, pInternal3d->CameraX_Radians);
        CameraY = RotatedCy;
        CameraZ = RotatedCz;
    }
    
    if(pInternal3d->CameraY_Radians)
    {
        RotatedCx = ROTATE_Y_NEW_X(CameraX, CameraY, CameraZ, pInternal3d->CameraY_Radians);
        RotatedCz = ROTATE_Y_NEW_Z(CameraX, CameraY, CameraZ, pInternal3d->CameraY_Radians);
        CameraX = RotatedCx;
        CameraZ = RotatedCz;	
    }
    
    if(pInternal3d->CameraZ_Radians)
    {
        RotatedCx = ROTATE_Z_NEW_X(CameraX, CameraY, CameraZ, pInternal3d->CameraZ_Radians);
        RotatedCy = ROTATE_Z_NEW_Y(CameraX, CameraY, CameraZ, pInternal3d->CameraZ_Radians);
        CameraX = RotatedCx;
        CameraY = RotatedCy;		
    }

    // Save the camera
    pTdCamera->x = CameraX;
    pTdCamera->y = CameraY;
    pTdCamera->z = CameraZ;
    
    // Finally, project the image into 2D.	
    if(CameraZ > 0)
    {
        Projected2DX_Raw = (((pInternal3d->ViewDistance*CameraX/CameraZ) + pInternal3d->HalfScreenWidth));
        Projected2DY_Raw = ((pInternal3d->HalfScreenHeight - (pInternal3d->ViewDistance*CameraY/CameraZ)));
        
        // Change the Aspect Ratio
        if(pInternal3d->Aspect > 1.0)
        {
            pTdPoint2d->x = (int)(Projected2DX_Raw / pInternal3d->Aspect);
            pTdPoint2d->y = (int)(Projected2DY_Raw);
        }
        else
        {
            pTdPoint2d->y = (int)(Projected2DY_Raw * pInternal3d->Aspect);
            pTdPoint2d->x = (int)(Projected2DX_Raw);
        }
        
        if(pTdPoint2d->x < 0 || pTdPoint2d->y < 0 || 
            pTdPoint2d->x >= (int)pInternal3d->ScreenWidth || pTdPoint2d->y >= (int)pInternal3d->ScreenHeight)
        {
        
           PixelStatus = TdPixelOffScreen;
        }
        else
        {
            PixelStatus = TdPixelPlotted;
        }
    }
    else
    {
       PixelStatus = TdPixelOffScreen;
    }
    
    return PixelStatus;
}



 /***********************************************************************
  * ThreeD_PlotWithCamera
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
TD_PIXEL_STATUS ThreeD_PlotWithCamera(PINTERNAL_3D pInternal3d, PTD_POINT pTdCamera, PTD_POINT_2D pTdPoint2d, DWORD Pixel)
{
    TD_PIXEL_STATUS PixelStatus;
    double Projected2DX_Raw;
    double Projected2DY_Raw;
    
    if(pTdCamera->z > 0)
    {
        Projected2DX_Raw = (((pInternal3d->ViewDistance*pTdCamera->x/pTdCamera->z) + pInternal3d->HalfScreenWidth));
        Projected2DY_Raw = ((pInternal3d->HalfScreenHeight - (pInternal3d->ViewDistance*pTdCamera->y/pTdCamera->z)));
        
        // Change the Aspect Ratio
        if(pInternal3d->Aspect > 1.0)
        {
            pTdPoint2d->x = (int)(Projected2DX_Raw / pInternal3d->Aspect);
            pTdPoint2d->y = (int)(Projected2DY_Raw);
        }
        else
        {
            pTdPoint2d->y = (int)(Projected2DY_Raw * pInternal3d->Aspect);
            pTdPoint2d->x = (int)(Projected2DX_Raw);
        }
        
        if(pTdPoint2d->x < 0 || pTdPoint2d->y < 0 || 
            pTdPoint2d->x >= (int)pInternal3d->ScreenWidth || pTdPoint2d->y >= (int)pInternal3d->ScreenHeight)
        {
        
           PixelStatus = TdPixelOffScreen;
        }
        else
        {
           if(pInternal3d->pZBuffer)
           {
               if(pInternal3d->pZBuffer[(pTdPoint2d->y*pInternal3d->ScreenWidth) + pTdPoint2d->x] > pTdCamera->z)
               {
                    pInternal3d->pScreenBuffer[(pTdPoint2d->y*pInternal3d->ScreenWidth) + pTdPoint2d->x] = Pixel;
                    PixelStatus = TdPixelPlotted;	
                    pInternal3d->pZBuffer[(pTdPoint2d->y*pInternal3d->ScreenWidth) + pTdPoint2d->x] = pTdCamera->z;
               }
               else
               {
                   PixelStatus = TdPixelHidden;
               }
           }
           else
           {
               pInternal3d->pScreenBuffer[(pTdPoint2d->y*pInternal3d->ScreenWidth) + pTdPoint2d->x] = Pixel;
               PixelStatus = TdPixelPlotted;		   
           }
        }
    }
    else
    {
       PixelStatus = TdPixelOffScreen;
    }
    
    return PixelStatus;
}



 /***********************************************************************
  * ThreeD_Get3DFrom2D
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
BOOL ThreeD_Get3DFrom2D(PINTERNAL_3D pInternal3d, PTD_POINT pTdCamera, PTD_POINT_2D pTdPoint2d)
{
    BOOL bPopulatedCoordinates = FALSE;
    TD_PIXEL_STATUS PixelStatus;
    double Projected2DX_Raw;
    double Projected2DY_Raw;
    
    if(pTdCamera->z > 0)
    {
        if(pInternal3d->Aspect > 1.0)
        {
            Projected2DX_Raw = pTdPoint2d->x*pInternal3d->Aspect;
            Projected2DY_Raw = pTdPoint2d->y;
        }
        else
        {
            Projected2DY_Raw = pTdPoint2d->y/pInternal3d->Aspect;
            Projected2DX_Raw = pTdPoint2d->x;
        }
        
        Projected2DX_Raw = Projected2DX_Raw - pInternal3d->HalfScreenWidth;
        Projected2DY_Raw = pInternal3d->HalfScreenHeight - Projected2DY_Raw;
        
        Projected2DX_Raw *= pTdCamera->z;
        Projected2DY_Raw *= pTdCamera->z;
        
        Projected2DX_Raw /= pInternal3d->ViewDistance;
        Projected2DY_Raw /= pInternal3d->ViewDistance;
        
        pTdCamera->x = Projected2DX_Raw;
        pTdCamera->y = Projected2DY_Raw;
        
        bPopulatedCoordinates = TRUE;
    }
    
    return bPopulatedCoordinates;
}

 /***********************************************************************
  * ThreeD_PlotPixel
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
TD_PIXEL_STATUS ThreeD_PlotPixel(H3D h3D, PTD_PIXEL pTdPixel, PTD_POINT pPixelWorld)
{
    PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
    TD_PIXEL_STATUS PixelStatus;
    TD_POINT_2D TdPoint2D;
    TD_POINT TdCamera;
    
    PixelStatus = ThreeD_Convert3Dto2D(pInternal3d, &pTdPixel->Point, pPixelWorld, &TdPoint2D, &TdCamera);
    
    if(PixelStatus == TdPixelPlotted)
    {
       if(pInternal3d->pZBuffer)
       {
           if(pInternal3d->pZBuffer[(TdPoint2D.y*pInternal3d->ScreenWidth) + TdPoint2D.x] > TdCamera.z)
           {
                pInternal3d->pScreenBuffer[(TdPoint2D.y*pInternal3d->ScreenWidth) + TdPoint2D.x] = pTdPixel->Color.PixelColor;
                PixelStatus = TdPixelPlotted;	
                pInternal3d->pZBuffer[(TdPoint2D.y*pInternal3d->ScreenWidth) + TdPoint2D.x] = TdCamera.z;
           }
           else
           {
               PixelStatus = TdPixelHidden;
           }
       }
       else
       {
           pInternal3d->pScreenBuffer[(TdPoint2D.y*pInternal3d->ScreenWidth) + TdPoint2D.x] = pTdPixel->Color.PixelColor;
           PixelStatus = TdPixelPlotted;		   
       }
    }
       
    return PixelStatus;
}

 /***********************************************************************
  * ThreeD_PlotPixel2DConverted
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
TD_PIXEL_STATUS ThreeD_PlotPixel2DConverted(PINTERNAL_3D pInternal3d, int X, int Y, double Z, DWORD PixelColor)
{
    TD_PIXEL_STATUS PixelStatus;
    TD_POINT_2D TdPoint2D;
    
    if(X < 0 || Y < 0 || 
        X >= (int)pInternal3d->ScreenWidth || Y >= (int)pInternal3d->ScreenHeight)	
    {
        PixelStatus = TdPixelOffScreen;
    }
    else
    {
        if(pInternal3d->pZBuffer)
        {
           if(pInternal3d->pZBuffer[(Y*pInternal3d->ScreenWidth) + X] > Z)
           {
                  
                pInternal3d->pScreenBuffer[(Y*pInternal3d->ScreenWidth) + X] = PixelColor;
                PixelStatus = TdPixelPlotted;	
                pInternal3d->pZBuffer[(Y*pInternal3d->ScreenWidth) + X] = Z;
           }
           else
           {
               PixelStatus = TdPixelHidden;
           }
        }
        else
        {
            pInternal3d->pScreenBuffer[(Y*pInternal3d->ScreenWidth) + X] = PixelColor;
            PixelStatus = TdPixelPlotted;	
        }
    }
       
    return PixelStatus;
}


/***********************************************************************
 * ThreeD_RotateInternal
 *  
 *    
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
void ThreeD_RotateInternal(PINTERNAL_3D pInternal3d, PTD_POINT pPoint, PTD_ROTATION pLocalRotation)
{
   TD_POINT Point;
   
   if(pLocalRotation->x)
   {
        Point.y = ROTATE_X_Y(pPoint, pLocalRotation->x);
        Point.z = ROTATE_X_Z(pPoint, pLocalRotation->x);

        pPoint->y = Point.y;
        pPoint->z = Point.z;
   }
   
   if(pLocalRotation->y)
   {
        Point.x = ROTATE_Y_X(pPoint, pLocalRotation->y);
        Point.z = ROTATE_Y_Z(pPoint, pLocalRotation->y);

        pPoint->x = Point.x;
        pPoint->z = Point.z;
   }
   
   if(pLocalRotation->z)
   {
        Point.x = ROTATE_Z_X(pPoint, pLocalRotation->z);
        Point.y = ROTATE_Z_Y(pPoint, pLocalRotation->z);

        pPoint->x = Point.x;
        pPoint->y = Point.y;   
   }
   
}



/***********************************************************************
 * ThreeD_DrawObjectWire
 *  
 *    
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
void ThreeD_DrawObjectWire(H3D h3D, PTHREE_D_OBJECT pThreeDObject, BOOL bBackfaceRemoval)
{
     PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
     TD_POINT_2D TdPointA;
     TD_POINT_2D TdPointB;
     TD_POINT_2D TdPointC;
     TD_POINT TdCameraA;
     TD_POINT TdCameraB;
     TD_POINT TdCameraC;
     TD_POINT TdRotatedA;
     TD_POINT TdRotatedB;
     TD_POINT TdRotatedC;	 
     UINT Index;
     
     for(Index = 0; Index < pThreeDObject->NumberOfFaces; Index++)
     {
         TdRotatedA = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[0]];
         TdRotatedB = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[1]];
         TdRotatedC = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[2]];
         
         ThreeD_RotateInternal(pInternal3d, &TdRotatedA, &pThreeDObject->LocalRotation);
         ThreeD_RotateInternal(pInternal3d, &TdRotatedB, &pThreeDObject->LocalRotation);
         ThreeD_RotateInternal(pInternal3d, &TdRotatedC, &pThreeDObject->LocalRotation);		 
         
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedA, &pThreeDObject->WorldCoordinates, &TdPointA, &TdCameraA);
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedB, &pThreeDObject->WorldCoordinates, &TdPointB, &TdCameraB);
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedC, &pThreeDObject->WorldCoordinates, &TdPointC, &TdCameraC);
         
         if(bBackfaceRemoval == FALSE || ThreeD_IsFaceVisible(pInternal3d, &TdCameraA, &TdCameraB, &TdCameraC))
         {
            ThreeD_DrawLineInternal(pInternal3d, &TdPointA, &TdPointB, &TdCameraA, &TdCameraB, pThreeDObject->pTriFaces[Index].Color.PixelColor);
            ThreeD_DrawLineInternal(pInternal3d, &TdPointB, &TdPointC, &TdCameraB, &TdCameraC, pThreeDObject->pTriFaces[Index].Color.PixelColor);
            ThreeD_DrawLineInternal(pInternal3d, &TdPointC, &TdPointA, &TdCameraC, &TdCameraA, pThreeDObject->pTriFaces[Index].Color.PixelColor);
         }
     }
}


/***********************************************************************
 * ThreeD_DrawObjectSolid
 *  
 *    
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
void ThreeD_DrawObjectSolid(H3D h3D, PTHREE_D_OBJECT pThreeDObject, BOOL bBackfaceRemoval)
{
     PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
     TD_POINT_2D TdPointA;
     TD_POINT_2D TdPointB;
     TD_POINT_2D TdPointC;
     TD_POINT TdCameraA;
     TD_POINT TdCameraB;
     TD_POINT TdCameraC;
     TD_POINT TdRotatedA;
     TD_POINT TdRotatedB;
     TD_POINT TdRotatedC;	 
     UINT Index;
     PTD_POINT_2D pLowestY;
     PTD_POINT_2D pHighestY;
     PTD_POINT_2D pMidY;
     PTD_POINT pLowCamera;
     PTD_POINT pMidCamera;
     PTD_POINT pHighCamera;	 
     TD_POINT OtherCamera;
     TD_POINT_2D  OtherPoint;
     double YDistance;
     
     for(Index = 0; Index < pThreeDObject->NumberOfFaces; Index++)
     {
         TdRotatedA = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[0]];
         TdRotatedB = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[1]];
         TdRotatedC = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[2]];
         
         ThreeD_RotateInternal(pInternal3d, &TdRotatedA, &pThreeDObject->LocalRotation);
         ThreeD_RotateInternal(pInternal3d, &TdRotatedB, &pThreeDObject->LocalRotation);
         ThreeD_RotateInternal(pInternal3d, &TdRotatedC, &pThreeDObject->LocalRotation);		 
         
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedA, &pThreeDObject->WorldCoordinates, &TdPointA, &TdCameraA);
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedB, &pThreeDObject->WorldCoordinates, &TdPointB, &TdCameraB);
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedC, &pThreeDObject->WorldCoordinates, &TdPointC, &TdCameraC);
         
           
         
         if(bBackfaceRemoval == FALSE || ThreeD_IsFaceVisible(pInternal3d, &TdCameraA, &TdCameraB, &TdCameraC))
         {
            if(TdPointA.y == TdPointB.y && TdPointA.y == TdPointC.y)
            {
                ThreeD_DrawLineInternal(pInternal3d, &TdPointA, &TdPointB, &TdCameraA, &TdCameraB, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                ThreeD_DrawLineInternal(pInternal3d, &TdPointB, &TdPointC, &TdCameraB, &TdCameraC, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                ThreeD_DrawLineInternal(pInternal3d, &TdPointC, &TdPointA, &TdCameraC, &TdCameraA, pThreeDObject->pTriFaces[Index].Color.PixelColor);			
            }
            else
            {			
                if(TdPointA.y == TdPointB.y)
                {
                    ThreeD_DrawFlatToporBottomTriangle(pInternal3d, &TdPointA, &TdPointB, &TdPointC, &TdCameraA, &TdCameraB, &TdCameraC, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                }
                else
                {
                    if(TdPointA.y == TdPointC.y)
                    {
                        ThreeD_DrawFlatToporBottomTriangle(pInternal3d, &TdPointA, &TdPointC, &TdPointB, &TdCameraA, &TdCameraC, &TdCameraB, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                    }
                    else
                    {
                        if(TdPointB.y == TdPointC.y)
                        {
                            ThreeD_DrawFlatToporBottomTriangle(pInternal3d, &TdPointB, &TdPointC, &TdPointA, &TdCameraB, &TdCameraC, &TdCameraA, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                        }
                        else
                        {
                           if(TdPointA.y < TdPointC.y && TdPointA.y < TdPointB.y)
                           {
                               pLowestY = &TdPointA;
                               pLowCamera = &TdCameraA;
                           }
                           
                           if(TdPointB.y < TdPointC.y && TdPointB.y < TdPointA.y)
                           {
                               pLowestY = &TdPointB;
                               pLowCamera = &TdCameraB;
                           }

                           if(TdPointC.y < TdPointA.y && TdPointC.y < TdPointB.y)
                           {
                               pLowestY = &TdPointC;
                               pLowCamera = &TdCameraC;
                           }						   
                           
                           if(TdPointA.y > TdPointC.y && TdPointA.y > TdPointB.y)
                           {
                               pHighestY = &TdPointA;
                               pHighCamera = &TdCameraA;
                           }
                           
                           if(TdPointB.y > TdPointC.y && TdPointB.y > TdPointA.y)
                           {
                               pHighestY = &TdPointB;
                               pHighCamera = &TdCameraB;
                           }

                           if(TdPointC.y > TdPointA.y && TdPointC.y > TdPointB.y)
                           {
                               pHighestY = &TdPointC;
                               pHighCamera = &TdCameraC;
                           }
                           
                           if(pHighestY != (&TdPointA) && pLowestY != (&TdPointA))
                           {
                               pMidY = &TdPointA;
                               pMidCamera = &TdCameraA;
                           }
                           
                           if(pHighestY != (&TdPointB) && pLowestY != (&TdPointB))
                           {
                               pMidY = &TdPointB;
                               pMidCamera = &TdCameraB;
                           }
                           
                           if(pHighestY != (&TdPointC) && pLowestY != (&TdPointC))
                           {
                               pMidY = &TdPointC;
                               pMidCamera = &TdCameraC;
                           }

                           YDistance = pHighestY->y - pLowestY->y;
                           
                           //OtherCamera.x = (pHighCamera->x - pLowCamera->x)
                           //OtherCamera.y = pMidCamera->y;
                           OtherPoint.y = pMidY->y;
                           OtherCamera.z = ((pHighCamera->z - pLowCamera->z)/YDistance)*(pMidY->y - pLowestY->y) + pLowCamera->z;
                           OtherPoint.x = (int)(((pHighestY->x - pLowestY->x)/YDistance)*(pMidY->y - pLowestY->y) + pLowestY->x);
                           
                           //ThreeD_Debug(" High(%i, %i) Mid(%i, %i) Low(%i, %i)\n", pHighestY->x, pHighestY->y, pMidY->x, pMidY->y, pLowestY->x, pLowestY->y);                 
                           //ThreeD_Debug(" Generated (%i, %i)\n", OtherPoint.y, OtherPoint.x);                 
                           ThreeD_DrawFlatToporBottomTriangle(pInternal3d, pMidY, &OtherPoint, pHighestY, pMidCamera, &OtherCamera, pHighCamera, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                           ThreeD_DrawFlatToporBottomTriangle(pInternal3d, pMidY, &OtherPoint, pLowestY, pMidCamera, &OtherCamera, pLowCamera, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                         //  ThreeD_PlotPixel2DConverted(pInternal3d, pHighestY->x, pHighestY->y, 1, 0xFFFFFF);
                          // ThreeD_PlotPixel2DConverted(pInternal3d, pMidY->x, pMidY->y, 1, 0xFFFFFF);
                          // ThreeD_PlotPixel2DConverted(pInternal3d, pLowestY->x, pLowestY->y, 1, 0xFFFFFF);
                         //  ThreeD_PlotPixel2DConverted(pInternal3d, OtherPoint.x, OtherPoint.y, 1, 0xFF0000);
                        }						
                    }
                }
            }

         }
     }
}


/***********************************************************************
 * ThreeD_DrawObjectTexture
 *  
 *    
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
void ThreeD_DrawObjectTexture(H3D h3D, PTHREE_D_OBJECT pThreeDObject, BOOL bBackfaceRemoval)
{
     PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
     TD_POINT_2D TdPointA;
     TD_POINT_2D TdPointB;
     TD_POINT_2D TdPointC;
     TD_POINT TdCameraA;
     TD_POINT TdCameraB;
     TD_POINT TdCameraC;
     TD_POINT TdRotatedA;
     TD_POINT TdRotatedB;
     TD_POINT TdRotatedC;	 
     UINT Index;
     PTD_POINT_2D pLowestY;
     PTD_POINT_2D pHighestY;
     PTD_POINT_2D pMidY;
     PTD_POINT pLowCamera;
     PTD_POINT pMidCamera;
     PTD_POINT pHighCamera;	 
     TD_POINT OtherCamera;
     TD_POINT_2D  OtherPoint;
     double YDistance;
     TEXTURE_INFO TextureInfo;
     TEXTURE_INFO TextureInfo2;
     
     
     for(Index = 0; Index < pThreeDObject->NumberOfFaces; Index++)
     {
         TextureInfo.pImageData = pThreeDObject->pImages[pThreeDObject->pTriFaces[Index].ImageIndex];
         TextureInfo.ImageHeightWidth = pThreeDObject->pTdImageSizes[pThreeDObject->pTriFaces[Index].ImageIndex];
         
         TdRotatedA = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[0]];
         TdRotatedB = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[1]];
         TdRotatedC = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[2]];
         
         ThreeD_RotateInternal(pInternal3d, &TdRotatedA, &pThreeDObject->LocalRotation);
         ThreeD_RotateInternal(pInternal3d, &TdRotatedB, &pThreeDObject->LocalRotation);
         ThreeD_RotateInternal(pInternal3d, &TdRotatedC, &pThreeDObject->LocalRotation);		 
         
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedA, &pThreeDObject->WorldCoordinates, &TdPointA, &TdCameraA);
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedB, &pThreeDObject->WorldCoordinates, &TdPointB, &TdCameraB);
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedC, &pThreeDObject->WorldCoordinates, &TdPointC, &TdCameraC);
         
        // ThreeD_Debug(" \n\n*****************************************************************\n", Index);                 
        // ThreeD_Debug(" Face %i\n", Index);      
         
         if(bBackfaceRemoval == FALSE || ThreeD_IsFaceVisible(pInternal3d, &TdCameraA, &TdCameraB, &TdCameraC))
         {
            if(TdPointA.y == TdPointB.y && TdPointA.y == TdPointC.y)
            {
                ThreeD_DrawLineInternal(pInternal3d, &TdPointA, &TdPointB, &TdCameraA, &TdCameraB, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                ThreeD_DrawLineInternal(pInternal3d, &TdPointB, &TdPointC, &TdCameraB, &TdCameraC, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                ThreeD_DrawLineInternal(pInternal3d, &TdPointC, &TdPointA, &TdCameraC, &TdCameraA, pThreeDObject->pTriFaces[Index].Color.PixelColor);			
            }
            else
            {			
                if(TdPointA.y == TdPointB.y)
                {
                    TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                    TextureInfo.ImageVertexes[1] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                    TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                    
                    ThreeD_DrawFlatToporBottomTriangleTexture(pInternal3d, &TdPointA, &TdPointB, &TdPointC, &TdCameraA, &TdCameraB, &TdCameraC, &TextureInfo);
                }
                else
                {
                    if(TdPointA.y == TdPointC.y)
                    {
                        TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                        TextureInfo.ImageVertexes[1] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                        TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];					
                        ThreeD_DrawFlatToporBottomTriangleTexture(pInternal3d, &TdPointA, &TdPointC, &TdPointB, &TdCameraA, &TdCameraC, &TdCameraB, &TextureInfo);
                    }
                    else
                    {
                        if(TdPointB.y == TdPointC.y)
                        {
                            TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                            TextureInfo.ImageVertexes[1] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                            TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];						
                            ThreeD_DrawFlatToporBottomTriangleTexture(pInternal3d, &TdPointB, &TdPointC, &TdPointA, &TdCameraB, &TdCameraC, &TdCameraA, &TextureInfo);
                        }
                        else
                        {
                        
                           TextureInfo2 = TextureInfo;
                           if(TdPointA.y < TdPointC.y && TdPointA.y < TdPointB.y)
                           {
                               TextureInfo2.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];						   
                               pLowestY = &TdPointA;
                               pLowCamera = &TdCameraA;
                           }
                           
                           if(TdPointB.y < TdPointC.y && TdPointB.y < TdPointA.y)
                           {
                               TextureInfo2.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];						   						   
                               pLowestY = &TdPointB;
                               pLowCamera = &TdCameraB;
                           }

                           if(TdPointC.y < TdPointA.y && TdPointC.y < TdPointB.y)
                           {
                               TextureInfo2.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];						   						   						   
                               pLowestY = &TdPointC;
                               pLowCamera = &TdCameraC;
                           }						   
                           
                           if(TdPointA.y > TdPointC.y && TdPointA.y > TdPointB.y)
                           {
                               TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];							   
                               pHighestY = &TdPointA;
                               pHighCamera = &TdCameraA;
                           }
                           
                           if(TdPointB.y > TdPointC.y && TdPointB.y > TdPointA.y)
                           {
                               TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];							   						   
                               pHighestY = &TdPointB;
                               pHighCamera = &TdCameraB;
                           }

                           if(TdPointC.y > TdPointA.y && TdPointC.y > TdPointB.y)
                           {
                               TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];							   						   
                               pHighestY = &TdPointC;
                               pHighCamera = &TdCameraC;
                           }
                           
                           if(pHighestY != (&TdPointA) && pLowestY != (&TdPointA))
                           {
                               TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                               TextureInfo2.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                               pMidY = &TdPointA;
                               pMidCamera = &TdCameraA;
                           }
                           
                           if(pHighestY != (&TdPointB) && pLowestY != (&TdPointB))
                           {
                               TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                               TextureInfo2.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                               pMidY = &TdPointB;
                               pMidCamera = &TdCameraB;
                           }
                           
                           if(pHighestY != (&TdPointC) && pLowestY != (&TdPointC))
                           {
                               TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                               TextureInfo2.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                               pMidY = &TdPointC;
                               pMidCamera = &TdCameraC;
                           }

                           YDistance = pHighestY->y - pLowestY->y;
                           
                           //OtherCamera.x = (pHighCamera->x - pLowCamera->x)
                           //OtherCamera.y = pMidCamera->y;
                           OtherPoint.y = pMidY->y;
                           OtherCamera.z = ((pHighCamera->z - pLowCamera->z)/YDistance)*(pMidY->y - pLowestY->y) + pLowCamera->z;
                           OtherPoint.x = (int)(((pHighestY->x - pLowestY->x)/YDistance)*(pMidY->y - pLowestY->y) + pLowestY->x);
                           
                            TextureInfo.ImageVertexes[1].x  = (int)(((double)(TextureInfo.ImageVertexes[2].x - TextureInfo2.ImageVertexes[2].x)/YDistance)*(pMidY->y - pLowestY->y) + TextureInfo2.ImageVertexes[2].x);
                            TextureInfo.ImageVertexes[1].y  = (int)(((double)(TextureInfo.ImageVertexes[2].y - TextureInfo2.ImageVertexes[2].y)/YDistance)*(pMidY->y - pLowestY->y) + TextureInfo2.ImageVertexes[2].y);
                            TextureInfo2.ImageVertexes[1] = TextureInfo.ImageVertexes[1];
                           
                           //ThreeD_Debug(" High(%i, %i) Mid(%i, %i) Low(%i, %i)\n", pHighestY->x, pHighestY->y, pMidY->x, pMidY->y, pLowestY->x, pLowestY->y);                 
                           //ThreeD_Debug(" Generated (%i, %i)\n", OtherPoint.y, OtherPoint.x);                 
                           ThreeD_DrawFlatToporBottomTriangleTexture(pInternal3d, pMidY, &OtherPoint, pHighestY, pMidCamera, &OtherCamera, pHighCamera, &TextureInfo);
                           ThreeD_DrawFlatToporBottomTriangleTexture(pInternal3d, pMidY, &OtherPoint, pLowestY, pMidCamera, &OtherCamera, pLowCamera, &TextureInfo2);
                         //  ThreeD_PlotPixel2DConverted(pInternal3d, pHighestY->x, pHighestY->y, 1, 0xFFFFFF);
                          // ThreeD_PlotPixel2DConverted(pInternal3d, pMidY->x, pMidY->y, 1, 0xFFFFFF);
                          // ThreeD_PlotPixel2DConverted(pInternal3d, pLowestY->x, pLowestY->y, 1, 0xFFFFFF);
                         //  ThreeD_PlotPixel2DConverted(pInternal3d, OtherPoint.x, OtherPoint.y, 1, 0xFF0000);
                        }						
                    }
                }
            }

         }
     }
}




/***********************************************************************
 * ThreeD_DrawObjectTextureEx
 *  
 *    
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
void ThreeD_DrawObjectTextureEx(H3D h3D, PTHREE_D_OBJECT pThreeDObject, BOOL bBackfaceRemoval)
{
     PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
     TD_POINT_2D TdPointA;
     TD_POINT_2D TdPointB;
     TD_POINT_2D TdPointC;
     TD_POINT TdCameraA;
     TD_POINT TdCameraB;
     TD_POINT TdCameraC;
     TD_POINT TdRotatedA;
     TD_POINT TdRotatedB;
     TD_POINT TdRotatedC;	 
     UINT Index;
     PTD_POINT_2D pLowestY;
     PTD_POINT_2D pHighestY;
     PTD_POINT_2D pMidY;
     PTD_POINT pLowCamera;
     PTD_POINT pMidCamera;
     PTD_POINT pHighCamera;	 
     TD_POINT OtherCamera;
     TD_POINT_2D  OtherPoint;
     double YDistance;
     TEXTURE_INFO TextureInfo;
     TEXTURE_INFO TextureInfo2;
     
     
     for(Index = 0; Index < pThreeDObject->NumberOfFaces; Index++)
     {
         TextureInfo.pImageData = pThreeDObject->pImages[pThreeDObject->pTriFaces[Index].ImageIndex];
         TextureInfo.ImageHeightWidth = pThreeDObject->pTdImageSizes[pThreeDObject->pTriFaces[Index].ImageIndex];
         
         TdRotatedA = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[0]];
         TdRotatedB = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[1]];
         TdRotatedC = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[2]];
         
         ThreeD_RotateInternal(pInternal3d, &TdRotatedA, &pThreeDObject->LocalRotation);
         ThreeD_RotateInternal(pInternal3d, &TdRotatedB, &pThreeDObject->LocalRotation);
         ThreeD_RotateInternal(pInternal3d, &TdRotatedC, &pThreeDObject->LocalRotation);		 
         
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedA, &pThreeDObject->WorldCoordinates, &TdPointA, &TdCameraA);
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedB, &pThreeDObject->WorldCoordinates, &TdPointB, &TdCameraB);
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedC, &pThreeDObject->WorldCoordinates, &TdPointC, &TdCameraC);
         
        // ThreeD_Debug(" \n\n*****************************************************************\n", Index);                 
        // ThreeD_Debug(" Face %i\n", Index);      
         
         if(bBackfaceRemoval == FALSE || ThreeD_IsFaceVisible(pInternal3d, &TdCameraA, &TdCameraB, &TdCameraC))
         {
            if(TdPointA.y == TdPointB.y && TdPointA.y == TdPointC.y)
            {
                ThreeD_DrawLineInternal(pInternal3d, &TdPointA, &TdPointB, &TdCameraA, &TdCameraB, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                ThreeD_DrawLineInternal(pInternal3d, &TdPointB, &TdPointC, &TdCameraB, &TdCameraC, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                ThreeD_DrawLineInternal(pInternal3d, &TdPointC, &TdPointA, &TdCameraC, &TdCameraA, pThreeDObject->pTriFaces[Index].Color.PixelColor);			
            }
            else
            {			
                if(TdPointA.y == TdPointB.y)
                {
                    TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                    TextureInfo.ImageVertexes[1] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                    TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                    
                    ThreeD_DrawFlatToporBottomTriangleTextureEx(pInternal3d, &TdCameraA, &TdCameraB, &TdCameraC, &TextureInfo);
                }
                else
                {
                    if(TdPointA.y == TdPointC.y)
                    {
                        TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                        TextureInfo.ImageVertexes[1] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                        TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];					
                        ThreeD_DrawFlatToporBottomTriangleTextureEx(pInternal3d, &TdCameraA, &TdCameraC, &TdCameraB, &TextureInfo);
                    }
                    else
                    {
                        if(TdPointB.y == TdPointC.y)
                        {
                            TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                            TextureInfo.ImageVertexes[1] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                            TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];						
                            ThreeD_DrawFlatToporBottomTriangleTextureEx(pInternal3d, &TdCameraB, &TdCameraC, &TdCameraA, &TextureInfo);
                        }
                        else
                        {
                        
                           TextureInfo2 = TextureInfo;
                           if(TdPointA.y < TdPointC.y && TdPointA.y < TdPointB.y)
                           {
                               TextureInfo2.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];						   
                               pLowestY = &TdPointA;
                               pLowCamera = &TdCameraA;
                           }
                           
                           if(TdPointB.y < TdPointC.y && TdPointB.y < TdPointA.y)
                           {
                               TextureInfo2.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];						   						   
                               pLowestY = &TdPointB;
                               pLowCamera = &TdCameraB;
                           }

                           if(TdPointC.y < TdPointA.y && TdPointC.y < TdPointB.y)
                           {
                               TextureInfo2.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];						   						   						   
                               pLowestY = &TdPointC;
                               pLowCamera = &TdCameraC;
                           }						   
                           
                           if(TdPointA.y > TdPointC.y && TdPointA.y > TdPointB.y)
                           {
                               TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];							   
                               pHighestY = &TdPointA;
                               pHighCamera = &TdCameraA;
                           }
                           
                           if(TdPointB.y > TdPointC.y && TdPointB.y > TdPointA.y)
                           {
                               TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];							   						   
                               pHighestY = &TdPointB;
                               pHighCamera = &TdCameraB;
                           }

                           if(TdPointC.y > TdPointA.y && TdPointC.y > TdPointB.y)
                           {
                               TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];							   						   
                               pHighestY = &TdPointC;
                               pHighCamera = &TdCameraC;
                           }
                           
                           if(pHighestY != (&TdPointA) && pLowestY != (&TdPointA))
                           {
                               TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                               TextureInfo2.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                               pMidY = &TdPointA;
                               pMidCamera = &TdCameraA;
                           }
                           
                           if(pHighestY != (&TdPointB) && pLowestY != (&TdPointB))
                           {
                               TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                               TextureInfo2.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                               pMidY = &TdPointB;
                               pMidCamera = &TdCameraB;
                           }
                           
                           if(pHighestY != (&TdPointC) && pLowestY != (&TdPointC))
                           {
                               TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                               TextureInfo2.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                               pMidY = &TdPointC;
                               pMidCamera = &TdCameraC;
                           }

                           YDistance = pHighestY->y - pLowestY->y;
                           
                           //OtherCamera.x = (pHighCamera->x - pLowCamera->x)
                           //OtherCamera.y = pMidCamera->y;
                           OtherPoint.y = pMidY->y;
                           OtherCamera.z = ((pHighCamera->z - pLowCamera->z)/YDistance)*(pMidY->y - pLowestY->y) + pLowCamera->z;
                           
                           OtherCamera.x = ((pHighCamera->x - pLowCamera->x)/YDistance)*(pMidY->y - pLowestY->y) + pLowCamera->x;
                           OtherCamera.y = ((pHighCamera->y - pLowCamera->y)/YDistance)*(pMidY->y - pLowestY->y) + pLowCamera->y;
                           
                           OtherPoint.x = (int)(((pHighestY->x - pLowestY->x)/YDistance)*(pMidY->y - pLowestY->y) + pLowestY->x);
                           
                            TextureInfo.ImageVertexes[1].x  = (int)(((double)(TextureInfo.ImageVertexes[2].x - TextureInfo2.ImageVertexes[2].x)/YDistance)*(pMidY->y - pLowestY->y) + TextureInfo2.ImageVertexes[2].x);
                            TextureInfo.ImageVertexes[1].y  = (int)(((double)(TextureInfo.ImageVertexes[2].y - TextureInfo2.ImageVertexes[2].y)/YDistance)*(pMidY->y - pLowestY->y) + TextureInfo2.ImageVertexes[2].y);
                            TextureInfo2.ImageVertexes[1] = TextureInfo.ImageVertexes[1];
                           
                           //ThreeD_Debug(" High(%i, %i) Mid(%i, %i) Low(%i, %i)\n", pHighestY->x, pHighestY->y, pMidY->x, pMidY->y, pLowestY->x, pLowestY->y);                 
                           //ThreeD_Debug(" Generated (%i, %i)\n", OtherPoint.y, OtherPoint.x);                 
                             ThreeD_DrawFlatToporBottomTriangleTextureEx(pInternal3d, pMidCamera, &OtherCamera, pHighCamera, &TextureInfo);
                             ThreeD_DrawFlatToporBottomTriangleTextureEx(pInternal3d, pMidCamera, &OtherCamera, pLowCamera, &TextureInfo2);
                         //  ThreeD_PlotPixel2DConverted(pInternal3d, pHighestY->x, pHighestY->y, 1, 0xFFFFFF);
                          // ThreeD_PlotPixel2DConverted(pInternal3d, pMidY->x, pMidY->y, 1, 0xFFFFFF);
                          // ThreeD_PlotPixel2DConverted(pInternal3d, pLowestY->x, pLowestY->y, 1, 0xFFFFFF);
                         //  ThreeD_PlotPixel2DConverted(pInternal3d, OtherPoint.x, OtherPoint.y, 1, 0xFF0000);
                        }						
                    }
                }
            }

         }
     }
}

/***********************************************************************
 * ThreeD_DrawObjectTexture
 *  
 *    
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
void ThreeD_DrawObjectTextureEx2(H3D h3D, PTHREE_D_OBJECT pThreeDObject, BOOL bBackfaceRemoval)
{
     PINTERNAL_3D pInternal3d = (PINTERNAL_3D)h3D;
     TD_POINT_2D TdPointA;
     TD_POINT_2D TdPointB;
     TD_POINT_2D TdPointC;
     TD_POINT TdCameraA;
     TD_POINT TdCameraB;
     TD_POINT TdCameraC;
     TD_POINT TdRotatedA;
     TD_POINT TdRotatedB;
     TD_POINT TdRotatedC;	 
     UINT Index;
     PTD_POINT_2D pLowestY;
     PTD_POINT_2D pHighestY;
     PTD_POINT_2D pMidY;
     PTD_POINT pLowCamera;
     PTD_POINT pMidCamera;
     PTD_POINT pHighCamera;	 
     TD_POINT OtherCamera;
     TD_POINT_2D  OtherPoint;
     double YDistance;
     TEXTURE_INFO TextureInfo;
     TEXTURE_INFO TextureInfo2;
     
     
     for(Index = 0; Index < pThreeDObject->NumberOfFaces; Index++)
     {
         TextureInfo.pImageData = pThreeDObject->pImages[pThreeDObject->pTriFaces[Index].ImageIndex];
         TextureInfo.ImageHeightWidth = pThreeDObject->pTdImageSizes[pThreeDObject->pTriFaces[Index].ImageIndex];
         
         TdRotatedA = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[0]];
         TdRotatedB = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[1]];
         TdRotatedC = pThreeDObject->pVertices[pThreeDObject->pTriFaces[Index].VertexIndex[2]];
         
         ThreeD_RotateInternal(pInternal3d, &TdRotatedA, &pThreeDObject->LocalRotation);
         ThreeD_RotateInternal(pInternal3d, &TdRotatedB, &pThreeDObject->LocalRotation);
         ThreeD_RotateInternal(pInternal3d, &TdRotatedC, &pThreeDObject->LocalRotation);		 
         
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedA, &pThreeDObject->WorldCoordinates, &TdPointA, &TdCameraA);
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedB, &pThreeDObject->WorldCoordinates, &TdPointB, &TdCameraB);
         ThreeD_Convert3Dto2D(pInternal3d, &TdRotatedC, &pThreeDObject->WorldCoordinates, &TdPointC, &TdCameraC);
         
        // ThreeD_Debug(" \n\n*****************************************************************\n", Index);                 
        // ThreeD_Debug(" Face %i\n", Index);      
         
         if(bBackfaceRemoval == FALSE || ThreeD_IsFaceVisible(pInternal3d, &TdCameraA, &TdCameraB, &TdCameraC))
         {
            if(TdPointA.y == TdPointB.y && TdPointA.y == TdPointC.y)
            {
                ThreeD_DrawLineInternal(pInternal3d, &TdPointA, &TdPointB, &TdCameraA, &TdCameraB, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                ThreeD_DrawLineInternal(pInternal3d, &TdPointB, &TdPointC, &TdCameraB, &TdCameraC, pThreeDObject->pTriFaces[Index].Color.PixelColor);
                ThreeD_DrawLineInternal(pInternal3d, &TdPointC, &TdPointA, &TdCameraC, &TdCameraA, pThreeDObject->pTriFaces[Index].Color.PixelColor);			
            }
            else
            {			
                if(TdPointA.y == TdPointB.y)
                {
                    TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                    TextureInfo.ImageVertexes[1] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                    TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                    
                    ThreeD_DrawFlatToporBottomTriangleTextureEx2(pInternal3d, &TdPointA, &TdPointB, &TdPointC, &TdCameraA, &TdCameraB, &TdCameraC, &TextureInfo);
                }
                else
                {
                    if(TdPointA.y == TdPointC.y)
                    {
                        TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                        TextureInfo.ImageVertexes[1] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                        TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];					
                        ThreeD_DrawFlatToporBottomTriangleTextureEx2(pInternal3d, &TdPointA, &TdPointC, &TdPointB, &TdCameraA, &TdCameraC, &TdCameraB, &TextureInfo);
                    }
                    else
                    {
                        if(TdPointB.y == TdPointC.y)
                        {
                            TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                            TextureInfo.ImageVertexes[1] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                            TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];						
                            ThreeD_DrawFlatToporBottomTriangleTextureEx2(pInternal3d, &TdPointB, &TdPointC, &TdPointA, &TdCameraB, &TdCameraC, &TdCameraA, &TextureInfo);
                        }
                        else
                        {
                        
                           TextureInfo2 = TextureInfo;
                           if(TdPointA.y < TdPointC.y && TdPointA.y < TdPointB.y)
                           {
                               TextureInfo2.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];						   
                               pLowestY = &TdPointA;
                               pLowCamera = &TdCameraA;
                           }
                           
                           if(TdPointB.y < TdPointC.y && TdPointB.y < TdPointA.y)
                           {
                               TextureInfo2.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];						   						   
                               pLowestY = &TdPointB;
                               pLowCamera = &TdCameraB;
                           }

                           if(TdPointC.y < TdPointA.y && TdPointC.y < TdPointB.y)
                           {
                               TextureInfo2.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];						   						   						   
                               pLowestY = &TdPointC;
                               pLowCamera = &TdCameraC;
                           }						   
                           
                           if(TdPointA.y > TdPointC.y && TdPointA.y > TdPointB.y)
                           {
                               TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];							   
                               pHighestY = &TdPointA;
                               pHighCamera = &TdCameraA;
                           }
                           
                           if(TdPointB.y > TdPointC.y && TdPointB.y > TdPointA.y)
                           {
                               TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];							   						   
                               pHighestY = &TdPointB;
                               pHighCamera = &TdCameraB;
                           }

                           if(TdPointC.y > TdPointA.y && TdPointC.y > TdPointB.y)
                           {
                               TextureInfo.ImageVertexes[2] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];							   						   
                               pHighestY = &TdPointC;
                               pHighCamera = &TdCameraC;
                           }
                           
                           if(pHighestY != (&TdPointA) && pLowestY != (&TdPointA))
                           {
                               TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                               TextureInfo2.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[0];
                               pMidY = &TdPointA;
                               pMidCamera = &TdCameraA;
                           }
                           
                           if(pHighestY != (&TdPointB) && pLowestY != (&TdPointB))
                           {
                               TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                               TextureInfo2.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[1];
                               pMidY = &TdPointB;
                               pMidCamera = &TdCameraB;
                           }
                           
                           if(pHighestY != (&TdPointC) && pLowestY != (&TdPointC))
                           {
                               TextureInfo.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                               TextureInfo2.ImageVertexes[0] = pThreeDObject->pTriFaces[Index].ImageVertexes[2];
                               pMidY = &TdPointC;
                               pMidCamera = &TdCameraC;
                           }

                           YDistance = pHighestY->y - pLowestY->y;
                           
                           //OtherCamera.x = (pHighCamera->x - pLowCamera->x)
                           //OtherCamera.y = pMidCamera->y;
                           OtherPoint.y = pMidY->y;
                           OtherCamera.z = ((pHighCamera->z - pLowCamera->z)/YDistance)*(pMidY->y - pLowestY->y) + pLowCamera->z;
                           
                           //OtherCamera.x = ((pHighCamera->x - pLowCamera->x)/YDistance)*(pMidY->y - pLowestY->y) + pLowCamera->x;
                           //OtherCamera.y = ((pHighCamera->y - pLowCamera->y)/YDistance)*(pMidY->y - pLowestY->y) + pLowCamera->y;
                           
                           OtherPoint.x = (int)(((pHighestY->x - pLowestY->x)/YDistance)*(pMidY->y - pLowestY->y) + pLowestY->x);
                           
                           ThreeD_Get3DFrom2D(pInternal3d, &OtherCamera, &OtherPoint);
                           
                           
                           //ThreeD_Debug(" High(%i, %i) Mid(%i, %i) Low(%i, %i)\n", pHighestY->x, pHighestY->y, pMidY->x, pMidY->y, pLowestY->x, pLowestY->y);                 
                           //ThreeD_Debug("Working\n");                 
                           //ThreeD_DrawFlatToporBottomTriangleTextureEx(pInternal3d, pMidCamera, &OtherCamera, pHighCamera, &TextureInfo);
                           //ThreeD_Debug("Not Working\n");                 
                           
                           if(pMidY->x < OtherPoint.x)
                           {
                                TextureInfo.ImageVertexes[1].x  = (int)(((double)(TextureInfo.ImageVertexes[2].x - TextureInfo2.ImageVertexes[2].x)/YDistance)*(pMidY->y - pLowestY->y) + TextureInfo2.ImageVertexes[2].x);
                                TextureInfo.ImageVertexes[1].y  = (int)(((double)(TextureInfo.ImageVertexes[2].y - TextureInfo2.ImageVertexes[2].y)/YDistance)*(pMidY->y - pLowestY->y) + TextureInfo2.ImageVertexes[2].y);
                                TextureInfo2.ImageVertexes[1] = TextureInfo.ImageVertexes[1];
                           
                                ThreeD_DrawFlatToporBottomTriangleTextureEx2(pInternal3d, pMidY, &OtherPoint, pHighestY, pMidCamera, &OtherCamera, pHighCamera, &TextureInfo);
                                ThreeD_DrawFlatToporBottomTriangleTextureEx2(pInternal3d, pMidY, &OtherPoint, pLowestY, pMidCamera, &OtherCamera, pLowCamera, &TextureInfo2);
                           }
                           else
                           {
                                TextureInfo.ImageVertexes[1]  = TextureInfo.ImageVertexes[0];
                                TextureInfo2.ImageVertexes[1] = TextureInfo.ImageVertexes[0];
                                
                           
                                TextureInfo.ImageVertexes[0].x  = (int)(((double)(TextureInfo.ImageVertexes[2].x - TextureInfo2.ImageVertexes[2].x)/YDistance)*(pMidY->y - pLowestY->y) + TextureInfo2.ImageVertexes[2].x);
                                TextureInfo.ImageVertexes[0].y  = (int)(((double)(TextureInfo.ImageVertexes[2].y - TextureInfo2.ImageVertexes[2].y)/YDistance)*(pMidY->y - pLowestY->y) + TextureInfo2.ImageVertexes[2].y);
                                TextureInfo2.ImageVertexes[0] = TextureInfo.ImageVertexes[0];
                           
                                ThreeD_DrawFlatToporBottomTriangleTextureEx2(pInternal3d, &OtherPoint, pMidY, pHighestY, &OtherCamera, pMidCamera, pHighCamera, &TextureInfo);
                                ThreeD_DrawFlatToporBottomTriangleTextureEx2(pInternal3d, &OtherPoint, pMidY, pLowestY, &OtherCamera, pMidCamera, pLowCamera, &TextureInfo2);
                           }
                         //  ThreeD_PlotPixel2DConverted(pInternal3d, pHighestY->x, pHighestY->y, 1, 0xFFFFFF);
                          // ThreeD_PlotPixel2DConverted(pInternal3d, pMidY->x, pMidY->y, 1, 0xFFFFFF);
                          // ThreeD_PlotPixel2DConverted(pInternal3d, pLowestY->x, pLowestY->y, 1, 0xFFFFFF);
                         //  ThreeD_PlotPixel2DConverted(pInternal3d, OtherPoint.x, OtherPoint.y, 1, 0xFF0000);
                        }						
                    }
                }
            }

         }
     }
}
/***********************************************************************
 * ThreeD_DrawLineInternal
 *  
 *    
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
void ThreeD_DrawLineInternal(PINTERNAL_3D pInternal3d, PTD_POINT_2D pTdPointA, PTD_POINT_2D pTdPointB, PTD_POINT pTdCameraA, PTD_POINT pTdCameraB, DWORD PixelColor)
{
     double StartX;
     double StartY;
     double StartZ;
     double X_Increment;
     double Y_Increment;
     double Z_Increment;
     double DistanceX;
     double DistanceY;
     double DistanceZ;
     
     if(pTdCameraA->z > 0 && pTdCameraB->z > 0)
     {
         X_Increment = pTdPointA->x - pTdPointB->x;
         Y_Increment = pTdPointA->y - pTdPointB->y;
         Z_Increment = pTdCameraA->z - pTdCameraB->z;
         
         DistanceY = ABS(Y_Increment);
         DistanceX = ABS(X_Increment);
          
         if(DistanceX == 0)
         {
              if(Y_Increment < 0)
              {
                 Y_Increment = 1;
              }
              else
              {
                 Y_Increment = -1;
              }
              
              Z_Increment = Z_Increment / DistanceY*-1;
               
              StartZ = pTdCameraA->z;
              for(StartY = pTdPointA->y; DistanceY-- && StartZ > 0; StartY += Y_Increment, StartZ += Z_Increment)
              {
                  ThreeD_PlotPixel2DConverted(pInternal3d, pTdPointA->x, (int)StartY, StartZ, PixelColor);
              }
         }
         else
         {
             if(DistanceY == 0)
             {
                  if(X_Increment < 0)
                  {
                     X_Increment = 1;
                  }
                  else
                  {
                     X_Increment = -1;
                  }

                  Z_Increment = Z_Increment / DistanceX*-1;			  
                  
                  StartZ = pTdCameraA->z;
                  for(StartX = pTdPointA->x; DistanceX-- && StartZ > 0; StartX += X_Increment, StartZ += Z_Increment)
                  {
                      ThreeD_PlotPixel2DConverted(pInternal3d, (int)StartX, pTdPointA->y, StartZ, PixelColor);
                  }
             }
             else
             {
                    if(DistanceY > DistanceX)
                    {
                    
                          if(Y_Increment < 0)
                          {
                             Y_Increment = 1;
                          }
                          else
                          {
                             Y_Increment = -1;
                          }
                          
                          X_Increment = (X_Increment / DistanceY)*-1;
                          Z_Increment = Z_Increment / DistanceY*-1;
                          
                          StartZ = pTdCameraA->z;
                          for(StartY = pTdPointA->y, StartX = pTdPointA->x; DistanceY-- && StartZ > 0; StartY += Y_Increment, StartZ += Z_Increment, StartX += X_Increment)
                          {
                              ThreeD_PlotPixel2DConverted(pInternal3d, (int)StartX, (int)StartY, StartZ, PixelColor);
                          }					
                    }
                    else
                    {
                          if(X_Increment < 0)
                          {
                             X_Increment = 1;
                          }
                          else
                          {
                             X_Increment = -1;
                          }
                          Y_Increment = (Y_Increment / DistanceX)*-1;
                          Z_Increment = Z_Increment / DistanceX*-1;
                          
                          StartZ = pTdCameraA->z;
                          for(StartY = pTdPointA->y, StartX = pTdPointA->x; DistanceX-- && StartZ > 0; StartY += Y_Increment, StartZ += Z_Increment, StartX += X_Increment)
                          {
                              ThreeD_PlotPixel2DConverted(pInternal3d, (int)StartX, (int)StartY, StartZ, PixelColor);
                          }							
                    }
             }
         }
    }
}


/***********************************************************************
 * ThreeD_Direction
 *  
 *    
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
BOOL ThreeD_IsFaceVisible(PINTERNAL_3D pInternal3d, PTD_POINT pPointA, PTD_POINT pPointB, PTD_POINT pPointC)
{
   BOOL FaceIsVisible;
   double VectorALength;
   double VectorBlength;
   double DotProduct;
   TD_POINT VectorA;
   TD_POINT VectorB;
   TD_POINT NormalVector;
   
   FaceIsVisible = TRUE;
   
  // ThreeD_Debug("PointA(%f, %f, %f) PointB(%f, %f, %f) PointC(%f, %f, %f)\n", pPointA->x, pPointA->y, pPointA->z, pPointB->x, pPointB->y, pPointB->z, pPointC->x, pPointC->y, pPointC->z);
   
   VectorA.x = pPointB->x - pPointA->x;
   VectorA.y = pPointB->y - pPointA->y;
   VectorA.z = pPointB->z - pPointA->z;
   
   VectorALength = sqrt(VectorA.x*VectorA.x + VectorA.y*VectorA.y + VectorA.z*VectorA.z);

   VectorA.x /= VectorALength;
   VectorA.y /= VectorALength;
   VectorA.z /= VectorALength;
   
   VectorB.x = pPointC->x - pPointA->x;
   VectorB.y = pPointC->y - pPointA->y;
   VectorB.z = pPointC->z - pPointA->z;   
   
   VectorBlength = sqrt(VectorB.x*VectorB.x + VectorB.y*VectorB.y + VectorB.z*VectorB.z);

   VectorB.x /= VectorBlength;
   VectorB.y /= VectorBlength;
   VectorB.z /= VectorBlength;   
   
 //  ThreeD_Debug(" VectorA(%f, %f, %f) VectorB(%f, %f, %f)\n", VectorA.x, VectorA.y, VectorA.z, VectorB.x, VectorB.y, VectorB.z);   
   
   NormalVector.x =  ((VectorA.y * VectorB.z) - (VectorA.z * VectorB.y));
   NormalVector.y = -1*((VectorA.x * VectorB.z) - (VectorA.z * VectorB.x));
   NormalVector.z =  (VectorA.x * VectorB.y) - (VectorA.y * VectorB.x);
   
  // ThreeD_Debug(" Normal(%f, %f, %f)\n", NormalVector.x, NormalVector.y, NormalVector.z);  
  
   DotProduct = NormalVector.x*pPointA->x + NormalVector.y*pPointA->y + NormalVector.z*pPointA->z;
      
   if(DotProduct >= 0)
   {
      FaceIsVisible = FALSE;
   }
   
   return FaceIsVisible;
}

/***********************************************************************
 * ThreeD_DrawFlatToporBottomTriangle
 *  
 *    Debug Shit
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
void ThreeD_DrawFlatToporBottomTriangle(PINTERNAL_3D pInternal3d, PTD_POINT_2D pTdPointA, PTD_POINT_2D pTdPointB, PTD_POINT_2D pTdPointC, PTD_POINT pTdCameraA, PTD_POINT pTdCameraB, PTD_POINT pTdCameraC, DWORD PixelColor)
{
     double StartX;
     double StartZ;
     double X_Increment;
     double Z_Increment;
     double DistanceX;
     double CurrentStartX;
     double CurrentEndX;
     double CurrentStartZ;
     double CurrentEndZ;
     double StartXChange;
     double EndXChange;
     double StartZChange;
     double EndZChange;
     int NumberOfLines;
     int Index;
     
     if(pTdCameraA->z > 0 && pTdCameraB->z > 0 && pTdCameraC->z > 0 && (pTdPointA->y == pTdPointB->y && pTdPointA->y != pTdPointC->y))
     {	 
     
          //ThreeD_Debug(" Point A(%i, %i) Point B(%i, %i) Point C(%i, %i)\n", pTdPointA->x, pTdPointA->y, pTdPointB->x, pTdPointB->y, pTdPointC->x, pTdPointC->y);
          
          CurrentStartX = pTdPointA->x;
          CurrentEndX   = pTdPointB->x;
          
          CurrentStartZ = pTdCameraA->z;
          CurrentEndZ   = pTdCameraB->z;
          NumberOfLines = pTdPointC->y - pTdPointA->y;
          
          StartXChange  = ((double)pTdPointC->x - (double)pTdPointA->x)/(double)ABS(NumberOfLines);
          EndXChange    = ((double)pTdPointC->x - (double)pTdPointB->x)/(double)ABS(NumberOfLines);
          
          
          StartZChange  = (pTdCameraC->z - pTdCameraA->z)/(double)ABS(NumberOfLines);
          EndZChange    = (pTdCameraC->z - pTdCameraB->z)/(double)ABS(NumberOfLines);
          
          
         // ThreeD_Debug(" StartXChange(%f) EndXChange (%f)\n", StartXChange, EndXChange);
          
          for(Index = 0; Index != NumberOfLines; )
          {
              X_Increment = CurrentStartX - CurrentEndX;
              Z_Increment = CurrentStartZ - CurrentEndZ;
              
              DistanceX = ABS(X_Increment);
             
              if(X_Increment < 0)
              {
                 X_Increment = 1;
              }
              else
              {
                 X_Increment = -1;
              }

              Z_Increment = Z_Increment / DistanceX*-1;			  
              
              StartZ = pTdCameraA->z;
              
              //ThreeD_Debug(" CurrentStartX(%f) CurrentEndX (%f) X Increment (%f) Distance(%f)\n", CurrentStartX, CurrentEndX, X_Increment, DistanceX);
              
              for(StartX = CurrentStartX; DistanceX > 0; StartX += X_Increment, StartZ += Z_Increment)
              {
                  DistanceX--;
                  ThreeD_PlotPixel2DConverted(pInternal3d, (int)StartX, pTdPointA->y + Index, StartZ, PixelColor);
              }
              
              ThreeD_PlotPixel2DConverted(pInternal3d, (int)StartX, pTdPointA->y + Index, StartZ, PixelColor);

              CurrentStartX += StartXChange;
              CurrentEndX += EndXChange;
              CurrentStartZ += StartZChange;
              CurrentEndZ += EndZChange;
              
              if(NumberOfLines < 0)
              {
                 Index--;
              }
              else
              {
                 Index++;
              }
          }
    }
}




   
   
/***********************************************************************
 * ThreeD_DrawFlatToporBottomTriangleTexture
 *  
 *    Debug Shit
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
void ThreeD_DrawFlatToporBottomTriangleTexture(PINTERNAL_3D pInternal3d, PTD_POINT_2D pTdPointA, PTD_POINT_2D pTdPointB, PTD_POINT_2D pTdPointC, PTD_POINT pTdCameraA, PTD_POINT pTdCameraB, PTD_POINT pTdCameraC, PPTEXTURE_INFO pTextureInfo)
{
     double StartX;
     double StartZ;
     double X_Increment;
     double Z_Increment;
     double DistanceX;
     double CurrentStartX;
     double CurrentEndX;
     double CurrentStartZ;
     double CurrentEndZ;
     double StartXChange;
     double EndXChange;
     double StartZChange;
     double EndZChange;
     double ImageStartX;
     double ImageLoopX;
     double ImageLoopY;
     double ImageEndX;
     double ImageEndY;
     double ImageLoopXInc;
     double ImageLoopYInc;
     double ImageIncrmentX_End;
     double ImageIncrmentY_End;
     double ImageStartY;
     double ImageIncrementX;
     double ImageIncrementY;
     DWORD PixelColor = 0xFFFFFF;
     int NumberOfLines;
     int Index;
     
     /*for(Index = 0; Index < 3; Index++)
     {
        ThreeD_Debug(" ImageVertexes[%i].x = %i \n", Index, pTextureInfo->ImageVertexes[Index].x);
        ThreeD_Debug(" ImageVertexes[%i].y = %i \n", Index, pTextureInfo->ImageVertexes[Index].y);
    }*/
     
     if(pTdCameraA->z > 0 && pTdCameraB->z > 0 && pTdCameraC->z > 0 && (pTdPointA->y == pTdPointB->y && pTdPointA->y != pTdPointC->y))
     {	 
     
         // ThreeD_Debug(" Point A(%i, %i) Point B(%i, %i) Point C(%i, %i)\n", pTdPointA->x, pTdPointA->y, pTdPointB->x, pTdPointB->y, pTdPointC->x, pTdPointC->y);

          ImageStartX = pTextureInfo->ImageVertexes[0].x;
          ImageStartY = pTextureInfo->ImageVertexes[0].y;
          
          ImageEndX = pTextureInfo->ImageVertexes[1].x;
          ImageEndY = pTextureInfo->ImageVertexes[1].y;		  
                  
                  
          CurrentStartX = pTdPointA->x;
          CurrentEndX   = pTdPointB->x;
          
          CurrentStartZ = pTdCameraA->z;
          CurrentEndZ   = pTdCameraB->z;
          NumberOfLines = pTdPointC->y - pTdPointA->y;
          
          StartXChange  = ((double)pTdPointC->x - (double)pTdPointA->x)/(double)ABS(NumberOfLines);
          EndXChange    = ((double)pTdPointC->x - (double)pTdPointB->x)/(double)ABS(NumberOfLines);
          
          StartZChange  = (pTdCameraC->z - pTdCameraA->z)/(double)ABS(NumberOfLines);
          EndZChange    = (pTdCameraC->z - pTdCameraB->z)/(double)ABS(NumberOfLines);
          
          ImageIncrementX  = ((double)pTextureInfo->ImageVertexes[2].x - (double)pTextureInfo->ImageVertexes[0].x)/(double)ABS(NumberOfLines);
          ImageIncrementY  = ((double)pTextureInfo->ImageVertexes[2].y - (double)pTextureInfo->ImageVertexes[0].y)/(double)ABS(NumberOfLines);
          
          ImageIncrmentX_End  = ((double)pTextureInfo->ImageVertexes[2].x - (double)pTextureInfo->ImageVertexes[1].x)/(double)ABS(NumberOfLines);		  
          ImageIncrmentY_End  = ((double)pTextureInfo->ImageVertexes[2].y - (double)pTextureInfo->ImageVertexes[1].y)/(double)ABS(NumberOfLines);
          
          //ThreeD_Debug(" ImageStartX(%f) ImageEndX (%f) ImageStartY (%f) ImageEndY(%f) \n", ImageStartX, ImageEndX, ImageStartY,ImageEndY );
          //ThreeD_Debug(" EndPointY(%i) EndPointX (%i) \n", pTextureInfo->ImageVertexes[2].y, pTextureInfo->ImageVertexes[2].x);
          //ThreeD_Debug(" ImageIncrementX(%f) ImageIncrementY (%f) ImageIncrmentX_End (%f) ImageIncrmentY_End(%f) \n", ImageIncrementX, ImageIncrementY, ImageIncrmentX_End, ImageIncrmentY_End );
          
          for(Index = 0; Index != NumberOfLines; )
          {
              X_Increment = CurrentStartX - CurrentEndX;
              Z_Increment = CurrentStartZ - CurrentEndZ;
              
              DistanceX = ABS(X_Increment);
             
              if(X_Increment < 0)
              {
                 X_Increment = 1;
              }
              else
              {
                 X_Increment = -1;
              }

              Z_Increment = Z_Increment / DistanceX*-1;			  
              
              StartZ = pTdCameraA->z;
              
              
              ImageLoopX = ImageStartX;
              ImageLoopY = ImageStartY;
              ImageLoopXInc = (ImageEndX - ImageStartX)/DistanceX;
              ImageLoopYInc = (ImageEndY - ImageStartY)/DistanceX;
              //ThreeD_Debug(" ImageStartX(%f) ImageStartY (%f) ImageEndX (%f) ImageEndY(%f)\n", ImageStartX, ImageStartY, ImageEndX, ImageEndY);
              //ThreeD_Debug(" ImageLoopXInc(%f) ImageLoopYInc (%f) \n", ImageLoopXInc, ImageLoopYInc);
              for(StartX = CurrentStartX; DistanceX > 0; StartX += X_Increment, StartZ += Z_Increment)
              {
                  DistanceX--;
                  if(ImageLoopX > 0 && ImageLoopY > 0 && ImageLoopY < pTextureInfo->ImageHeightWidth.y && ImageLoopX < pTextureInfo->ImageHeightWidth.x)
                  {
                      PixelColor = ((DWORD *)pTextureInfo->pImageData)[(int)ImageLoopX + ((int)ImageLoopY*pTextureInfo->ImageHeightWidth.x)];
                  }
                  else
                  {
                      // Use Last Color
                  }
                  
                  ThreeD_PlotPixel2DConverted(pInternal3d, (int)StartX, pTdPointA->y + Index, StartZ, PixelColor);
                  ImageLoopX += ImageLoopXInc;
                  ImageLoopY += ImageLoopYInc;
              }
              
              if(ImageLoopX > 0 && ImageLoopY > 0 && ImageLoopY < pTextureInfo->ImageHeightWidth.y && ImageLoopX < pTextureInfo->ImageHeightWidth.x)
              {
                  PixelColor = ((DWORD *)pTextureInfo->pImageData)[(int)ImageLoopX + ((int)ImageLoopY*pTextureInfo->ImageHeightWidth.x)];
              }
              else
              {
                  // Use Last Color
              }
              
              ThreeD_PlotPixel2DConverted(pInternal3d, (int)StartX, pTdPointA->y + Index, StartZ, PixelColor);

              ImageStartX += ImageIncrementX;
              ImageStartY += ImageIncrementY;
              ImageEndX += ImageIncrmentX_End;
              ImageEndY += ImageIncrmentY_End;			  
              CurrentStartX += StartXChange;
              CurrentEndX += EndXChange;
              CurrentStartZ += StartZChange;
              CurrentEndZ += EndZChange;
              
              if(NumberOfLines < 0)
              {
                 Index--;
              }
              else
              {
                 Index++;
              }
          }
          
          //ThreeD_Debug(" DONE:::: ImageStartX(%f) ImageEndX (%f) ImageStartY (%f) ImageEndY(%f) \n", ImageStartX, ImageEndX, ImageStartY,ImageEndY );
    }
}


/***********************************************************************
 * ThreeD_DrawFlatToporBottomTriangleTextureEx
 *  
 *    Debug Shit
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
void ThreeD_DrawFlatToporBottomTriangleTextureEx(PINTERNAL_3D pInternal3d, PTD_POINT pTdCameraA, PTD_POINT pTdCameraB, PTD_POINT pTdCameraC, PPTEXTURE_INFO pTextureInfo)
{
     TD_POINT CameraConverted;
     TD_POINT CameraConvertedEnd;
     double X_Increment;
     double Y_Increment;
     double Z_Increment;
     double CurrentStartX;
     double CurrentEndX;
     double CurrentStartY;
     double CurrentEndY;	 
     double CurrentStartZ;
     double CurrentEndZ;
     double StartXChange;
     double EndXChange;
     double StartZChange;
     double StartYChange;
     double EndYChange;
     double EndZChange;
     double ImageStartX;
     double ImageLoopX;
     double ImageLoopY;
     double ImageEndX;
     double ImageEndY;
     double ImageLoopXInc;
     double ImageLoopYInc;
     double ImageIncrmentX_End;
     double ImageIncrmentY_End;
     double ImageStartY;
     double ImageIncrementX;
     double ImageIncrementY;
     double LineLength;
     double InnerNumberOfLines;
     TD_POINT VectorA;
     TD_POINT VectorB;
     double NumberOfLines;
     DWORD PixelColor = 0xFFFFFF;
     int Index;
     TD_POINT_2D PlottedPointA;
     TD_POINT_2D PlottedPointB;
     
     //if(pTdCameraA->z > 0 && pTdCameraB->z > 0 && pTdCameraC->z > 0)
     {	 
          VectorB.x = pTdCameraC->x - pTdCameraA->x; 
          VectorB.y = pTdCameraC->y - pTdCameraA->y;
          VectorB.z = pTdCameraC->z - pTdCameraA->z;
          
         // NumberOfLines = sqrt(VectorB.x*VectorB.x + VectorB.y*VectorB.y + VectorB.z*VectorB.z);
          
          ThreeD_PlotWithCamera(pInternal3d, pTdCameraA, &PlottedPointA, 0);
          ThreeD_PlotWithCamera(pInternal3d, pTdCameraC, &PlottedPointB, 0);
          
          NumberOfLines = PlottedPointB.y - PlottedPointA.y;
          
          NumberOfLines *= 2;
          
          ImageStartX = pTextureInfo->ImageVertexes[0].x;
          ImageStartY = pTextureInfo->ImageVertexes[0].y;
          
          ImageEndX = pTextureInfo->ImageVertexes[1].x;
          ImageEndY = pTextureInfo->ImageVertexes[1].y;	 

          CurrentStartZ = pTdCameraA->z;
          CurrentEndZ   = pTdCameraB->z;
                          
          CurrentStartX = pTdCameraA->x;
          CurrentEndX   = pTdCameraB->x;
          
          CurrentStartY = pTdCameraA->y;
          CurrentEndY   = pTdCameraB->y;
          
          StartXChange  = (pTdCameraC->x - pTdCameraA->x)/ABS(NumberOfLines);
          EndXChange    = (pTdCameraC->x - pTdCameraB->x)/ABS(NumberOfLines);

          StartYChange  = (pTdCameraC->y - pTdCameraA->y)/ABS(NumberOfLines);
          EndYChange    = (pTdCameraC->y - pTdCameraB->y)/ABS(NumberOfLines);
          
          StartZChange  = (pTdCameraC->z - pTdCameraA->z)/ABS(NumberOfLines);
          EndZChange    = (pTdCameraC->z - pTdCameraB->z)/ABS(NumberOfLines);
          
          ImageIncrementX  = ((double)pTextureInfo->ImageVertexes[2].x - (double)pTextureInfo->ImageVertexes[0].x)/(double)ABS(NumberOfLines);
          ImageIncrementY  = ((double)pTextureInfo->ImageVertexes[2].y - (double)pTextureInfo->ImageVertexes[0].y)/(double)ABS(NumberOfLines);
          
          ImageIncrmentX_End  = ((double)pTextureInfo->ImageVertexes[2].x - (double)pTextureInfo->ImageVertexes[1].x)/(double)ABS(NumberOfLines);		  
          ImageIncrmentY_End  = ((double)pTextureInfo->ImageVertexes[2].y - (double)pTextureInfo->ImageVertexes[1].y)/(double)ABS(NumberOfLines);
          
         // ThreeD_Debug("0, %i, %i, %f, %f, %f, %f, W\n",PlottedPointA.y, PlottedPointB.y, ImageStartX, ImageStartY, ImageEndX, ImageEndY);
          
          for(Index = 0; Index != (int)NumberOfLines; )
          {
              VectorA.x = CurrentEndX - CurrentStartX; 
              VectorA.y = CurrentEndY - CurrentStartY;
              VectorA.z = CurrentEndZ - CurrentStartZ;	
              CameraConvertedEnd.x = CurrentEndX;
              CameraConvertedEnd.y = CurrentEndY;
              CameraConvertedEnd.z = CurrentEndZ;
              CameraConverted.z = CurrentStartZ;
              CameraConverted.x = CurrentStartX;
              CameraConverted.y = CurrentStartY;

             // InnerNumberOfLines = sqrt(VectorA.x*VectorA.x + VectorA.y*VectorA.y + VectorA.z*VectorA.z);			  
              
              ThreeD_PlotWithCamera(pInternal3d, &CameraConverted, &PlottedPointA, 0);
              ThreeD_PlotWithCamera(pInternal3d, &CameraConvertedEnd, &PlottedPointB, 0);
              
              InnerNumberOfLines =  PlottedPointB.x - PlottedPointA.x;
              InnerNumberOfLines = ABS(InnerNumberOfLines)*2;

              X_Increment = VectorA.x / InnerNumberOfLines;		
              Y_Increment = VectorA.y / InnerNumberOfLines;		
              Z_Increment = VectorA.z / InnerNumberOfLines;
              
              
              ImageLoopX = ImageStartX;
              ImageLoopY = ImageStartY;
              ImageLoopXInc = (ImageEndX - ImageStartX)/InnerNumberOfLines;
              ImageLoopYInc = (ImageEndY - ImageStartY)/InnerNumberOfLines;
              
    
              for(; (int)InnerNumberOfLines > 0; CameraConverted.x += X_Increment, CameraConverted.y  += Y_Increment, CameraConverted.z += Z_Increment)
              {
                  if(ImageLoopX > 0 && ImageLoopY > 0 && ImageLoopY < pTextureInfo->ImageHeightWidth.y && ImageLoopX < pTextureInfo->ImageHeightWidth.x)
                  {
                      PixelColor = ((DWORD *)pTextureInfo->pImageData)[(int)ImageLoopX + ((int)ImageLoopY*pTextureInfo->ImageHeightWidth.x)];
                  }
                  else
                  {
                      // Use Last Color
                  }
                  
                  ThreeD_PlotWithCamera(pInternal3d, &CameraConverted, &PlottedPointA, PixelColor);
                  
                //  ThreeD_Debug("1, %i, %i, %f, %f, %f, %f, %i, %i, Working \n", (int)PlottedPointA.x, (int)PlottedPointA.y, CameraConverted.z, CameraConverted.x, CameraConverted.y, CameraConverted.z, (int)ImageLoopX, (int)ImageLoopY);
                  
                  ImageLoopX += ImageLoopXInc;
                  ImageLoopY += ImageLoopYInc;
                  InnerNumberOfLines--;
              }
              
              if(ImageLoopX > 0 && ImageLoopY > 0 && ImageLoopY < pTextureInfo->ImageHeightWidth.y && ImageLoopX < pTextureInfo->ImageHeightWidth.x)
              {
                  PixelColor = ((DWORD *)pTextureInfo->pImageData)[(int)ImageLoopX + ((int)ImageLoopY*pTextureInfo->ImageHeightWidth.x)];
              }
              else
              {
                  // Use Last Color
              }
              
              ThreeD_PlotWithCamera(pInternal3d, &CameraConverted, &PlottedPointA, PixelColor);

              ImageStartX += ImageIncrementX;
              ImageStartY += ImageIncrementY;
              ImageEndX += ImageIncrmentX_End;
              ImageEndY += ImageIncrmentY_End;			  
              CurrentStartX += StartXChange;
              CurrentEndX += EndXChange;
              CurrentStartZ += StartZChange;
              CurrentEndZ += EndZChange;
              CurrentStartY += StartYChange;
              CurrentEndY += EndYChange;
              
              if((int)NumberOfLines < 0)
              {
                 Index--;
              }
              else
              {
                 Index++;
              }
          }
          
          //ThreeD_Debug(" DONE:::: ImageStartX(%f) ImageEndX (%f) ImageStartY (%f) ImageEndY(%f) \n", ImageStartX, ImageEndX, ImageStartY,ImageEndY );
    }
}

/***********************************************************************
 * ThreeD_DrawFlatToporBottomTriangleTextureEx2
 *  
 *    Debug Shit
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
void ThreeD_DrawFlatToporBottomTriangleTextureEx2(PINTERNAL_3D pInternal3d, PTD_POINT_2D pTdPointA, PTD_POINT_2D pTdPointB, PTD_POINT_2D pTdPointC, PTD_POINT pTdCameraA, PTD_POINT pTdCameraB, PTD_POINT pTdCameraC, PPTEXTURE_INFO pTextureInfo)
{
     TD_POINT Projection2DA;
     TD_POINT Projection2DA_Change;
     
     TD_POINT Projection2DB;
     TD_POINT Projection2DB_Change;
     
     TD_POINT Projection3DA_Change;
     TD_POINT Projection3DB_Change;

     TD_POINT CurrentItterationProjection;
     TD_POINT_2D CurrentItterationProjection2D;
     
     TD_POINT ImagePointA;
     TD_POINT ImagePointB;
     TD_POINT ImagePointA_Change;
     TD_POINT ImagePointB_Change;
     
     TD_POINT CurrentCamera;
     
     TD_POINT LineChange;

     int NumberOfHorizontalLines2D;
     int NumberOfLines3DA_X;
     int NumberOfLines3DA_Y;
     
     int NumberOfLines3DB_X;
     int NumberOfLines3DB_Y;
     
     int NumberOfLines3D;
     
     TD_POINT_2D ImageLocation;
     
     int Index;
     int ItterationsOnX;
     
     double CameraItterationsEndX;
     double CameraItterationsEndY;
     double CameraItterationsStartY;
     double CameraItterationsStartX;

     double CameraPercentageX;
     double CameraPercentageY;
     
     TD_POINT StartPoint;
     TD_POINT EndPoint;

     TD_POINT StartPointCamera;
     TD_POINT EndPointCamera;
     BOOL bFirstLine;
     
     DWORD PixelColor = 0;
     bFirstLine = TRUE;
     
     if(pTdCameraA->z > 0 && pTdCameraB->z > 0 && pTdCameraC->z > 0 && (pTdPointA->y == pTdPointB->y && pTdPointA->y != pTdPointC->y))
     {	 
          ThreeD_Debug(" Point A(%i, %i) Point B(%i, %i) Point C(%i, %i)\n", pTdPointA->x, pTdPointA->y, pTdPointB->x, pTdPointB->y, pTdPointC->x, pTdPointC->y);

          NumberOfHorizontalLines2D = (pTdPointC->y - pTdPointA->y);
         
          /*
           * Set-up the 2D projection starting points at PointA and PointB.
           * Then, set up the interval from A to C and B to C.
           */
          Projection2DA.x        = pTdPointA->x;
          Projection2DA.y        = pTdPointA->y;
          Projection2DA.z        = pTdCameraA->z;
          
          Projection2DB.x        = pTdPointB->x;
          Projection2DB.y        = pTdPointB->y;
          Projection2DB.z        = pTdCameraB->z;
          
          Projection2DA_Change.x =  ((double)pTdPointC->x - (double)pTdPointA->x)/(double)ABS(NumberOfHorizontalLines2D);
          Projection2DA_Change.y =  ((double)pTdPointC->y - (double)pTdPointA->y)/(double)ABS(NumberOfHorizontalLines2D);
          Projection2DA_Change.z =  ((double)pTdCameraC->z - (double)pTdCameraA->z)/(double)ABS(NumberOfHorizontalLines2D);
          
          Projection2DB_Change.x =  ((double)pTdPointC->x - (double)pTdPointB->x)/(double)ABS(NumberOfHorizontalLines2D);
          Projection2DB_Change.y =  ((double)pTdPointC->y - (double)pTdPointB->y)/(double)ABS(NumberOfHorizontalLines2D);
          Projection2DB_Change.z =  ((double)pTdCameraC->z - (double)pTdCameraB->z)/(double)ABS(NumberOfHorizontalLines2D);
                  
          /*
           * Set-up the Image projections in 3D.
           * Then, set up the interval from A to C and B to C.
           * We need to do this with the camera because we need to plot in 3D.
           */
          ImagePointA.x        = pTextureInfo->ImageVertexes[0].x;
          ImagePointA.y        = pTextureInfo->ImageVertexes[0].y;
          ImagePointA.z        = 0;
          
          ImagePointB.x        = pTextureInfo->ImageVertexes[1].x;
          ImagePointB.y        = pTextureInfo->ImageVertexes[1].y;
          ImagePointB.z        = 0;
          
          Projection3DA_Change.x =  (pTdCameraC->x - pTdCameraA->x)/(double)ABS(NumberOfHorizontalLines2D);
          Projection3DA_Change.y =  (pTdCameraC->y - pTdCameraA->y)/(double)ABS(NumberOfHorizontalLines2D);
          
          Projection3DB_Change.x =  (pTdCameraC->x - pTdCameraB->x)/(double)ABS(NumberOfHorizontalLines2D);
          Projection3DB_Change.y =  (pTdCameraC->y - pTdCameraB->y)/(double)ABS(NumberOfHorizontalLines2D);
          
          ImagePointA_Change.x =  ((double)pTextureInfo->ImageVertexes[2].x - (double)pTextureInfo->ImageVertexes[0].x)/(double)ABS(NumberOfHorizontalLines2D);
          ImagePointA_Change.y =  ((double)pTextureInfo->ImageVertexes[2].y - (double)pTextureInfo->ImageVertexes[0].y)/(double)ABS(NumberOfHorizontalLines2D);
                  
          ImagePointB_Change.x =  ((double)pTextureInfo->ImageVertexes[2].x - (double)pTextureInfo->ImageVertexes[1].x)/(double)ABS(NumberOfHorizontalLines2D);
          ImagePointB_Change.y =  ((double)pTextureInfo->ImageVertexes[2].y - (double)pTextureInfo->ImageVertexes[1].y)/(double)ABS(NumberOfHorizontalLines2D);
          
          ThreeD_Debug("   ImagePointA_Change(%lf, %lf)\n", ImagePointA_Change.x, ImagePointA_Change.y);	
          ThreeD_Debug("   ImagePointB_Change(%lf, %lf)\n", ImagePointB_Change.x, ImagePointB_Change.y);			  
          ThreeD_Debug("   Projection3DA_Change(%lf, %lf)\n", Projection3DA_Change.x, Projection3DA_Change.y);					   
          ThreeD_Debug("   Projection3DB_Change(%lf, %lf)\n", Projection3DB_Change.x, Projection3DB_Change.y);		
                  
         // ThreeD_Debug("0, %i, %i, %f, %f, %f, %f, NW\n", pTdPointA->y, pTdPointC->y, ImagePointA.x, ImagePointA.y, ImagePointB.x, ImagePointB.y);
          ThreeD_Debug("\n Image A(%f, %f) - B(%f, %f) - C(%i, %i)\n", ImagePointA.x, ImagePointA.y, ImagePointB.x, ImagePointB.y, pTextureInfo->ImageVertexes[2].x, pTextureInfo->ImageVertexes[2].y);					   
         /* ThreeD_Debug(" Camera A(%f, %f) - B(%f, %f) - C(%f, %f)\n", pTdCameraA->x, pTdCameraA->y, pTdCameraB->x, pTdCameraB->y,pTdCameraC->x, pTdCameraC->y);					   
          ThreeD_Debug(" Points A(%i, %i) - B(%i, %i) - C(%i, %i)\n", pTdPointA->x, pTdPointA->y, pTdPointB->x, pTdPointB->y, pTdPointC->x, pTdPointC->y);					   
          ThreeD_Debug("   ImagePointA_Change(%lf, %lf)\n", ImagePointA_Change.x, ImagePointA_Change.y);	
          ThreeD_Debug("   ImagePointB_Change(%lf, %lf)\n", ImagePointB_Change.x, ImagePointB_Change.y);					   
          ThreeD_Debug("   Projection3DA_Change(%lf, %lf)\n", Projection3DA_Change.x, Projection3DA_Change.y);					   
          ThreeD_Debug("   Projection3DB_Change(%lf, %lf)\n", Projection3DB_Change.x, Projection3DB_Change.y);								 
          ThreeD_Debug("   NumberOfHorizontalLines3DA(%lf, %lf)\n", NumberOfLines3DA_X, NumberOfLines3DA_Y);					   
          ThreeD_Debug("   NumberOfHorizontalLines3DB(%lf, %lf)\n", NumberOfLines3DB_X, NumberOfLines3DB_Y);								 */
          
        //  ThreeD_Debug("ThreeD_DrawFlatToporBottomTriangleTextureEx2\n");
          
          /*
           * Start looping through each horizontal line.
           * 
           */		  
          for(Index = 0; Index != NumberOfHorizontalLines2D; )
          {
              ItterationsOnX = (int)(Projection2DB.x - Projection2DA.x);
              LineChange.x   = 1;
              
              if(ItterationsOnX < 0)
              {
                 LineChange.x = -1;
              }
              
              ItterationsOnX = ABS(ItterationsOnX)+1;

              LineChange.z = (Projection2DB.z - Projection2DA.z)/(double)ItterationsOnX;
              
              CurrentItterationProjection.x = Projection2DA.x;
              CurrentItterationProjection.y = Projection2DA.y;
              CurrentItterationProjection.z = Projection2DA.z;
              
              /*
               * Iterations through the line.
               * 
               */		

              while(ItterationsOnX  >= 0)
              {
                  CurrentItterationProjection2D.x = (int)CurrentItterationProjection.x;
                  CurrentItterationProjection2D.y = (int)CurrentItterationProjection.y;
                  CurrentCamera.z = CurrentItterationProjection.z;
                  
                  if(ThreeD_Get3DFrom2D(pInternal3d, &CurrentCamera, &CurrentItterationProjection2D))
                  {
                  if(bFirstLine)
                     ThreeD_Debug(" 2D(%i, %i) 3D(%f, %f, %f) A(%f, %f, %f) B(%f, %f, %f)\n", CurrentItterationProjection2D.x, CurrentItterationProjection2D.y, CurrentCamera.x, CurrentCamera.y, CurrentCamera.z, pTdCameraA->x, pTdCameraA->y, pTdCameraA->z, pTdCameraB->x, pTdCameraB->y, pTdCameraB->z);
                      /*
                       * Synchronize Start and End on Y location.
                       */
                      CameraItterationsStartX = ((CurrentCamera.x -  pTdCameraA->x));
                      CameraItterationsStartY = ((CurrentCamera.y -  pTdCameraA->y));
                      CameraItterationsEndX   = ((CurrentCamera.x -  pTdCameraB->x));
                      CameraItterationsEndY   = ((CurrentCamera.y -  pTdCameraB->y));
                    #if 1
                      if(Projection3DA_Change.y)
                      {
                         CameraItterationsStartY = CameraItterationsStartY / Projection3DA_Change.y;
                      }
                      else
                      {
                         CameraItterationsStartY = 0;
                      }					  
                      
                      if(Projection3DB_Change.y)
                      {
                         CameraItterationsEndY = CameraItterationsEndY / Projection3DB_Change.y;
                      }
                      else
                      {
                         CameraItterationsEndY = 0;
                      }							  
                      
                      //CameraItterationsStartY = ABS(CameraItterationsStartY);
                     // CameraItterationsEndY   = ABS(CameraItterationsEndY);
                      
                      StartPoint.x = ImagePointA.x + (ImagePointA_Change.x*CameraItterationsStartY);
                      StartPoint.y = ImagePointA.y + (ImagePointA_Change.y*CameraItterationsStartY);
                      
                      EndPoint.x = ImagePointB.x + (ImagePointB_Change.x*CameraItterationsEndY);
                      EndPoint.y = ImagePointB.y + (ImagePointB_Change.y*CameraItterationsEndY);
                      
                      StartPointCamera.x = pTdCameraA->x + (Projection3DA_Change.x*CameraItterationsStartY);
                      StartPointCamera.y = pTdCameraA->y + (Projection3DA_Change.y*CameraItterationsStartY);
                      
                      EndPointCamera.x   = pTdCameraB->x + (Projection3DB_Change.x*CameraItterationsEndY);
                      EndPointCamera.y   = pTdCameraB->y + (Projection3DB_Change.y*CameraItterationsEndY); 	  		

                      
                      #else
                      if(Projection3DA_Change.x)
                      {
                         CameraItterationsStartX = CameraItterationsStartX / Projection3DA_Change.x;
                      }
                      else
                      {
                         CameraItterationsStartX = 0;
                      }					  
                      
                      if(Projection3DB_Change.x)
                      {
                         CameraItterationsEndX = CameraItterationsEndX / Projection3DB_Change.x;
                      }
                      else
                      {
                         CameraItterationsEndX = 0;
                      }							  
                      
                      CameraItterationsStartX = ABS(CameraItterationsStartX);
                      CameraItterationsEndX   = ABS(CameraItterationsEndX);
                      
                      StartPoint.x = ImagePointA.x + (ImagePointA_Change.x*CameraItterationsStartX);
                      StartPoint.y = ImagePointA.y + (ImagePointA_Change.y*CameraItterationsStartX);
                      
                      EndPoint.x = ImagePointB.x + (ImagePointB_Change.x*CameraItterationsEndX);
                      EndPoint.y = ImagePointB.y + (ImagePointB_Change.y*CameraItterationsEndX);
                      
                      StartPointCamera.x = pTdCameraA->x + (Projection3DA_Change.x*CameraItterationsStartX);
                      StartPointCamera.y = pTdCameraA->y + (Projection3DA_Change.y*CameraItterationsStartX);
                      
                      EndPointCamera.x   = pTdCameraB->x + (Projection3DB_Change.x*CameraItterationsEndX);
                      EndPointCamera.y   = pTdCameraB->y + (Projection3DB_Change.y*CameraItterationsEndX); 	  						  
                      #endif
                      
                      /* Get Start and End in 2D and use that as interpolation */
                     // ThreeD_Convert3Dto2D(PINTERNAL_3D pInternal3d, PTD_POINT pTdPoint, PTD_POINT pPixelWorld, PTD_POINT_2D pTdPoint2d, PTD_POINT pTdCamera)
                     // ThreeD_Convert3Dto2D(PINTERNAL_3D pInternal3d, PTD_POINT pTdPoint, PTD_POINT pPixelWorld, PTD_POINT_2D pTdPoint2d, PTD_POINT pTdCamera)
                     
                     // Use Y values from Point A to Point B instead of Point B to Point C in some cases.
                      
                      
                      if((EndPoint.x - StartPoint.x) != 0)
                      {
                            CameraPercentageX = ABS((EndPoint.x - StartPoint.x));
                            CameraPercentageX = (EndPointCamera.x - StartPointCamera.x)/CameraPercentageX;
                            
                            if(CameraPercentageX != 0)
                            {
                                ImageLocation.x  = (int)(StartPoint.x + ABS((CurrentCamera.x - StartPointCamera.x))/CameraPercentageX);
                                ImageLocation.x = ABS(ImageLocation.x);
                            }
                            else
                            {
                                ImageLocation.x = (int)StartPoint.x;
                            }
                            
                            CameraPercentageY = ABS((EndPoint.y - StartPoint.y));
                            
                            if(CameraPercentageY)
                            {
                                CameraPercentageY = (EndPointCamera.x - StartPointCamera.x)/CameraPercentageY;							
                            
                                ImageLocation.y  = (int)(StartPoint.y + ABS((CurrentCamera.x - StartPointCamera.x))/CameraPercentageY);
                                ImageLocation.y = ABS(ImageLocation.y);
                            }
                            else
                            {
                                ImageLocation.y = (int)StartPoint.y;
                            }
                            
                      }
                      else
                      {
                            ImageLocation.x = (int)StartPoint.x;
                            ImageLocation.y = (int)StartPoint.y;
                      }
                     // ThreeD_Debug("(%i, %i) - (%f, %f) - (%f, %f) - (%f, %f)\n", (int)ImageLocation.x , (int)ImageLocation.y, CurrentCamera.x, CurrentCamera.y, StartPointCamera.x, StartPointCamera.y, EndPointCamera.x, EndPointCamera.y);
                      /*
                      if((EndPoint.y - StartPoint.y) != 0)
                      {
                            CameraPercentageY = ABS((EndPoint.y - StartPoint.y));
                            CameraPercentageY = ABS((EndPointCamera.y - StartPointCamera.y))/CameraPercentageY;
                            
                            if(CameraPercentageX != 0)
                            {
                                ImageLocation.y  = (int)(StartPoint.y + ABS((CurrentCamera.y - StartPointCamera.y))/CameraPercentageY);
                            }
                            else
                            {
                                ImageLocation.y = (int)StartPoint.y;
                            }

                      }
                      else
                      {
                            ImageLocation.y = (int)StartPoint.y;
                      }	*/				  
                  if(bFirstLine)
                     ThreeD_Debug(" Image(%i, %i)\n\n", ImageLocation.x, ImageLocation.y);
                      
                      if(ImageLocation.x >= 0 && ImageLocation.y >= 0 && ImageLocation.y < pTextureInfo->ImageHeightWidth.y && ImageLocation.x < pTextureInfo->ImageHeightWidth.x)
                      {
                          PixelColor = ((DWORD *)pTextureInfo->pImageData)[(int)ImageLocation.x + ((int)ImageLocation.y*pTextureInfo->ImageHeightWidth.x)];
                      }		
                      else
                      {
                         ThreeD_Debug("Error 1\n");
                      }
                  }
                  else
                  {
                     ThreeD_Debug(" No 3D Projection\n");
                     PixelColor = 0;
                  }
                 
                  ThreeD_PlotPixel2DConverted(pInternal3d, (int)CurrentItterationProjection.x, (int)CurrentItterationProjection.y, CurrentItterationProjection.z, PixelColor);
                  
                  ItterationsOnX--;
                  CurrentItterationProjection.x += LineChange.x;
                  CurrentItterationProjection.z += LineChange.z;
              }
              
              
              Projection2DA.x +=  Projection2DA_Change.x;
              Projection2DA.z +=  Projection2DA_Change.z;
              
              Projection2DB.x +=  Projection2DB_Change.x;
              Projection2DB.z +=  Projection2DB_Change.z;
              
              if(NumberOfHorizontalLines2D < 0)
              {
                 Projection2DB.y--;
                 Projection2DA.y--;
                 Index--;
              }
              else
              {
                 Projection2DA.y++;
                 Projection2DB.y++;
                 Index++;
              }
              bFirstLine = FALSE;

          }
          
          //ThreeD_Debug(" DONE:::: ImageStartX(%f) ImageEndX (%f) ImageStartY (%f) ImageEndY(%f) \n", ImageStartX, ImageEndX, ImageStartY,ImageEndY );
    }
}
/***********************************************************************
 * ThreeD_Debug
 *  
 *    Debug Shit
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
 void ThreeD_Debug(char *pszFormatString, ...)
 {
     char DebugString[256];
     va_list vl;

     va_start(vl, pszFormatString);
     vsprintf(DebugString, pszFormatString, vl);
     va_end(vl);

     OutputDebugStringA(DebugString);
 }

  