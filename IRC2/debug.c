/***************************************************************************** 
 *                             IRC Library Version 2.0                       *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2001-2005       Toby Opferman                             *
 *****************************************************************************/

#include <windows.h>
#include <winsock.h>
#include <dtl.h>
#include <irc2.h>
#include <stdio.h>
#include "ircinternal.h"

#ifdef _DEBUG
/***********************************************************************
 * IRC_Debug
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
 void IRC_Debug(char *pszFormatString, ...)
 {
     char DebugString[256];
     va_list vl;

     va_start(vl, pszFormatString);
     vsprintf(DebugString, pszFormatString, vl);
     va_end(vl);

     OutputDebugString(DebugString);
 }
#endif