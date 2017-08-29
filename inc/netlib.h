/*
 * Network Library
 *
 *  Information about the Network
 *
 *  Toby Opferman
 *
 *  Copyright 2004, All Rights Reserved
 *
 */

#ifndef __NETLIB_H__
#define __NETLIB_H__                                                       


typedef struct _NETWORK_INFORMATION
{
  DWORD dwSize;
  DWORD dwLocalAddr;
  DWORD dwLocalPort;

  BOOL  bConnectionIsTCP;
  DWORD dwState;
  DWORD dwRemoteAddr;
  DWORD dwRemotePort;
  
  BOOL  bExtendedInformationIsValid;
  DWORD dwPid;
  char szImageName[MAX_PATH + 1];

} NETWORK_INFORMATION, *PNETWORK_INFORMATION;


#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (WINAPI *PFN_NETWORKENUMERATIONCALLBACK)(PVOID pContext, PNETWORK_INFORMATION pNetworkInformation);

BOOL NetworkLibrary_Initialize(void);
BOOL NetworkLibrary_EnumerateAllNetworkConnections(PFN_NETWORKENUMERATIONCALLBACK pfnNetworkCallback, PVOID pContext);
BOOL NetworkLibrary_EnumerateUDPNetworkConnections(PFN_NETWORKENUMERATIONCALLBACK pfnNetworkCallback, PVOID pContext);
BOOL NetworkLibrary_EnumerateTCPNetworkConnections(PFN_NETWORKENUMERATIONCALLBACK pfnNetworkCallback, PVOID pContext);


#ifdef __cplusplus
}
#endif

#endif


