/***************************************************************************** 
 *                             IRC Library Version 2.0                       *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2001-2005      Toby Opferman                              *
 *****************************************************************************/


#ifndef __IRC_INTERNAL_H__
#define __IRC_INTERNAL_H__

#define IRC_FindEndOfMessage(pBuffer,uiLength,uiEndOfMessage) \
                for(; pBuffer[uiEndOfMessage] != '\r' &&  \
                                        pBuffer[uiEndOfMessage] != '\n' &&  \
                                        uiEndOfMessage < uiLength; uiEndOfMessage++) ;

#define IRC_IsEndOfMessage(pBuffer, uiEndOfMessage) (pBuffer[uiEndOfMessage] == '\r' || pBuffer[uiEndOfMessage] == '\n')

#define IRC_SkipToNextMessage(pBuffer,uiLength,uiEndOfMessage) \
                for(; (pBuffer[uiEndOfMessage] == '\r' ||  \
                      pBuffer[uiEndOfMessage] == '\n') &&  \
                      uiEndOfMessage < uiLength; uiEndOfMessage++) ;


#define IRC_FindNextWhiteSpace(pBuffer,uiEndOfMessage,uiIndex) \
                             for(;pBuffer[uiIndex] != ' ' && uiIndex < uiEndOfMessage; uiIndex++) ;


#define IRC_FindEndOfNick(pBuffer,uiEndOfMessage,uiIndex) \
                             for(;pBuffer[uiIndex] != ' ' && pBuffer[uiIndex] != '!' && uiIndex < uiEndOfMessage; uiIndex++) ;


#define IRC_IsIrcChannelName(pBuffer) (pBuffer[0] == '#' || pBuffer[0] == '&')


#define IRC_IsMessageMarker(pBuffer) (pBuffer[0] == ':')

#define IRC_SetIfNickIsSelf(pIrcEventStruc) \
	if(!_stricmp(pIrcEventStruc->szNick, pIrcEventStruc->IrcUser.szNick))\
    {\
		pIrcEventStruc->bNickIsSelf = TRUE;\
	}

#define IRC_SetIfNickCommandIsSelf(pIrcEventStruc) \
	if(!_stricmp(pIrcEventStruc->szNickCommand, pIrcEventStruc->IrcUser.szNick))\
    {\
		pIrcEventStruc->bNickCommandIsSelf = TRUE;\
	}

typedef struct _IRCSTRUC {

    DTL_SOCKET            DtlSocket;
    EVENT_CALLBACKS       IrcCallBacks;
    PVOID                 pContext;
    HANDLE                hThread;
    DWORD                 ThreadID;
    IRC_USER              IrcUser;

} IRCSTRUC, *PIRCSTRUC;

#define IRC_BUFFER_SIZE   15000
#define IRC_SEND_MSG    0
#define IRC_SEND_NOTICE 1

/*
 * Internal Prototypes
 */
IRCSTATUS IRC_SendText(HIRC hIrc, PCHAR pszTarget, PCHAR pszText, UINT uiTypeFlags);
DWORD WINAPI IRC_InternalEngine(PVOID pContext);
void IRC_ExtractMessageDetails(PIRCSTRUC pIrcStruc, PCHAR pBuffer, UINT uiEndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);


#ifdef _DEBUG
void IRC_Debug(char *pszFormatString, ...);
#else
#define IRC_Debug
#endif 

#endif

