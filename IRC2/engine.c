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


/*
 * Internal Prototypes
 */
void IRC_ParseIncommingData(PIRCSTRUC pIrcStruc, PCHAR pBuffer, UINT *puiIndex);
void IRC_CheckForEngineUpdates(PIRCSTRUC pIrcStruc, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_CheckForNickChange(PIRCSTRUC pIrcStruc, PIRC_EVENT_STRUC pIrcEventStruc);

/*******************************************************************************
 * IRC_InternalEngine                                                          *
 *                                                                             *
 * DESCRIPTION: Internal IRC Engine                                            *
 *                                                                             *
 * INPUT                                                                       *
 *   Thread Context                                                            *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Status                                                                    *
 *                                                                             *
 *******************************************************************************/
DWORD WINAPI IRC_InternalEngine(PVOID pContext)
{
    DTLSOCKET_STATUS DtlStatus;
    PIRCSTRUC pIrcStruc = (PIRCSTRUC)pContext;
    char szIrcBuffer[IRC_BUFFER_SIZE] = {0};
    UINT uiIndex = 0, uiRetCode, uiTimeOutMs = 50;
    IRC_EVENT_STRUC IrcEventStruc = {0};

    IRC_ChangeNick((HIRC)pIrcStruc, pIrcStruc->IrcUser.szNick);
    IRC_SendUserData((HIRC)pIrcStruc, &pIrcStruc->IrcUser);

    IrcEventStruc.IrcEventId = IRC_EVENT_CONNECT;
    IrcEventStruc.hIrc       = (HIRC)pIrcStruc;

    pIrcStruc->IrcCallBacks.pfnIrcEventCallback[IRC_EVENT_CONNECT](pIrcStruc->pContext, &IrcEventStruc);

    while((DtlStatus = Dtl_SocketWaitForRead(pIrcStruc->DtlSocket, uiTimeOutMs)) != DTLSOCKET_ERROR)
    {
        if(DtlStatus == DTLSOCKET_DATA)
        {
            uiRetCode = Dtl_ReadSocketData(pIrcStruc->DtlSocket, szIrcBuffer + uiIndex, IRC_BUFFER_SIZE - uiIndex);

            if(uiRetCode && uiRetCode != DTL_SOCKET_ERROR)
            {
                uiIndex += uiRetCode;
                IRC_ParseIncommingData(pIrcStruc, szIrcBuffer, &uiIndex);
            }
            else
            {
                Dtl_Destroy(pIrcStruc->DtlSocket);
            }

        }
        else 
        {
            if(uiIndex)
            {
                IRC_ParseIncommingData(pIrcStruc, szIrcBuffer, &uiIndex);
            }
        }

        if(uiIndex)
        {
            uiTimeOutMs = 1;
        }
        else
        {
            uiTimeOutMs = 50;
        }
    }

    IrcEventStruc.IrcEventId = IRC_EVENT_DISCONNECT;
    IrcEventStruc.hIrc       = (HIRC)pIrcStruc;

    pIrcStruc->IrcCallBacks.pfnIrcEventCallback[IRC_EVENT_DISCONNECT](pIrcStruc->pContext, &IrcEventStruc);

    return 0;
}


/*******************************************************************************
 * IRC_ParseIncommingData                                                      *
 *                                                                             *
 * DESCRIPTION: Parse & Dispatch Data                                          *
 *                                                                             *
 * INPUT                                                                       *
 *   Irc Structure, Buffer, Size Of Buffer                                     *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Status                                                                    *
 *                                                                             *
 *******************************************************************************/
void IRC_ParseIncommingData(PIRCSTRUC pIrcStruc, PCHAR pBuffer, UINT *puiIndex)
{
    UINT uiEndOfMessageIndex = 0, uiStartOfNextMessage = 0;
    IRC_EVENT_STRUC IrcEventStruc = {0};
    BOOL bFoundWholeMessage;

    if(*puiIndex != 0)
    {
       IRC_FindEndOfMessage(pBuffer, *puiIndex, uiEndOfMessageIndex);

       if(IRC_IsEndOfMessage(pBuffer, uiEndOfMessageIndex))
       {
           do {
			    memset(&IrcEventStruc, 0, sizeof(IRC_EVENT_STRUC));
				memcpy(&IrcEventStruc.IrcUser, &pIrcStruc->IrcUser, sizeof(IRC_USER));

                IrcEventStruc.hIrc = (HIRC)pIrcStruc;
                
				IRC_ExtractMessageDetails(pIrcStruc, pBuffer + uiStartOfNextMessage, uiEndOfMessageIndex - uiStartOfNextMessage, &IrcEventStruc);

                if(IrcEventStruc.IrcEventId != IRC_EVENT_EMPTY_MESSAGE)
                {
				   IRC_CheckForEngineUpdates(pIrcStruc, &IrcEventStruc);

				   if(pIrcStruc->IrcCallBacks.pfnIrcEventCallback[IrcEventStruc.IrcEventId])
				   {
                      pIrcStruc->IrcCallBacks.pfnIrcEventCallback[IrcEventStruc.IrcEventId](pIrcStruc->pContext, &IrcEventStruc);
				   }
                }

                IRC_SkipToNextMessage(pBuffer, *puiIndex, uiEndOfMessageIndex);

				if(*puiIndex > uiEndOfMessageIndex)
                {
                   uiStartOfNextMessage = uiEndOfMessageIndex;

                   IRC_FindEndOfMessage(pBuffer, *puiIndex, uiEndOfMessageIndex);

				   if(IRC_IsEndOfMessage(pBuffer, uiEndOfMessageIndex) == FALSE)
                   {
					   *puiIndex = (*puiIndex) - uiStartOfNextMessage;
                       strncpy(pBuffer, pBuffer + uiStartOfNextMessage, *puiIndex);
                       bFoundWholeMessage = FALSE;
				   }
                   else
                   {
                       bFoundWholeMessage = TRUE;
                   }
                }
                else
                {
                   *puiIndex = 0;
                    bFoundWholeMessage = FALSE;
                }

           } while(bFoundWholeMessage != FALSE);
       }
    }
}



/*******************************************************************************
 * IRC_CheckForEngineUpdates                                                   *
 *                                                                             *
 * DESCRIPTION: Check if any Engine Information needs updated                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Irc Structure, Event Data                                                 *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   None                                                                    *
 *                                                                             *
 *******************************************************************************/
void IRC_CheckForEngineUpdates(PIRCSTRUC pIrcStruc, PIRC_EVENT_STRUC pIrcEventStruc)
{
	switch(pIrcEventStruc->IrcEventId)
	{
	      case IRC_EVENT_NICK:
			   IRC_CheckForNickChange(pIrcStruc, pIrcEventStruc);
			   break;
	}
}


/*******************************************************************************
 * IRC_CheckForNickChange                                                      *
 *                                                                             *
 * DESCRIPTION: Change nick information if it is updated                    *
 *                                                                             *
 * INPUT                                                                       *
 *   Irc Structure, Event Data                                                 *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   None                                                                    *
 *                                                                             *
 *******************************************************************************/
void IRC_CheckForNickChange(PIRCSTRUC pIrcStruc, PIRC_EVENT_STRUC pIrcEventStruc)
{
	if(pIrcEventStruc->bNickIsSelf)
	{
		strcpy(pIrcStruc->IrcUser.szNick, pIrcEventStruc->szNickCommand);	
	}
}
