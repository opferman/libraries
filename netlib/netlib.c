/*
 * Network Library
 *
 *  
 *
 *  Toby Opferman
 *
 *  Copyright 2004, All Rights Reserved
 *
 */


 #include <windows.h>
 #include "networkinternal.h"
 #include <netlib.h>
 #include <peblib.h>
 
 
PFN_TCPTABLE GetTcpTable;
PFN_UDPTABLE GetUdpTable;
PFN_TCPTABLEFROMSTACK AllocateAndGetTcpExTableFromStack;
PFN_UDPTABLEFROMSTACK AllocateAndGetUdpExTableFromStack;
 
 /*
  * Internal Prototypes
  */
PMIB_TCPTABLE NetworkLibrary_AllocateTcpTable(void);  
PMIB_TCPTABLE_EX NetworkLibrary_AllocateTcpTableEx(void);
PMIB_UDPTABLE NetworkLibrary_AllocateUdpTable(void);
PMIB_UDPTABLE_EX NetworkLibrary_AllocateUdpTableEx(void);
void NetworkLibrary_FreeData(PVOID pData);
void NetworkLibrary_FreeDataEx(PVOID pData);


/***********************************************
 *
 * NetworkLibrary_Initialize
 *
 * Parameters
 *   
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
 BOOL NetworkLibrary_Initialize(void)
 {
     HINSTANCE hInstance;
     BOOL bRetValue = FALSE;

     hInstance = LoadLibrary("iphlpapi.dll");

     if(hInstance)
     {
         /*
          * XP and Greater (OPTIONAL)
          */
         AllocateAndGetUdpExTableFromStack = (PFN_UDPTABLEFROMSTACK)GetProcAddress(hInstance, "AllocateAndGetUdpExTableFromStack");
         AllocateAndGetTcpExTableFromStack = (PFN_TCPTABLEFROMSTACK)GetProcAddress(hInstance, "AllocateAndGetTcpExTableFromStack");

         GetTcpTable = (PFN_TCPTABLE)GetProcAddress(hInstance, "GetTcpTable");
         GetUdpTable = (PFN_UDPTABLE)GetProcAddress(hInstance, "GetUdpTable");

         if(GetTcpTable && GetUdpTable)
         {
             bRetValue = TRUE;
         }
     }

     return bRetValue;
 }



/***********************************************
 *
 * NetworkLibrary_EnumerateAllNetworkConnections
 *
 * Parameters
 *   
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
BOOL NetworkLibrary_EnumerateAllNetworkConnections(PFN_NETWORKENUMERATIONCALLBACK pfnNetworkCallback, PVOID pContext)
{
    BOOL bReturnValue = FALSE;

    bReturnValue = NetworkLibrary_EnumerateTCPNetworkConnections(pfnNetworkCallback, pContext);

    if(bReturnValue)
    {
        bReturnValue = NetworkLibrary_EnumerateUDPNetworkConnections(pfnNetworkCallback, pContext);
    }

    return bReturnValue;
}

/***********************************************
 *
 * NetworkLibrary_EnumerateUDPNetworkConnections
 *
 * Parameters
 *   
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
BOOL NetworkLibrary_EnumerateUDPNetworkConnections(PFN_NETWORKENUMERATIONCALLBACK pfnNetworkCallback, PVOID pContext)
{
    BOOL bReturnValue = FALSE;
    PMIB_UDPTABLE pMibUdpTable = NULL;
    PMIB_UDPTABLE_EX pMibUdpTableEx = NULL;
    PMIB_UDPROW_EX pMibUdpRowEx;
    PMIB_UDPROW pMibUdpRow;
    DWORD dwIndex = 0;
    NETWORK_INFORMATION NetworkInformation;

    pMibUdpTableEx = NetworkLibrary_AllocateUdpTableEx();

    if(!pMibUdpTableEx)
    {
        pMibUdpTable = NetworkLibrary_AllocateUdpTable();

        if(pMibUdpTable)
        {
           bReturnValue = TRUE;
           pMibUdpRow   = &pMibUdpTable->UdpEntries;

           while(bReturnValue && dwIndex < pMibUdpTable->dwNumEntries)
           {
               ZeroMemory(&NetworkInformation, sizeof(NetworkInformation));

               NetworkInformation.dwSize      = sizeof(NetworkInformation);
               NetworkInformation.dwLocalAddr = pMibUdpRow[dwIndex].dwLocalAddr;
               NetworkInformation.dwLocalPort = pMibUdpRow[dwIndex].dwLocalPort;

               bReturnValue = pfnNetworkCallback(pContext, &NetworkInformation);

               dwIndex++;
           }

           NetworkLibrary_FreeData(pMibUdpTable);
        }
    }
    else
    {

       bReturnValue = TRUE;
       pMibUdpRowEx   = &pMibUdpTableEx->UdpEntries;

       while(bReturnValue && dwIndex < pMibUdpTableEx->dwNumEntries)
       {
           ZeroMemory(&NetworkInformation, sizeof(NetworkInformation));

           NetworkInformation.dwSize      = sizeof(NetworkInformation);
           NetworkInformation.dwLocalAddr = pMibUdpRowEx[dwIndex].dwLocalAddr;
           NetworkInformation.dwLocalPort = pMibUdpRowEx[dwIndex].dwLocalPort;

           NetworkInformation.bExtendedInformationIsValid = TRUE;
           NetworkInformation.dwPid =  pMibUdpRowEx[dwIndex].dwPid;
           PEB_QueryProcessShortImageNameA(NetworkInformation.dwPid, NetworkInformation.szImageName, sizeof(NetworkInformation.szImageName));

           bReturnValue = pfnNetworkCallback(pContext, &NetworkInformation);

           dwIndex++;
       }

       NetworkLibrary_FreeDataEx(pMibUdpTableEx);
    }

    return bReturnValue;
}


/***********************************************
 *
 * NetworkLibrary_EnumerateTCPNetworkConnections
 *
 * Parameters
 *   
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
BOOL NetworkLibrary_EnumerateTCPNetworkConnections(PFN_NETWORKENUMERATIONCALLBACK pfnNetworkCallback, PVOID pContext)
{
    BOOL bReturnValue = FALSE;
    PMIB_TCPTABLE pMibTcpTable = NULL;
    PMIB_TCPTABLE_EX pMibTcpTableEx = NULL;
    PMIB_TCPROW_EX pMibTcpRowEx;
    PMIB_TCPROW pMibTcpRow;
    DWORD dwIndex = 0;
    NETWORK_INFORMATION NetworkInformation;

    pMibTcpTableEx = NetworkLibrary_AllocateTcpTableEx();

    if(!pMibTcpTableEx)
    {
        pMibTcpTable = NetworkLibrary_AllocateTcpTable();

        if(pMibTcpTable)
        {
           bReturnValue = TRUE;
           pMibTcpRow   = &pMibTcpTable->TcpEntries;

           while(bReturnValue && dwIndex < pMibTcpTable->dwNumEntries)
           {
               ZeroMemory(&NetworkInformation, sizeof(NetworkInformation));

               NetworkInformation.dwSize      = sizeof(NetworkInformation);
               NetworkInformation.dwLocalAddr = pMibTcpRow[dwIndex].dwLocalAddr;
               NetworkInformation.dwLocalPort = pMibTcpRow[dwIndex].dwLocalPort;
               
               NetworkInformation.bConnectionIsTCP = TRUE;
               NetworkInformation.dwState      = pMibTcpRow[dwIndex].dwState;
               NetworkInformation.dwRemoteAddr = pMibTcpRow[dwIndex].dwRemoteAddr;
               NetworkInformation.dwRemotePort = pMibTcpRow[dwIndex].dwRemotePort;

               bReturnValue = pfnNetworkCallback(pContext, &NetworkInformation);

               dwIndex++;
           }

           NetworkLibrary_FreeData(pMibTcpTable);
        }
    }
    else
    {

       bReturnValue = TRUE;
       pMibTcpRowEx   = &pMibTcpTableEx->TcpEntries;

       while(bReturnValue && dwIndex < pMibTcpTableEx->dwNumEntries)
       {
           ZeroMemory(&NetworkInformation, sizeof(NetworkInformation));

           NetworkInformation.dwSize      = sizeof(NetworkInformation);
           NetworkInformation.dwLocalAddr = pMibTcpRowEx[dwIndex].dwLocalAddr;
           NetworkInformation.dwLocalPort = pMibTcpRowEx[dwIndex].dwLocalPort;
           
           NetworkInformation.bConnectionIsTCP = TRUE;
           NetworkInformation.dwState      = pMibTcpRowEx[dwIndex].dwState;
           NetworkInformation.dwRemoteAddr = pMibTcpRowEx[dwIndex].dwRemoteAddr;
           NetworkInformation.dwRemotePort = pMibTcpRowEx[dwIndex].dwRemotePort;
           
           NetworkInformation.bExtendedInformationIsValid = TRUE;
           NetworkInformation.dwPid =  pMibTcpRowEx[dwIndex].dwPid;
           PEB_QueryProcessShortImageNameA(NetworkInformation.dwPid, NetworkInformation.szImageName, sizeof(NetworkInformation.szImageName));

           bReturnValue = pfnNetworkCallback(pContext, &NetworkInformation);

           dwIndex++;
       }

       NetworkLibrary_FreeDataEx(pMibTcpTableEx);
    }

    return bReturnValue;

}

     
/***********************************************
 *
 * NetworkLibrary_AllocateTcpTable
 *
 * Parameters
 *   
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
PMIB_TCPTABLE NetworkLibrary_AllocateTcpTable(void)
{
    PMIB_TCPTABLE pMibTcpTable = NULL;
    DWORD dwTableSize = 0;

    GetTcpTable(0, &dwTableSize, 0);

    if(dwTableSize)
    {
        pMibTcpTable = LocalAlloc(LMEM_ZEROINIT, dwTableSize + 4);

        if(pMibTcpTable)
        {
            GetTcpTable(pMibTcpTable, &dwTableSize, 0);
        }
    }

    return pMibTcpTable;
}

/***********************************************
 *
 * NetworkLibrary_AllocateUdpTable
 *
 * Parameters
 *   
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
PMIB_UDPTABLE NetworkLibrary_AllocateUdpTable(void)
{
    PMIB_UDPTABLE pMibUdpTable = NULL;
    DWORD dwTableSize = 0;

    GetUdpTable(0, &dwTableSize, 0);

    if(dwTableSize)
    {
        pMibUdpTable = LocalAlloc(LMEM_ZEROINIT, dwTableSize + 4);

        if(pMibUdpTable)
        {
            GetUdpTable(pMibUdpTable, &dwTableSize, 0);
        }
    }

    return pMibUdpTable;
}




/***********************************************
 *
 * NetworkLibrary_AllocateTcpTableEx
 *
 * Parameters
 *   
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
PMIB_TCPTABLE_EX NetworkLibrary_AllocateTcpTableEx(void)
{
    PMIB_TCPTABLE_EX pMibTcpTableEx = NULL;

    if(AllocateAndGetTcpExTableFromStack)
    {
        AllocateAndGetTcpExTableFromStack(&pMibTcpTableEx, 1, GetProcessHeap(), 0, 2);
    }

    return pMibTcpTableEx;
}

/***********************************************
 *
 * NetworkLibrary_AllocateUdpTableEx
 *
 * Parameters
 *   
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
PMIB_UDPTABLE_EX NetworkLibrary_AllocateUdpTableEx(void)
{
    PMIB_UDPTABLE_EX pMibUdpTableEx = NULL;
    DWORD dwTableSize = 0;

    if(AllocateAndGetUdpExTableFromStack)
    {
        AllocateAndGetUdpExTableFromStack(&pMibUdpTableEx, 1, GetProcessHeap(), 0, 2);
    }

    return pMibUdpTableEx;
}


/***********************************************
 *
 * NetworkLibrary_FreeData
 *
 * Parameters
 *   
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
void NetworkLibrary_FreeData(PVOID pData)
{
    LocalFree(pData);
}



/***********************************************
 *
 * NetworkLibrary_FreeDataEx
 *
 * Parameters
 *   
 *
 * Return Value
 *   TRUE = Succeeded
 *   FALSE = Failure
 ***********************************************/
void NetworkLibrary_FreeDataEx(PVOID pData)
{
    HeapFree(GetProcessHeap(), 0, pData);
}






