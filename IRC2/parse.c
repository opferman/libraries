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


typedef BOOL (*PFNPARSEEVENT)(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
typedef void (*PFNPARSEEVENTDATA)(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);

typedef struct _EVENT_PARSE_DATA {
    /*
     * Implement only Event Name or a Parse Event function, not both
     */
    char *pszEventName;
    PFNPARSEEVENT pfnParseEvent;

    IRC_EVENT IrcEvent;

    PFNPARSEEVENTDATA pfnParseEventData;
} EVENT_PARSE_DATA, *PEVENT_PARSE_DATA;

#define FLAG_GENERIC_EXTRACT_CHANNEL_AND_NICK   1

/*
 * Macros to copy data into event structure
 */
#define IRC_CopyNick(pIrcEventStruc, pBuffer, uiSize) \
            strncpy(pIrcEventStruc->szNick, pBuffer, uiSize); \
			pIrcEventStruc->dwFlags |= FLAG_NICK_VALID;

#define IRC_CopyHostDetails(pIrcEventStruc, pBuffer, uiSize) \
            strncpy(pIrcEventStruc->szNickHostDetails, pBuffer, uiSize); \
			pIrcEventStruc->dwFlags |= FLAG_NICK_HOST_DETAILS_VALID;

#define IRC_CopyChannel(pIrcEventStruc, pBuffer, uiSize) \
            strncpy(pIrcEventStruc->szChannel, pBuffer, uiSize); \
			pIrcEventStruc->dwFlags |= FLAG_CHANNEL_VALID;

#define IRC_CopyText(pIrcEventStruc, pBuffer, uiSize) \
            strncpy(pIrcEventStruc->szText, pBuffer, uiSize); \
			pIrcEventStruc->dwFlags |= FLAG_TEXT_VALID;

#define IRC_CopyChannelCommand(pIrcEventStruc, pBuffer, uiSize) \
            strncpy(pIrcEventStruc->szChannelCommand, pBuffer, uiSize); \
			pIrcEventStruc->dwFlags |= FLAG_CHANNEL_COMMAND_VALID;

#define IRC_CopyNickCommand(pIrcEventStruc, pBuffer, uiSize) \
            strncpy(pIrcEventStruc->szNickCommand, pBuffer, uiSize); \
			pIrcEventStruc->dwFlags |= FLAG_NICK_COMMAND_VALID;

#define IRC_CopyServer(pIrcEventStruc, pBuffer, uiSize) \
            strncpy(pIrcEventStruc->szServer, pBuffer, uiSize); \
			pIrcEventStruc->dwFlags |= FLAG_SERVER_VALID;


/*
 * Internal Prototypes
 */
UINT IRC_ExtractNickOrChannel(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractEvent(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ParsePrivMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractJoinMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractQuitMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ParsePingMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractNickMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractPartMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractTopicMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractKickMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ParseNoticeMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractNumericMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
BOOL IRC_EventIsNumericId(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractPingMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractNoticeMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractCtcpNoticeMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractPrivMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractCtcpMsgMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
UINT IRC_GenericNameExtract(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_GenericExtractMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc, DWORD dwFlags);

/*
 * Supported IRC Events
 */
EVENT_PARSE_DATA g_EventParseData[] =
{
    {"PRIVMSG", NULL, IRC_EVENT_PRIVMSG,       IRC_ParsePrivMessage   },
    {"JOIN",    NULL, IRC_EVENT_JOIN,          IRC_ExtractJoinMessage },
	{"QUIT",    NULL, IRC_EVENT_QUIT,          IRC_ExtractQuitMessage },
	{"PING",    NULL, IRC_EVENT_EMPTY_MESSAGE, IRC_ParsePingMessage   },
	{"NICK",    NULL, IRC_EVENT_NICK,          IRC_ExtractNickMessage },
	{"PART",    NULL, IRC_EVENT_PART,          IRC_ExtractPartMessage },
	{"TOPIC",   NULL, IRC_EVENT_TOPIC,         IRC_ExtractTopicMessage },
	{"KICK",    NULL, IRC_EVENT_KICK,          IRC_ExtractKickMessage },
	{"NOTICE",  NULL, IRC_EVENT_NOTICE,        IRC_ParseNoticeMessage },
	{NULL,      IRC_EventIsNumericId, IRC_EVENT_NUMERIC, IRC_ExtractNumericMessage } 
};




/*******************************************************************************
 * IRC_ExtractMessageDetails                                                   *
 *                                                                             *
 * DESCRIPTION: Extract Message Details                                        *
 *                                                                             *
 * INPUT                                                                       *
 *   Irc Structure, Message, End of Message, Irc Event Structure               *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractMessageDetails(PIRCSTRUC pIrcStruc, PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT uiIndex = 0;
#ifdef _DEBUG
   char *pszDebugMessage = _alloca(uiEndOfMessageIndex + 1);
   strncpy(pszDebugMessage, pBuffer, uiEndOfMessageIndex);

   IRC_Debug("IRC_ExtractMessageDetails - Enter '%s'\r\n", pszDebugMessage);
#endif

   pIrcEventStruc->IrcEventId = IRC_EVENT_EMPTY_MESSAGE;

   uiIndex = IRC_ExtractNickOrChannel(pBuffer, uiEndOfMessageIndex, pIrcEventStruc);

   if(uiIndex != 0)
   {
      uiIndex++;
   }

   if(uiIndex < uiEndOfMessageIndex)
   {
        IRC_ExtractEvent(pBuffer + uiIndex, uiEndOfMessageIndex - uiIndex, pIrcEventStruc);
   }

   IRC_SetIfNickIsSelf(pIrcEventStruc);
   IRC_SetIfNickCommandIsSelf(pIrcEventStruc);
}


/*******************************************************************************
 * IRC_ExtractNickOrChannel                                                    *
 *                                                                             *
 * DESCRIPTION: Extract Nick or Channel                                        *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Index                                                                     *
 *                                                                             *
 *******************************************************************************/
UINT IRC_ExtractNickOrChannel(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT uiIndex = 0, uiHostStartIndex;
   
   IRC_Debug("IRC_ExtractNickOrChannel - Enter\r\n");

   if(pBuffer[uiIndex] == ':')
   {
       if(pBuffer[uiIndex + 1] == '#' || pBuffer[uiIndex + 1] == '&')
       {
           IRC_FindNextWhiteSpace(pBuffer, uiEndOfMessageIndex, uiIndex);
		   IRC_CopyChannel(pIrcEventStruc, pBuffer + 1, uiIndex - 1);
		   IRC_Debug("IRC_ExtractNickOrChannel - Channel = %s\r\n", pIrcEventStruc->szChannel);
       }
       else
       {
           IRC_FindEndOfNick(pBuffer, uiEndOfMessageIndex, uiIndex);
		   IRC_CopyNick(pIrcEventStruc, pBuffer + 1, uiIndex - 1);
           
		   IRC_Debug("IRC_ExtractNickOrChannel - Nick = %s\r\n", pIrcEventStruc->szNick);

           if(pBuffer[uiIndex] == '!')
           {
               uiHostStartIndex = uiIndex + 1;

               IRC_FindNextWhiteSpace(pBuffer, uiEndOfMessageIndex, uiIndex);
		       IRC_CopyHostDetails(pIrcEventStruc, pBuffer + uiHostStartIndex, uiIndex - uiHostStartIndex);
               
			   IRC_Debug("IRC_ExtractNickOrChannel - Host = %s\r\n", pIrcEventStruc->szNickHostDetails);
           }
       }
   }

   return uiIndex;
}




/*******************************************************************************
 * IRC_ExtractEvent                                                            *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractEvent(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT uiEndOfEventIndex = 0, uiEventIndex = 0;
   BOOL bFoundEvent = FALSE;

   IRC_Debug("IRC_ExtractEvent - Enter\r\n");

   IRC_FindNextWhiteSpace(pBuffer, uiEndOfMessageIndex, uiEndOfEventIndex);

   for(uiEventIndex = 0; uiEventIndex < sizeof(g_EventParseData)/sizeof(EVENT_PARSE_DATA) && !bFoundEvent; uiEventIndex++)
   {
       if(g_EventParseData[uiEventIndex].pszEventName)
       {
           if(!strncmp(pBuffer, g_EventParseData[uiEventIndex].pszEventName, uiEndOfEventIndex - 1))
           {
               bFoundEvent = TRUE;
           }
       }
       else
       {
           if(g_EventParseData[uiEventIndex].pfnParseEvent)
           {
               bFoundEvent = g_EventParseData[uiEventIndex].pfnParseEvent(pBuffer, uiEndOfEventIndex - 1, pIrcEventStruc);
           }
       }

       if(bFoundEvent)
       {
           pIrcEventStruc->IrcEventId = g_EventParseData[uiEventIndex].IrcEvent;

           if(g_EventParseData[uiEventIndex].pfnParseEventData && uiEndOfMessageIndex > (uiEndOfEventIndex + 1))
           {
               g_EventParseData[uiEventIndex].pfnParseEventData(pBuffer + uiEndOfEventIndex + 1, uiEndOfMessageIndex - (uiEndOfEventIndex + 1), pIrcEventStruc);
           }
       }
   }
}



/*******************************************************************************
 * IRC_ParsePrivMessage                                                            *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ParsePrivMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
    IRC_Debug("IRC_ParsePrivMessage - Enter\r\n");
	IRC_ExtractPrivMessage(pBuffer, uiEndOfMessageIndex, pIrcEventStruc);
	IRC_ExtractCtcpMsgMessage(pBuffer, uiEndOfMessageIndex, pIrcEventStruc);
}


/*******************************************************************************
 * IRC_EventIsNumericId                                                             *
 *                                                                             *
 * DESCRIPTION: Extract Numeric ID                                             *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   TRUE is Numeric, FALSE is Not Numeric                                     *
 *                                                                             *
 *******************************************************************************/
BOOL IRC_EventIsNumericId(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
    BOOL bEventIsNumericId = FALSE;

    IRC_Debug("IRC_EventIsNumericId - Enter\r\n");

    if(uiEndOfMessageIndex == 2)
	{
       if(isdigit(*pBuffer) && isdigit(*(pBuffer + 1)) && isdigit(*(pBuffer + 2)))
       {
           bEventIsNumericId = TRUE;
           pIrcEventStruc->dwNumericId = (((*pBuffer) - '0')*100) + (((*pBuffer + 1) - '0')*10) + ((*pBuffer + 2) - '0');
		   IRC_Debug("IRC_EventIsNumericId - %i\r\n",  pIrcEventStruc->dwNumericId);
       }
	}

    return bEventIsNumericId;
}



/*******************************************************************************
 * IRC_ExtractCtcpNoticeMessage                                                *
 *                                                                             *
 * DESCRIPTION: Extract CTCP Message From Notice                               *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractCtcpNoticeMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT uiIndex;
   IRC_Debug("IRC_ExtractCtcpNoticeMessage - Enter\r\n");
   if(pIrcEventStruc->szText[0] == 1)
   {
	   uiIndex = 1;
       pIrcEventStruc->IrcEventId = IRC_EVENT_NOTICE_CTCP;

	   for(uiIndex = 1; pIrcEventStruc->szText[uiIndex] != 1 && pIrcEventStruc->szText[uiIndex] != ' '; uiIndex++) ;

/*       if(!strncmp(pIrcEventStruc->Text + 1, "DCC", Index - 1))
       {
       }
       else*/

   }
}




/*******************************************************************************
 * IRC_ExtractCtcpMsgMessage                                                   *
 *                                                                             *
 * DESCRIPTION: Extract CTCP Message From PrivMsg                              *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractCtcpMsgMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT uiIndex;
   IRC_Debug("IRC_ExtractCtcpMsgMessage - Enter\r\n");
   if(pIrcEventStruc->szText[0] == 1)
   {
	   uiIndex = 1;
       pIrcEventStruc->IrcEventId = IRC_EVENT_PRIVMSG_CTCP;

       for(uiIndex = 1; pIrcEventStruc->szText[uiIndex] != 1 && pIrcEventStruc->szText[uiIndex] != ' '; uiIndex++) ;

/*       if(!strncmp(pIrcEventStruc->Text + 1, "DCC", Index - 1))
       {
       }
       else*/

   }
}



/*******************************************************************************
 * IRC_ExtractPingMessage                                                      *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractPingMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT uiIndex = 0;

   IRC_Debug("IRC_ExtractPingMessage - Enter\r\n");

   IRC_FindNextWhiteSpace(pBuffer, uiEndOfMessageIndex, uiIndex);

   IRC_CopyServer(pIrcEventStruc, pBuffer, uiIndex);
   
   IRC_Debug("IRC_ExtractPingMessage - Server = %s\r\n", pIrcEventStruc->szServer);
}


/*******************************************************************************
 * IRC_GenericNameExtract                                                      *
 *                                                                             *
 * DESCRIPTION: Generically extracts a channel or nick                         *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   End Of Name Index                                                         *
 *                                                                             *
 *******************************************************************************/
UINT IRC_GenericNameExtract(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT uiIndex = 0;

   if(*pBuffer == ':')
   {
      pBuffer++;
   }

   IRC_FindNextWhiteSpace(pBuffer, uiEndOfMessageIndex, uiIndex);

   if(IRC_IsIrcChannelName(pBuffer))
   {
	   IRC_CopyChannelCommand(pIrcEventStruc, pBuffer, uiIndex);
	   IRC_Debug("IRC_GenericNameExtract - Channel = %s\r\n", pIrcEventStruc->szChannelCommand);
   }
   else
   {
	   IRC_CopyNickCommand(pIrcEventStruc, pBuffer, uiIndex);
	   IRC_Debug("IRC_GenericNameExtract - Nick = %s\r\n", pIrcEventStruc->szNickCommand);
   }

   return uiIndex;
}




/*******************************************************************************
 * IRC_ExtractPartMessage                                                      *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractPartMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
    IRC_Debug("IRC_ExtractPartMessage - Enter\r\n");
    IRC_GenericNameExtract(pBuffer, uiEndOfMessageIndex, pIrcEventStruc);
}




/*******************************************************************************
 * IRC_ExtractJoinMessage                                                      *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractJoinMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
    IRC_Debug("IRC_ExtractJoinMessage - Enter\r\n");
    IRC_GenericNameExtract(pBuffer, uiEndOfMessageIndex, pIrcEventStruc);
}


/*******************************************************************************
 * IRC_ExtractNickMessage                                                      *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractNickMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
    IRC_Debug("IRC_ExtractNickMessage - Enter\r\n");
    IRC_GenericNameExtract(pBuffer, uiEndOfMessageIndex, pIrcEventStruc);
}



/*******************************************************************************
 * IRC_GenericExtractMessage                                                      *
 *                                                                             *
 * DESCRIPTION: Extract Message                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure, Flags                       *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_GenericExtractMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc, DWORD dwFlags)
{
	UINT uiIndex;
   
	if(IRC_IsMessageMarker(pBuffer))
	{
		IRC_CopyText(pIrcEventStruc, pBuffer + 1, uiEndOfMessageIndex - 1);
		IRC_Debug("IRC_GenericExtractMessage - Text = %s\r\n", pIrcEventStruc->szText);
	}
	else
	{
		uiIndex = IRC_GenericNameExtract(pBuffer, uiEndOfMessageIndex, pIrcEventStruc) + 1;

		if(dwFlags & FLAG_GENERIC_EXTRACT_CHANNEL_AND_NICK)
		{
			uiIndex += IRC_GenericNameExtract(pBuffer+uiIndex, uiEndOfMessageIndex-uiIndex, pIrcEventStruc) + 1;
		}

		if(uiIndex < uiEndOfMessageIndex)
		{
			if(IRC_IsMessageMarker((pBuffer + uiIndex)))
			{
				IRC_CopyText(pIrcEventStruc, pBuffer + uiIndex + 1, uiEndOfMessageIndex - uiIndex - 1);
			}
			else
			{
                IRC_CopyText(pIrcEventStruc, pBuffer + uiIndex, uiEndOfMessageIndex - uiIndex);
			}
			IRC_Debug("IRC_GenericExtractMessage - Text = %s\r\n", pIrcEventStruc->szText);
		}
	}
}

/*******************************************************************************
 * IRC_ExtractPrivMessage                                                      *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractPrivMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   IRC_Debug("IRC_ExtractPrivMessage - Enter\r\n");
   IRC_GenericExtractMessage(pBuffer, uiEndOfMessageIndex, pIrcEventStruc, 0);
}


/*******************************************************************************
 * IRC_ExtractNoticeMessage                                                    *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractNoticeMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   IRC_Debug("IRC_ExtractNoticeMessage - Enter\r\n");
   IRC_GenericExtractMessage(pBuffer, uiEndOfMessageIndex, pIrcEventStruc, 0);
}


/*******************************************************************************
 * IRC_ExtractKickMessage                                                    *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractKickMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
    IRC_Debug("IRC_ExtractKickMessage - Enter\r\n");
	IRC_GenericExtractMessage(pBuffer, uiEndOfMessageIndex, pIrcEventStruc, FLAG_GENERIC_EXTRACT_CHANNEL_AND_NICK);
}
	



/*******************************************************************************
 * IRC_ExtractQuitMessage                                                      *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractQuitMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   IRC_Debug("IRC_ExtractQuitMessage - Enter\r\n");
   IRC_GenericExtractMessage(pBuffer, uiEndOfMessageIndex, pIrcEventStruc, 0);
}


/*******************************************************************************
 * IRC_ExtractTopicMessage                                                    *
 *                                                                             *
 * DESCRIPTION: Extract Event                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractTopicMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   IRC_Debug("IRC_ExtractTopicMessage - Enter\r\n");
   IRC_GenericExtractMessage(pBuffer, uiEndOfMessageIndex, pIrcEventStruc, 0);
}



/*******************************************************************************
 * IRC_ParsePingMessage                                                    *
 *                                                                             *
 * DESCRIPTION: Parse Ping and Send Reply                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ParsePingMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   IRC_Debug("IRC_ParsePingMessage - Enter\r\n");
   IRC_ExtractPingMessage(pBuffer, uiEndOfMessageIndex, pIrcEventStruc);
   IRC_SendPongMessage(pIrcEventStruc->hIrc, pIrcEventStruc->szServer);
}


/*******************************************************************************
 * IRC_ParseNoticeMessage                                                    *
 *                                                                             *
 * DESCRIPTION: Parse a NOTICE                                                  *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ParseNoticeMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   IRC_Debug("IRC_ParseNoticeMessage - Enter\r\n");
   IRC_ExtractNoticeMessage(pBuffer, uiEndOfMessageIndex, pIrcEventStruc);
   IRC_ExtractCtcpNoticeMessage(pBuffer, uiEndOfMessageIndex, pIrcEventStruc);
}



/*******************************************************************************
 * IRC_ExtractNumericMessage                                                   *
 *                                                                             *
 * DESCRIPTION: Extract Numeric Message                                        *
 *                                                                             *
 * INPUT                                                                       *
 *   Message, End of Message, Irc Event Structure                              *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Nothing                                                                   *
 *                                                                             *
 *******************************************************************************/
void IRC_ExtractNumericMessage(PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT uiIndex = 0;
   IRC_Debug("IRC_ExtractNumericMessage - Enter\r\n");
   
   IRC_FindNextWhiteSpace(pBuffer, uiEndOfMessageIndex, uiIndex);
   
   if(uiIndex < uiEndOfMessageIndex)
   {
	   uiIndex = IRC_GenericNameExtract(pBuffer, uiEndOfMessageIndex, pIrcEventStruc) + 1;

	   if(uiIndex < uiEndOfMessageIndex)
	   {
		   pBuffer += uiIndex;
		   uiEndOfMessageIndex -= uiIndex;

		   if(IRC_IsMessageMarker(pBuffer))
		   {
			   IRC_CopyText(pIrcEventStruc, pBuffer + 1, uiEndOfMessageIndex - 1);
		   }
		   else
		   {
			   uiIndex = IRC_GenericNameExtract(pBuffer, uiEndOfMessageIndex, pIrcEventStruc) + 1;

			   if(uiIndex < uiEndOfMessageIndex)
			   {
				   pBuffer += uiIndex;
				   uiEndOfMessageIndex -= uiIndex;

				   if(IRC_IsMessageMarker(pBuffer))
				   {
					   IRC_CopyText(pIrcEventStruc, pBuffer + 1, uiEndOfMessageIndex - 1);
				   }
				   else
				   {
					   IRC_CopyText(pIrcEventStruc, pBuffer, uiEndOfMessageIndex);
				   }
				   IRC_Debug("IRC_ExtractNumericMessage - Text = %s\r\n", pIrcEventStruc->szText);
			   }
		   }
	   }
   }
   else
   {
	   if(IRC_IsMessageMarker(pBuffer))
	   {
		   IRC_CopyText(pIrcEventStruc, pBuffer + 1, uiEndOfMessageIndex - 1);
	   }
	   else
	   {
		   IRC_CopyText(pIrcEventStruc, pBuffer, uiEndOfMessageIndex);
	   }
	   IRC_Debug("IRC_ExtractNumericMessage - Text = %s\r\n", pIrcEventStruc->szText);
   }
}




