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

#include <winsock2.h>
#include <windows.h>
#include <socket.h>



 /*
  * Macros
  */ 
#define WSA_SOCKET_VERSION   2
 
 /*
  * Internal Structures
  */
typedef struct _CLIENT_CONNECTION
{
    PFNCLIENT pfnClient;
    PVOID pContext;

    SOCKET ClientSocket;

    CRITICAL_SECTION csWriteLock;
    CRITICAL_SECTION csReadLock;

    HANDLE hClientThread;
    HANDLE hStopClientEvent;
    WSAEVENT wsaClientEvent;
    DWORD dwThreadId;

    BOOL bClientThreadAlive;
    BOOL bDelayClose;

    DWORD dwIpAddress;
    DWORD dwPort;

} CLIENT_CONNECTION, *HCLIENT;

typedef struct _SERVER_SOCKET
{
  PFNCLIENT pfnClient;
  PVOID pContext;

  SOCKET ServerSocket;

  HANDLE hServerThread;
  HANDLE hStopServerEvent;
  WSAEVENT wsaServerEvent;

  BOOL bServerThreadAlive;

} SERVER_SOCKET, *HSERVER;

 /*
  * Internal Prototypes
  */
 SOCKET Socket_CreateServerSocket(UINT uiPort);
 DWORD WINAPI Socket_ServerThread(PVOID pContext);
 DWORD WINAPI Socket_ClientThread(PVOID pContext);
 BOOL Socket_ResolveServer(PCHAR pServer, UINT uiPort, SOCKADDR_IN *pSockAddr);
 SOCKET Socket_Connect(SOCKADDR_IN *pSockAddr); 
 HCLIENT Socket_CreateClientThread(PFNCLIENT pfnClient, PVOID pContext, SOCKET ClientSocket);
 void Socket_RealClose(HCLIENT hClient);

/***********************************************
 *
 * Socket_Initialize
 *
 * Parameters
 *    None
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
 BOOL Socket_Initialize(void)
 {
     BOOL bRetValue = FALSE;
     WSADATA WsaData = {0};

     if(WSAStartup(WSA_SOCKET_VERSION, &WsaData) == 0)
     {
        bRetValue = TRUE;
     }

     return bRetValue;
 }

/***********************************************
 *
 * Socket_UnInitialize
 *
 * Parameters
 *   None
 *
 * Return Value
 *   None
 ***********************************************/
 void Socket_UnInitialize(void)
 {
     WSACleanup();
 }



/***********************************************
 *
 * Socket_CreateServer
 *
 * Parameters
 *   Client Connection Callback, Data Context, Port Number
 *
 * Return Value
 *   Handle to Server
 ***********************************************/
 HSERVER Socket_CreateServer(PFNCLIENT pfnClient, PVOID pContext, UINT uiPortNumber)
 {
     HSERVER phServer        = NULL;
     HANDLE hStopServerEvent = NULL;
     SOCKET ServerSocket     = 0;
     DWORD dwThreadId;
     WSAEVENT wsaServerEvent;

     hStopServerEvent = CreateEvent(NULL,FALSE, FALSE, NULL);

     if(hStopServerEvent)
     {
         ServerSocket = Socket_CreateServerSocket(uiPortNumber);

         if(ServerSocket)
         {
             wsaServerEvent = WSACreateEvent();

             if(wsaServerEvent)
             {
             
                 phServer = (HSERVER)LocalAlloc(LMEM_ZEROINIT, sizeof(SERVER_SOCKET));
            
                 if(phServer)
                 {
                    phServer->pContext           = pContext;
                    phServer->pfnClient          = pfnClient;
                    phServer->hStopServerEvent   = hStopServerEvent;
                    phServer->ServerSocket       = ServerSocket;
                    phServer->wsaServerEvent     = wsaServerEvent;
                    phServer->bServerThreadAlive = TRUE;
    
                    phServer->hServerThread = CreateThread(NULL, 0, Socket_ServerThread, phServer, 0, &dwThreadId);
    
                    if(phServer->hServerThread == NULL)
                    {
                        LocalFree(phServer);
                        phServer = NULL;
                    }
    
                 }
    
                 if(phServer == NULL)
                 {
                    WSACloseEvent(wsaServerEvent);
                    wsaServerEvent = NULL;
                 }
             }

             if(wsaServerEvent == NULL)
             {
                 closesocket(ServerSocket);
                 ServerSocket = 0;
             }
         }

         if(ServerSocket == 0)
         {
            CloseHandle(hStopServerEvent);
            hStopServerEvent = NULL;
         }
     }

     return phServer;
 }


/***********************************************
 *
 * Socket_ServerThread
 *
 *   This thread will accept incomming
 *   connections.
 *
 * Parameters
 *   Server Context
 *
 * Return Value
 *   0
 ***********************************************/
 DWORD WINAPI Socket_ServerThread(PVOID pContext)
 {
    HSERVER phServer = (HSERVER)pContext;
    HANDLE hWaitEvents[2] ={0};
    DWORD dwWaitEventNumber;
    SOCKADDR_IN SockAddr = {0};
    SOCKET ClientSocket;
    int iReturnSize;

    while(phServer->bServerThreadAlive)
    {
        WSAResetEvent(phServer->wsaServerEvent);
        WSAEventSelect(phServer->ServerSocket, phServer->wsaServerEvent, FD_ACCEPT);
        
        hWaitEvents[0] = phServer->wsaServerEvent;
        hWaitEvents[1] = phServer->hStopServerEvent;

        dwWaitEventNumber = WaitForMultipleObjects(2, hWaitEvents, FALSE, INFINITE);

        if(dwWaitEventNumber == WAIT_OBJECT_0)
        {
           iReturnSize = sizeof(SockAddr); 
           ClientSocket = accept(phServer->ServerSocket, (struct sockaddr *)&SockAddr, &iReturnSize);

           if(ClientSocket)
           {
               if(Socket_CreateClientThread(phServer->pfnClient, phServer->pContext, ClientSocket) == NULL)
               {
                  closesocket(ClientSocket);
               }   
           }
        }
    }

    return 0;
 }

/***********************************************
 *
 * Socket_ServerThread
 *
 *   This thread will implement communications
 *   between a client and server.
 *
 * Parameters
 *   Client Context
 *
 * Return Value
 *   0
 ***********************************************/
 DWORD WINAPI Socket_ClientThread(PVOID pContext)
 {
   HCLIENT phClient = (HCLIENT)pContext;
   HANDLE hWaitEvents[2] ={0};
   DWORD dwWaitEventNumber;

   phClient->pfnClient(phClient, ConnectionConnected, phClient->pContext);

   while(phClient->bClientThreadAlive)
   {
        WSAResetEvent(phClient->wsaClientEvent);
        WSAEventSelect(phClient->ClientSocket, phClient->wsaClientEvent, FD_READ | FD_CLOSE);
        
        hWaitEvents[0] = phClient->wsaClientEvent;
        hWaitEvents[1] = phClient->hStopClientEvent;

        dwWaitEventNumber = WaitForMultipleObjects(2, hWaitEvents, FALSE, INFINITE);

        if(dwWaitEventNumber == WAIT_OBJECT_0)
        {
           phClient->pfnClient(phClient, ConnectionDataAvailable, phClient->pContext);
        }
   }
   
   phClient->pfnClient(phClient, ConnectionDisconnect, phClient->pContext);
   
   if(phClient->bDelayClose)
   {
       Socket_RealClose(phClient);
   }

   return 0;
 }



/***********************************************
 *
 * Socket_CreateServerSocket
 *
 * Parameters
 *   Server Port
 *
 * Return Value
 *   Socket Handle
 ***********************************************/
 SOCKET Socket_CreateServerSocket(UINT uiPort)
 {
    SOCKET ServerSocket = 0;
    SOCKADDR_IN SockAddr = {0}; 
    int iErrorCode;

    ServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    
    if(ServerSocket)
    {
        SockAddr.sin_family = PF_INET;
        SockAddr.sin_port   = htons((short)uiPort);

        iErrorCode = bind(ServerSocket, (struct sockaddr *)&SockAddr, sizeof(SockAddr));

        if(iErrorCode != SOCKET_ERROR)
        {
            iErrorCode = listen(ServerSocket, SOMAXCONN);
        }   

        if(iErrorCode == SOCKET_ERROR)
        {
           closesocket(ServerSocket);
           ServerSocket = 0;
        }   
    }                   

    return ServerSocket;
 }

/***********************************************
 *
 * Socket_CloseServer
 *
 * Parameters
 *   Handle to Server
 *
 * Return Value
 *   Nothing
 ***********************************************/
 void Socket_CloseServer(HSERVER hServer)
 {
    hServer->bServerThreadAlive = FALSE;

    SetEvent(hServer->hStopServerEvent);

    WaitForSingleObject(hServer->hServerThread, INFINITE);

    WSACloseEvent(hServer->wsaServerEvent);
    
    CloseHandle(hServer->hServerThread);
    CloseHandle(hServer->hStopServerEvent);

    closesocket(hServer->ServerSocket);

    LocalFree(hServer);
 }


/***********************************************
 *
 * Socket_ResolveServer
 *
 * Parameters
 *   Server Name, Server Address (return)
 *
 * Return Value
 *   TRUE/FALSE
 ***********************************************/
BOOL Socket_ResolveServer(PCHAR pServer, UINT uiPort, SOCKADDR_IN *pSockAddr)
{
    BOOL bReturnValue = TRUE;
    LPHOSTENT lpHost;

    pSockAddr->sin_family = PF_INET;
    pSockAddr->sin_port   = htons((short)uiPort);

    if((lpHost = gethostbyname(pServer)) == NULL)
        pSockAddr->sin_addr.s_addr = inet_addr(pServer);
    else
        pSockAddr->sin_addr.s_addr = *((long *)(lpHost->h_addr));

    return bReturnValue;
}


/***********************************************
 *
 * Socket_Connect
 *
 * Parameters
 *    Socket Address
 *
 * Return Value
 *   Handle to Socket
 ***********************************************/
SOCKET Socket_Connect(SOCKADDR_IN *pSockAddr)
{
    SOCKET ClientSocket = 0;
    int iErrorCode;

    ClientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    
    if(ClientSocket)
    {
        iErrorCode = connect(ClientSocket, (struct sockaddr *)pSockAddr, sizeof(SOCKADDR_IN));

        if(iErrorCode == SOCKET_ERROR)
        {
           closesocket(ClientSocket);
           ClientSocket = 0;
        }   
    }                   

    return ClientSocket;
}


/***********************************************
 *
 * Socket_CreateClientThread
 *
 * Parameters
 *   Client Callback, Context, Client Socket
 *
 * Return Value
 *   Handle to Client
 ***********************************************/
 HCLIENT Socket_CreateClientThread(PFNCLIENT pfnClient, PVOID pContext, SOCKET ClientSocket)
 {
     HCLIENT phClient = NULL;
     HANDLE hClientStopEvent = NULL;
     WSAEVENT wsaClientEvent = WSACreateEvent();

     if(wsaClientEvent)
     {
         hClientStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

         if(hClientStopEvent)
         {
         
             phClient = (HCLIENT)LocalAlloc(LMEM_ZEROINIT, sizeof(CLIENT_CONNECTION));
        
             if(phClient)
             {
                 phClient->pfnClient          = pfnClient;
                 phClient->pContext           = pContext;
                 phClient->ClientSocket       = ClientSocket;
                 phClient->wsaClientEvent     = wsaClientEvent;
                 phClient->bClientThreadAlive = TRUE;
                 phClient->hStopClientEvent   = hClientStopEvent;
                 
                 InitializeCriticalSection(&phClient->csReadLock);
                 InitializeCriticalSection(&phClient->csWriteLock);

                 phClient->hClientThread  = CreateThread(NULL, 0, Socket_ClientThread, phClient, 0, &phClient->dwThreadId);
        
                 if(phClient->hClientThread == NULL)
                 {
                     LocalFree(phClient);
                     phClient = NULL;
                 }
             }
    
             if(phClient == NULL)
             {
                 CloseHandle(hClientStopEvent);
                 hClientStopEvent = NULL;
             }
         }

         if(hClientStopEvent == NULL)
         {
             WSACloseEvent(wsaClientEvent);
             wsaClientEvent = NULL;
         }
     }

     return phClient;
 }

/***********************************************
 *
 * Socket_CreateClientConnection
 *
 * Parameters
 *   
 *
 * Return Value
 *   Handle to Client
 ***********************************************/
 HCLIENT Socket_CreateClientConnection(PFNCLIENT pfnClient, PVOID pContext, PCHAR pServer, UINT uiPortNumber)
 {
     HCLIENT phClient = NULL;
     SOCKADDR_IN SockAddr = {0};
     SOCKET ClientSocket;

     if(Socket_ResolveServer(pServer, uiPortNumber, &SockAddr))
     {
        ClientSocket = Socket_Connect(&SockAddr);

        if(ClientSocket)
        {
            phClient = Socket_CreateClientThread(pfnClient, pContext, ClientSocket);

            if(phClient == NULL)
            {
                closesocket(ClientSocket);
                ClientSocket = 0;
            }
            else
            {
                phClient->dwIpAddress = SockAddr.sin_addr.s_addr;
                phClient->dwPort      = SockAddr.sin_port;
            }
        }

     }

     return phClient;
 }

/***********************************************
 *
 * Socket_CreateDirectClientConnection
 *
 * Parameters
 *   
 *
 * Return Value
 *   Handle to Client
 ***********************************************/
 HCLIENT Socket_CreateDirectClientConnection(PFNCLIENT pfnClient, PVOID pContext, DWORD dwIpAddress, DWORD uiPortNumber)
 {
     HCLIENT phClient = NULL;
     SOCKADDR_IN SockAddr = {0};
     SOCKET ClientSocket;

     SockAddr.sin_family      = PF_INET;
     SockAddr.sin_port        = (short)uiPortNumber;
     SockAddr.sin_addr.s_addr = dwIpAddress;

     ClientSocket = Socket_Connect(&SockAddr);

     if(ClientSocket)
     {
        phClient = Socket_CreateClientThread(pfnClient, pContext, ClientSocket);

        if(phClient == NULL)
        {
            closesocket(ClientSocket);
            ClientSocket = 0;
        }
        else
        {
            phClient->dwIpAddress = SockAddr.sin_addr.s_addr;
            phClient->dwPort      = SockAddr.sin_port;
        }
     }

     return phClient;
 }


 /***********************************************
 *
 * Socket_SetClientContext
 *
 * Parameters
 *    Handle To Client, New Context
 *
 * Return Value
 *   None
 ***********************************************/
void Socket_SetClientContext(HCLIENT hClient, PVOID pContext)
{
   hClient->pContext = pContext;
}

/***********************************************
 *
 * Socket_CloseClientConnection
 *
 * Parameters
 *    Handle To Client
 *
 * Return Value
 *   None
 ***********************************************/
 void Socket_CloseClientConnection(HCLIENT hClient)
 {
    DWORD dwCurrentThreadId = GetCurrentThreadId();

    if(dwCurrentThreadId == hClient->dwThreadId)
    {
        hClient->bClientThreadAlive = FALSE;
        hClient->bDelayClose        = TRUE;
        SetEvent(hClient->hStopClientEvent);
    }
    else
    {
        hClient->bClientThreadAlive = FALSE;
        SetEvent(hClient->hStopClientEvent);
        WaitForSingleObject(hClient->hClientThread, INFINITE);
        Socket_RealClose(hClient);
    }
 }


/***********************************************
 *
 * Socket_RealClose
 *
 * Parameters
 *    Handle To Client
 *
 * Return Value
 *   None
 ***********************************************/
 void Socket_RealClose(HCLIENT hClient)
 {

     WSACloseEvent(hClient->wsaClientEvent);

     CloseHandle(hClient->hClientThread);
     CloseHandle(hClient->hStopClientEvent);

     closesocket(hClient->ClientSocket);

     DeleteCriticalSection(&hClient->csReadLock);
     DeleteCriticalSection(&hClient->csWriteLock);

     LocalFree(hClient);
 }


/***********************************************
 *
 * Socket_QueryServerInformation
 *
 * Parameters
 *    Handle To Client, Return For Address, Return For Port
 *
 * Return Value
 *   TRUE/FALSE
 ***********************************************/
BOOL Socket_QueryServerInformation(HCLIENT hClient, DWORD *pdwIpAddress, DWORD *pdwPort)
{
    BOOL bReturnValue = FALSE;

    if(hClient->dwIpAddress)
    {
        bReturnValue = TRUE;

        *pdwIpAddress = hClient->dwIpAddress;
        *pdwPort      = hClient->dwPort;
    }

    return bReturnValue;
}

/***********************************************
 *
 * Socket_SendData
 *
 * Parameters
 *    Handle To Client, Input Buffer, Size In
 *
 * Return Value
 *   TRUE/FALSE
 ***********************************************/
BOOL Socket_SendData(HCLIENT hClient, PVOID pData, UINT uiDataSize)
{
    int iBytesSent = 0, iTotalBytesSent = 0, iBytesToSend = (int)uiDataSize;
    BOOL bPacketSent = FALSE, bNoErrorOccured = TRUE;

    EnterCriticalSection(&hClient->csWriteLock);

    while(iTotalBytesSent < (int)uiDataSize && bNoErrorOccured)
    {
       iBytesSent = send(hClient->ClientSocket, ((char *)pData + iTotalBytesSent), iBytesToSend, 0);

       if(iBytesSent == SOCKET_ERROR || iBytesSent == 0)
       {
           if(WSAGetLastError() != WSAEWOULDBLOCK)
           {
              bNoErrorOccured = FALSE;
           }
       }
       else
       {
           iBytesToSend    = iBytesToSend - iBytesSent;
           iTotalBytesSent = iTotalBytesSent + iBytesSent;
       }
    }

    LeaveCriticalSection(&hClient->csWriteLock);

    if(bNoErrorOccured && iTotalBytesSent == (int)uiDataSize)
    {
       bPacketSent = TRUE;
    }

    return bPacketSent;
}


/***********************************************
 *
 * Socket_RecvData
 *
 * Parameters
 *    Handle To Client, Return Buffer, Size In/Out
 *
 * Return Value
 *   TRUE/FALSE
 ***********************************************/
BOOL Socket_RecvData(HCLIENT hClient, PVOID pData, UINT *puiDataSize)
{
    int iBytesRecv = 0, iBytesToRecv = (int)*puiDataSize;
    BOOL bNoErrorOccured = TRUE;

    EnterCriticalSection(&hClient->csReadLock);
    iBytesRecv = recv(hClient->ClientSocket, pData, iBytesToRecv, 0);

    if(iBytesRecv == SOCKET_ERROR || iBytesRecv == 0)
    {
        if(WSAGetLastError() != WSAEWOULDBLOCK)
        {
           bNoErrorOccured = FALSE;
        }
    }
    else
    {
        *puiDataSize = iBytesRecv;
    }
    LeaveCriticalSection(&hClient->csReadLock);

    return bNoErrorOccured;
}





 
