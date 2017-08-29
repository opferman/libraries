/*
 * Socket Library
 *
 *    This is a socket wrapping library
 *    which can be used to create servers
 *    and client connections.
 *         
 *  Toby Opferman
 *
 *  Copyright 2005, All Rights Reserved
 *
 */



#ifndef __SOCKET_H__
#define __SOCKET_H__

struct _CLIENT_CONNECTION;                              
typedef struct _CLIENT_CONNECTION *HCLIENT;

struct _SERVER_SOCKET;
typedef struct _SERVER_SOCKET *HSERVER;

 
typedef enum _CONNECTION_EVENT
{
    ConnectionConnected = 0,
    ConnectionDisconnect,
    ConnectionDataAvailable

} CONNECTION_EVENT;

typedef void (*PFNCLIENT)(HCLIENT hClient, CONNECTION_EVENT ConnectionEvent, PVOID pContext);


BOOL Socket_Initialize(void);
void Socket_UnInitialize(void);

HSERVER Socket_CreateServer(PFNCLIENT pfnClient, PVOID pContext, UINT uiPortNumber);
void Socket_CloseServer(HSERVER hServer);

HCLIENT Socket_CreateClientConnection(PFNCLIENT pfnClient, PVOID pContext, PCHAR pServer, UINT uiPortNumber);
HCLIENT Socket_CreateDirectClientConnection(PFNCLIENT pfnClient, PVOID pContext, DWORD dwIpAddress, DWORD uiPortNumber);
void Socket_CloseClientConnection(HCLIENT hClient);

void Socket_SetClientContext(HCLIENT hClient, PVOID pContext);
BOOL Socket_QueryServerInformation(HCLIENT hClient, DWORD *pdwIpAddress, DWORD *pdwPort);

BOOL Socket_SendData(HCLIENT hClient, PVOID pData, UINT uiDataSize);
BOOL Socket_RecvData(HCLIENT hClient, PVOID pData, UINT *puiDataSize);



#endif


