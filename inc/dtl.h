/***************************************************************************** 
 *                         Data Transfer Layer Library                       *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2001           Toby Opferman                              *
 *****************************************************************************/


#ifndef __DTL_H__
#define __DTL_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef PVOID DTL_SOCKET;
typedef enum { DTLSOCKET_NODATA, DTLSOCKET_DATA, DTLSOCKET_ERROR } DTLSOCKET_STATUS ;


#define INVALID_DTL_SOCKET (DTL_SOCKET)NULL
#define DTL_SOCKET_ERROR   SOCKET_ERROR

DTL_SOCKET Dtl_Create_Client(PCHAR Address, USHORT ServerPort, DWORD Flags);
DTL_SOCKET Dtl_Create_Server(UINT Listeners, USHORT Port, DWORD Flags);
DTLSOCKET_STATUS Dtl_SocketWaitForRead(DTL_SOCKET DtlSocket, UINT msTimeOut);
UINT Dtl_SendSocketData(DTL_SOCKET DtlSocket, PCHAR pBuffer, UINT Size);
UINT Dtl_ReadSocketData(DTL_SOCKET DtlSocket, PCHAR pBuffer, UINT Size);
UINT Dtl_GetLastError(void);
void Dtl_Destroy(DTL_SOCKET DtlSocket);


#ifdef __cplusplus
}
#endif


#endif

