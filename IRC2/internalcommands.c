/***************************************************************************** 
 *                             IRC Library Version 2.0                       *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2001-2005      Toby Opferman                              *
 *****************************************************************************/

#include <windows.h>
#include <winsock.h>
#include <dtl.h>
#include <irc2.h>
#include <stdio.h>
#include "ircinternal.h"


/*******************************************************************************
 * IRC_SendUserData                                                            *
 *                                                                             *
 * DESCRIPTION: Send User Data                                                 *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Pointer to User Data                                       *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_SendUserData(HIRC hIrc, PIRC_USER pIrcUser)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char szMessage[MAX_MESSAGE_SIZE + 1];

   sprintf(szMessage, "USER %s %s %s :%s\r\n", pIrcUser->szUserName, pIrcUser->szHostName, pIrcUser->szServerName, pIrcUser->szRealName);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, szMessage, strlen(szMessage));

   return IRC_SUCCESS;
}




/*******************************************************************************
 * IRC_SendText                                                                *
 *                                                                             *
 * DESCRIPTION: Sends text to an IRC Target                                    *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Target, Text, Message Type                                 *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_SendText(HIRC hIrc, PCHAR pszTarget, PCHAR pszText, UINT uiTypeFlags)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char szMessage[MAX_MESSAGE_SIZE + 1], szTextMessage[MAX_MESSAGE_SIZE + 1];
   UINT uiIndex = 0;
   char *pszMessageTypeArray[] = { "PRIVMSG %s :%s\r\n", "NOTICE %s :%s\r\n" };

   while(*pszText)
   {
      for(uiIndex = 0; *pszText && *pszText != '\n' && *pszText !='\r'; uiIndex++, pszText++)
      {
         szTextMessage[uiIndex] = *pszText;
      }

      szTextMessage[uiIndex] = 0;
      sprintf(szMessage, pszMessageTypeArray[uiTypeFlags], pszTarget, szTextMessage);

      Dtl_SendSocketData(pIrcStruc->DtlSocket, szMessage, strlen(szMessage));

      while(*pszText == '\r' || *pszText == '\n')
          pszText++;
   }

   return IRC_SUCCESS;
}

 