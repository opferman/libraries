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
 * IRC_ConnectToServer                                                         *
 *                                                                             *
 * DESCRIPTION: Connect to IRC Server                                          *
 *                                                                             *
 * INPUT                                                                       *
 *   Irc Server Name, Irc Port, User Structure, IRC Event Data Callback        *
 *   Callback Context                                                          *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Handle To IRC Object                                                      *
 *                                                                             *
 *******************************************************************************/
HIRC IRC_ConnectToServer(PCHAR IrcServer, USHORT IrcPort, PIRC_USER pIrcUser, PEVENT_CALLBACKS pIrcCallbacks, PVOID pContext)
{
    PIRCSTRUC pIrcStruc = (PIRCSTRUC)INVALID_IRC_HANDLE;

    if(pIrcStruc = LocalAlloc(LMEM_ZEROINIT, sizeof(IRCSTRUC)))
    {
           if(pIrcStruc->DtlSocket = Dtl_Create_Client(IrcServer, IrcPort, 0))
           {
			   memcpy(&pIrcStruc->IrcCallBacks, pIrcCallbacks, sizeof(EVENT_CALLBACKS));
               pIrcStruc->pContext    = pContext;
               memcpy(&pIrcStruc->IrcUser, pIrcUser, sizeof(IRC_USER));

               if(!(pIrcStruc->hThread = CreateThread(NULL, 0, IRC_InternalEngine, (PVOID)pIrcStruc, 0, &pIrcStruc->ThreadID)))
               {
                   Dtl_Destroy(pIrcStruc->DtlSocket);
                   LocalFree(pIrcStruc);
                   pIrcStruc = (PIRCSTRUC)INVALID_IRC_HANDLE;
               }
           }
           else
           {               
               LocalFree(pIrcStruc);
               pIrcStruc = (PIRCSTRUC)INVALID_IRC_HANDLE;
           } 
    }

    return (HIRC)pIrcStruc;
}



/*******************************************************************************
 * IRC_Destroy                                                                 *
 *                                                                             *
 * DESCRIPTION: DisConnect IRC Server / Cleanup                                *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle To IRC Object                                                      *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_Destroy(HIRC hIrc)
{
    PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;

    Dtl_Destroy(pIrcStruc->DtlSocket);

    WaitForSingleObject(pIrcStruc->hThread, INFINITE);
    CloseHandle(pIrcStruc->hThread);

    LocalFree(pIrcStruc);
}



/*******************************************************************************
 * IRC_MsgChannel                                                              *
 *                                                                             *
 * DESCRIPTION: Msg Channel                                                    *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Channel, Text                                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_MsgChannel(HIRC hIrc, PCHAR pszChannel, PCHAR pszText)
{
   return IRC_SendText(hIrc, pszChannel, pszText, IRC_SEND_MSG);
}

/*******************************************************************************
 * IRC_MsgNick                                                                 *
 *                                                                             *
 * DESCRIPTION: Msg Nick                                                       *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Nick, Text                                                 *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_MsgNick(HIRC hIrc, PCHAR pszNick, PCHAR pszText)
{
   return IRC_SendText(hIrc, pszNick, pszText, IRC_SEND_MSG);
}

/*******************************************************************************
 * IRC_NoticeChannel                                                           *
 *                                                                             *
 * DESCRIPTION: Notice Channel                                                 *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Channel, Text                                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_NoticeChannel(HIRC hIrc, PCHAR pszChannel, PCHAR pszText)
{
   return IRC_SendText(hIrc, pszChannel, pszText, IRC_SEND_NOTICE);
}


/*******************************************************************************
 * IRC_NoticeNick                                                              *
 *                                                                             *
 * DESCRIPTION: Notice Nick                                                    *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Nick, Text                                                 *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_NoticeNick(HIRC hIrc, PCHAR pszNick, PCHAR pszText)
{
   return IRC_SendText(hIrc, pszNick, pszText, IRC_SEND_NOTICE);
}





/*******************************************************************************
 * IRC_PartChannel                                                             *
 *                                                                             *
 * DESCRIPTION: Part Channel                                                   *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Channel To Part                                            *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_PartChannel(HIRC hIrc, PCHAR pszChannel)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char szMessage[MAX_MESSAGE_SIZE + 1];

   sprintf(szMessage, "PART %s\r\n", pszChannel);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, szMessage, strlen(szMessage));

   return IRC_SUCCESS;
}

/*******************************************************************************
 * IRC_JoinChannel                                                             *
 *                                                                             *
 * DESCRIPTION: Join Channel                                                   *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Channel To Join                                            *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_JoinChannel(HIRC hIrc, PCHAR pszChannel)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char szMessage[MAX_MESSAGE_SIZE + 1];

   sprintf(szMessage, "JOIN %s\r\n", pszChannel);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, szMessage, strlen(szMessage));

   return IRC_SUCCESS;
}

/*******************************************************************************
 * IRC_SendQuitMessage                                                         *
 *                                                                             *
 * DESCRIPTION: Quit IRC                                                       *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Quit Message                                               *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_SendQuitMessage(HIRC hIrc, PCHAR pszQuitText)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char szMessage[MAX_MESSAGE_SIZE + 1];

   sprintf(szMessage, "QUIT :%s\r\n", pszQuitText);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, szMessage, strlen(szMessage));

   return IRC_SUCCESS;
}


/*******************************************************************************
 * IRC_ChangeNick                                                              *
 *                                                                             *
 * DESCRIPTION: Change Nick                                                    *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, New Nick Name                                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_ChangeNick(HIRC hIrc, PCHAR pszNick)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char szMessage[MAX_MESSAGE_SIZE + 1];

   sprintf(szMessage, "NICK %s\r\n", pszNick);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, szMessage, strlen(szMessage));

   return IRC_SUCCESS;
}

/*******************************************************************************
 * IRC_PongMessage                                                             *
 *                                                                             *
 * DESCRIPTION: PONG!                                                          *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Server                                                     *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_SendPongMessage(HIRC hIrc, PCHAR pszServer)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char szMessage[MAX_MESSAGE_SIZE + 1];

   sprintf(szMessage, "PONG %s\r\n", pszServer);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, szMessage, strlen(szMessage));

   return IRC_SUCCESS;
}