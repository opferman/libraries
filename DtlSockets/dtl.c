/***************************************************************************** 
 *                         Data Transfer Layer Library                       *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2001           Toby Opferman                              *
 *****************************************************************************/

#include <windows.h>
#include <winsock.h>
#include <dtl.h>
#include <imagehlp.h>

/***************************************************************************** 
 * Internal Data Structure                                                   *
 *****************************************************************************/
typedef struct {

    SOCKET       hSocket;
    SOCKADDR_IN  SockAddr;
    LPHOSTENT    lpHost;

} DTL_SOCKET_INTERNAL, *PDTL_SOCKET_INTERNAL;

#define WSA_VERSION    0x0101



/***************************************************************************** 
 * Internal Prototypes                                                       *
 *****************************************************************************/
BOOL Dtl_Initialize(void);
void Dtl_UnInitialize(void);


/*******************************************************************************
 * DllMain                                                                     *
 *                                                                             *
 * DESCRIPTION: DLL Entry Point                                                *
 *                                                                             *
 * INPUT                                                                       *
 *   Standard Win32 DLL Entry                                                  *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   TRUE = Initialized, FALSE = Failure                                       *
 *                                                                             *
 *******************************************************************************/
BOOL WINAPI DllMain(HANDLE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    BOOL bRetValue = TRUE;

    switch(dwReason)
    {
       case DLL_PROCESS_ATTACH:
            if(!Dtl_Initialize())
                bRetValue = FALSE;
            break;

       case DLL_PROCESS_DETACH:
            Dtl_UnInitialize();
            break;

    }

    return bRetValue;
}



/*******************************************************************************
 * Dtl_Initialize                                                              *
 *                                                                             *
 * DESCRIPTION: Initialize the DTL library                                     *
 *                                                                             *
 * INPUT                                                                       *
 *   Nothing                                                                   *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   TRUE = Initialized, FALSE = Failure                                       *
 *                                                                             *
 *******************************************************************************/
BOOL Dtl_Initialize(void)
{
    WSADATA WsaData;
    BOOL bRetValue = TRUE;

    if(WSAStartup(WSA_VERSION, &WsaData))
       bRetValue = FALSE;

    return bRetValue;
}

/*******************************************************************************
 * Dtl_UnInitialize                                                            *
 *                                                                             *
 * DESCRIPTION: UnInitialize the DTL library                                   *
 *                                                                             *
 * INPUT                                                                       *
 *   Nothing                                                                   *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void Dtl_UnInitialize(void)
{
    WSACleanup();
}


/*******************************************************************************
 * Dtl_Create_Client                                                           *
 *                                                                             *
 * DESCRIPTION: Connect to a server                                            *
 *                                                                             *
 * INPUT                                                                       *
 *   ServerName, Server Port, Flags (not used)                                 *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Handle to a DTL Socket                                                    *
 *                                                                             *
 *******************************************************************************/
DTL_SOCKET Dtl_Create_Client(PCHAR ServerName, USHORT ServerPort, DWORD Flags)
{
    PDTL_SOCKET_INTERNAL pDtlSocketInt = (PDTL_SOCKET_INTERNAL)INVALID_DTL_SOCKET;

    if(pDtlSocketInt = LocalAlloc(LMEM_ZEROINIT, sizeof(DTL_SOCKET_INTERNAL)))
    {
        if((pDtlSocketInt->hSocket = socket(AF_INET, SOCK_STREAM, 0)) != INVALID_SOCKET)
        {
             pDtlSocketInt->SockAddr.sin_family = PF_INET;
             pDtlSocketInt->SockAddr.sin_port   = htons(ServerPort);

             if((pDtlSocketInt->lpHost = gethostbyname(ServerName)) == NULL)
                 pDtlSocketInt->SockAddr.sin_addr.s_addr = inet_addr(ServerName);
             else
                 pDtlSocketInt->SockAddr.sin_addr.s_addr = *((long *)(pDtlSocketInt->lpHost->h_addr));

             if(!pDtlSocketInt->SockAddr.sin_addr.s_addr)
             {
                    closesocket(pDtlSocketInt->hSocket);
                    LocalFree(pDtlSocketInt);
                    pDtlSocketInt = (PDTL_SOCKET_INTERNAL)INVALID_DTL_SOCKET;
             }
             else if(connect(pDtlSocketInt->hSocket, (struct sockaddr *)&pDtlSocketInt->SockAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
                  {
                        closesocket(pDtlSocketInt->hSocket);
                        LocalFree(pDtlSocketInt);
                        pDtlSocketInt = (PDTL_SOCKET_INTERNAL)INVALID_DTL_SOCKET;
                  }  

        }
        else
        {
            LocalFree(pDtlSocketInt);
            pDtlSocketInt = (PDTL_SOCKET_INTERNAL)INVALID_DTL_SOCKET;
        }
    }

    return (DTL_SOCKET)pDtlSocketInt;
}


/*******************************************************************************
 * Dtl_Create_Server                                                           *
 *                                                                             *
 * DESCRIPTION: Create a server                                                *
 *                                                                             *
 * INPUT                                                                       *
 *   Numer of listeners, Server Port, Flags (not used)                         *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Handle to a DTL Socket                                                    *
 *                                                                             *
 *******************************************************************************/
DTL_SOCKET Dtl_Create_Server(UINT Listeners, USHORT Port, DWORD Flags)
{
    return INVALID_DTL_SOCKET;
}


/*******************************************************************************
 * Dtl_SendSocketData                                                          *
 *                                                                             *
 * DESCRIPTION: Send Socket Data                                               *
 *                                                                             *
 * INPUT                                                                       *
 *   DTL Socket, Output Buffer, Size of Output buffer                          *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Bytes Sent                                                                *
 *                                                                             *
 *******************************************************************************/
UINT Dtl_SendSocketData(DTL_SOCKET DtlSocket, PCHAR pBuffer, UINT Size)
{
    PDTL_SOCKET_INTERNAL pDtlSocketInt = (PDTL_SOCKET_INTERNAL)DtlSocket;

    return send(pDtlSocketInt->hSocket, pBuffer, Size, 0);
}


/*******************************************************************************
 * Dtl_SocketWaitForRead                                                       *
 *                                                                             *
 * DESCRIPTION: Wait For Socket Data                                           *
 *                                                                             *
 * INPUT                                                                       *
 *   DTL Socket, Millisecond Timeout                                           *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   DTLSOCKET_DATA, DTLSOCKET_NODATA, DTLSOCKET_ERROR                         *
 *                                                                             *
 *******************************************************************************/
DTLSOCKET_STATUS Dtl_SocketWaitForRead(DTL_SOCKET DtlSocket, UINT msTimeOut)
{
    PDTL_SOCKET_INTERNAL pDtlSocketInt = (PDTL_SOCKET_INTERNAL)DtlSocket;
    fd_set SocketList;
    struct timeval TimeOut;
    DTLSOCKET_STATUS RetVal = DTLSOCKET_NODATA;

    TimeOut.tv_sec = 0;
    TimeOut.tv_usec = msTimeOut;

    FD_ZERO(&SocketList);
    FD_SET(pDtlSocketInt->hSocket, &SocketList);

    if(select(0, &SocketList, NULL, NULL, &TimeOut) != SOCKET_ERROR)
    {
        if(FD_ISSET(pDtlSocketInt->hSocket, &SocketList))
            RetVal = DTLSOCKET_DATA;
    }
    else
      RetVal = DTLSOCKET_ERROR;

    FD_ZERO(&SocketList);

    return RetVal;
}



/*******************************************************************************
 * Dtl_GetLastError                                                            *
 *                                                                             *
 * DESCRIPTION: Get last error                                                 *
 *                                                                             *
 * INPUT                                                                       *
 *   Nothing                                                                   *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Error Code                                                                *
 *                                                                             *
 *******************************************************************************/
UINT Dtl_GetLastError(void)
{
    return WSAGetLastError();
}

/*******************************************************************************
 * Dtl_ReadSocketData                                                          *
 *                                                                             *
 * DESCRIPTION: Read Socket Data                                               *
 *                                                                             *
 * INPUT                                                                       *
 *   DTL Socket, Input Buffer, Size of Input buffer                            *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Bytes Read                                                                *
 *                                                                             *
 *******************************************************************************/
UINT Dtl_ReadSocketData(DTL_SOCKET DtlSocket, PCHAR pBuffer, UINT Size)
{
    PDTL_SOCKET_INTERNAL pDtlSocketInt = (PDTL_SOCKET_INTERNAL)DtlSocket;

    return recv(pDtlSocketInt->hSocket, pBuffer, Size, 0);
}

/*******************************************************************************
 * Dtl_Destroy                                                                 *
 *                                                                             *
 * DESCRIPTION: Destroy a DTL Socket                                           *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to a DTL Socket                                                    *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void Dtl_Destroy(DTL_SOCKET DtlSocket)
{
    PDTL_SOCKET_INTERNAL pDtlSocketInt = (PDTL_SOCKET_INTERNAL)DtlSocket;

    closesocket(pDtlSocketInt->hSocket);
    LocalFree(pDtlSocketInt);
}






 