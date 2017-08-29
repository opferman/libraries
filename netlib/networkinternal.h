/*
 * Network Library - Internal
 *
 *  Information about Networks
 *
 *  Toby Opferman
 *
 *  Copyright 2004, All Rights Reserved
 *
 */

#ifndef __NETWORKINTERNAL_H__
#define __NETWORKINTERNAL_H__

 
 
/***********************************************
 * Internal Defines & Structures 
 ***********************************************/ 

typedef struct _MIB_TCPROW
{

  DWORD dwState;
  DWORD dwLocalAddr;
  DWORD dwLocalPort;
  DWORD dwRemoteAddr;
  DWORD dwRemotePort;

} MIB_TCPROW, *PMIB_TCPROW;


typedef struct _MIB_UDPROW
{

  DWORD dwLocalAddr;
  DWORD dwLocalPort;

} MIB_UDPROW, *PMIB_UDPROW;


typedef struct _MIB_TCPTABLE
{

  DWORD dwNumEntries;
  MIB_TCPROW TcpEntries;

} MIB_TCPTABLE, *PMIB_TCPTABLE;

typedef struct _MIB_UDPTABLE
{
  
  DWORD dwNumEntries;
  MIB_UDPROW UdpEntries;

} MIB_UDPTABLE, *PMIB_UDPTABLE;



typedef struct _MIB_TCPROW_EX
{

  DWORD dwState;
  DWORD dwLocalAddr;
  DWORD dwLocalPort;
  DWORD dwRemoteAddr;
  DWORD dwRemotePort;
  DWORD dwPid;

} MIB_TCPROW_EX, *PMIB_TCPROW_EX;


typedef struct _MIB_UDPROW_EX
{

  DWORD dwLocalAddr;
  DWORD dwLocalPort;
  DWORD dwPid;

} MIB_UDPROW_EX, *PMIB_UDPROW_EX;


typedef struct _MIB_TCPTABLE_EX
{

  DWORD dwNumEntries;
  MIB_TCPROW_EX TcpEntries;

} MIB_TCPTABLE_EX, *PMIB_TCPTABLE_EX;

typedef struct _MIB_UDPTABLE_EX
{
  
  DWORD dwNumEntries;
  MIB_UDPROW_EX UdpEntries;

} MIB_UDPTABLE_EX, *PMIB_UDPTABLE_EX;



typedef DWORD (WINAPI *PFN_TCPTABLE)(OUT PMIB_TCPTABLE pTcpTable, IN OUT PDWORD pdwSize, IN BOOL bOrder);
extern PFN_TCPTABLE GetTcpTable;

typedef DWORD (WINAPI *PFN_UDPTABLE)(OUT PMIB_UDPTABLE pUdpTable, IN OUT PDWORD pdwSize, IN BOOL bOrder);
extern PFN_UDPTABLE GetUdpTable;

typedef DWORD (WINAPI *PFN_TCPTABLEFROMSTACK)(PMIB_TCPTABLE_EX *, BOOL, HANDLE, DWORD, DWORD);
extern PFN_TCPTABLEFROMSTACK AllocateAndGetTcpExTableFromStack;

typedef DWORD (WINAPI *PFN_UDPTABLEFROMSTACK)(PMIB_UDPTABLE_EX *, BOOL, HANDLE, DWORD, DWORD);
extern PFN_UDPTABLEFROMSTACK AllocateAndGetUdpExTableFromStack;



 


#endif





