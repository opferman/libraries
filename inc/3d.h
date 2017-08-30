#ifndef __3D_H__
#define __3D_H__

/*
 * 3D Macros
 */
#define ROTATE_X_NEW_Y(x, y, z, r)  (cos(r)*y - sin(r)*z)
#define ROTATE_X_NEW_Z(x, y, z, r)  (sin(r)*y + cos(r)*z)
#define ROTATE_X_NEW_X(x, y, z, r)  (x)

#define ROTATE_Y_NEW_X(x, y, z, r)  (cos(r)*x - sin(r)*z)
#define ROTATE_Y_NEW_Z(x, y, z, r)  (sin(r)*x + cos(r)*z)
#define ROTATE_Y_NEW_Y(x, y, z, r)  (y)

#define ROTATE_Z_NEW_X(x, y, z, r)  (cos(r)*x - sin(r)*y)
#define ROTATE_Z_NEW_Y(x, y, z, r)  (sin(r)*x + cos(r)*y)
#define ROTATE_Z_NEW_Z(x, y, z, r)  (z)

#define ROTATE_X_Y(p, r)  (cos(r)*p->y - sin(r)*p->z)
#define ROTATE_X_Z(p, r)  (sin(r)*p->y + cos(r)*p->z)
#define ROTATE_X_X(p, r)  (p->x)

#define ROTATE_Y_X(p, r)  (cos(r)*p->x - sin(r)*p->z)
#define ROTATE_Y_Z(p, r)  (sin(r)*p->x + cos(r)*p->z)
#define ROTATE_Y_Y(p, r)  (p->y)

#define ROTATE_Z_X(p, r)  (cos(r)*p->x - sin(r)*p->y)
#define ROTATE_Z_Y(p, r)  (sin(r)*p->x + cos(r)*p->y)
#define ROTATE_Z_Z(p, r)  (p->z)

#define DEGREES_TO_RAD(deg) (deg*(3.14/180.0))
#define RAD_TO_DEGREES(deg) (deg*(180.0/3.14))

#define ABS(x) (x < 0 ? x*-1 : x)

/*
 * Structures for 3D
 */
typedef union _PIXEL_COLOR {

     struct {
                unsigned char Blue;
                unsigned char Green;
                unsigned char Red;
                unsigned char Alpha;				
     } Color;
     
     DWORD PixelColor;

} PIXEL_COLOR, *PPIXEL_COLOR;


typedef struct _TD_ROTATION {

   double x;
   double y;
   double z;
   
} TD_ROTATION, *PTD_ROTATION;

typedef struct _TD_POINT {

   double x;
   double y;
   double z;
   
} TD_POINT, *PTD_POINT;

typedef struct _TD_POINT_2D {

   int x;
   int y;
   
} TD_POINT_2D, *PTD_POINT_2D;

typedef struct _TD_PIXEL {

   TD_POINT Point;
   PIXEL_COLOR Color;

} TD_PIXEL, *PTD_PIXEL;

#define TRI_FACE_NO_IMAGE  ((UINT)-1) 

typedef struct _TRI_FACE {

   UINT VertexIndex[3];
      
   PIXEL_COLOR Color;
   
   UINT ImageIndex;
   TD_POINT_2D ImageVertexes[3];

} TRI_FACE, *PTRI_FACE;

typedef struct _TEXTURE_INFO {
   TD_POINT_2D ImageVertexes[3];
   TD_POINT_2D ImageHeightWidth;
   char *pImageData;
} TEXTURE_INFO, *PPTEXTURE_INFO;


typedef struct _THREE_D_OBJECT {

   UINT NumberOfVertices;
   PTD_POINT pVertices;
   
   UINT NumberOfFaces;
   PTRI_FACE pTriFaces;
   
   TD_POINT WorldCoordinates;
   TD_ROTATION LocalRotation;
   
   UINT NumberOfImages;
   unsigned char **pImages;
   PTD_POINT_2D pTdImageSizes;
   
} THREE_D_OBJECT, *PTHREE_D_OBJECT;


typedef enum _TD_PIXEL_STATUS
{
    TdPixelPlotted = 0,
    TdPixelOffScreen,
    TdPixelHidden	

} TD_PIXEL_STATUS, *PTD_PIXEL_STATUS;

#define T3D_FLAG_ZBUFFER 1

typedef PVOID H3D;

H3D ThreeD_Init(HGDI hScreenBuffer, ULONG Flags);
void ThreeD_Close(H3D h3D);

ULONG ThreeD_GetScreenHeight(H3D h3D);
ULONG ThreeD_GetScreenWidth(H3D h3D);

void ThreeD_SetViewPoint(H3D h3D, PTD_POINT pViewPoint);
void ThreeD_SetCameraRotation(H3D h3D, double X_Radians, double Y_Radians, double Z_Radians);
void ThreeD_SetViewDistance(H3D h3D, double ViewDistance);

void ThreeD_ClearScreen(H3D h3D);

void ThreeD_DrawLine(H3D h3D, PTD_POINT pTdPointA, PTD_POINT pTdPointB, PPIXEL_COLOR pLineColor, PTD_POINT pPixelWorld);
TD_PIXEL_STATUS ThreeD_PlotPixel(H3D h3D, PTD_PIXEL pTdPixel, PTD_POINT pPixelWorld);
void ThreeD_DrawObjectWire(H3D h3D, PTHREE_D_OBJECT pThreeDObject, BOOL bBackfaceRemoval);
void ThreeD_DrawObjectSolid(H3D h3D, PTHREE_D_OBJECT pThreeDObject, BOOL bBackfaceRemoval);
void ThreeD_DrawObjectTexture(H3D h3D, PTHREE_D_OBJECT pThreeDObject, BOOL bBackfaceRemoval);
void ThreeD_DrawObjectTextureEx(H3D h3D, PTHREE_D_OBJECT pThreeDObject, BOOL bBackfaceRemoval);
void ThreeD_DrawObjectTextureEx2(H3D h3D, PTHREE_D_OBJECT pThreeDObject, BOOL bBackfaceRemoval);

double ThreeD_GetAspectRatio(H3D h3D);
void ThreeD_SetAspectRatio(H3D h3D, double AspectRatio);


 void ThreeD_Debug(char *pszFormatString, ...);
 
#endif

  