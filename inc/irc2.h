/***************************************************************************** 
 *                         IRC Library   Version 2.0                         *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2002           Toby Opferman                              *
 *****************************************************************************/


#ifndef __IRC2_H__
#define __IRC2_H__


#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NICK         100
#define MAX_CHANNEL      256
#define MAX_CHAT_TEXT    512
#define MAX_HOST         256
#define MAX_TOPIC        256
#define MAX_REALNAME     256
#define MAX_USERNAME     256
#define MAX_MESSAGE_SIZE 1000

typedef enum _IRC_EVENT { 
               IRC_EVENT_EMPTY_MESSAGE = 0,
               IRC_EVENT_DISCONNECT,
               IRC_EVENT_CONNECT,
               IRC_EVENT_JOIN, 
               IRC_EVENT_PART,  
               IRC_EVENT_QUIT,
               IRC_EVENT_NICK,
               IRC_EVENT_TOPIC,  
               IRC_EVENT_KICK,
               IRC_EVENT_PRIVMSG, 
               IRC_EVENT_NOTICE,
               IRC_EVENT_PING,
               IRC_EVENT_NOTICE_CTCP, 
               IRC_EVENT_PRIVMSG_CTCP,
               IRC_EVENT_NUMERIC,
               IRC_EVENT_MAXIMUM /* Must Be Last, Not A Real Event */
} IRC_EVENT, *PIRC_EVENT;


typedef enum _IRC_CTCP {
               IRC_CTCP_UNKNOWN = 0, 
               IRC_CTCP_VERSION,
               IRC_CTCP_PING,  
               IRC_CTCP_SOURCE,
               IRC_CTCP_USERINFO, 
               IRC_CTCP_ERRMSG,
               IRC_CTCP_TIME, 
               IRC_CTCP_DCC_SEND,
               IRC_CTCP_DCC_GET, 
               IRC_CTCP_DCC_CHAT,
               IRC_CTCP_ACTION

} IRC_CTCP, *PIRC_CTCP;


typedef enum { IRC_SUCCESS, IRC_FAILURE } IRCSTATUS;
typedef PVOID HIRC;



typedef struct _IRC_USER {
    
    char szNick[MAX_NICK + 1];
    char szRealName[MAX_REALNAME + 1];
    char szUserName[MAX_USERNAME + 1];

    /* Can be left blank, Will be filled in */
    char szHostName[MAX_HOST + 1];
    char szServerName[MAX_HOST + 1];

} IRC_USER, *PIRC_USER;

/*
 * What data is valid in the event
 */
#define FLAG_NICK_VALID               0x01
#define FLAG_NICK_HOST_DETAILS_VALID  0x02
#define FLAG_CHANNEL_VALID            0x04
#define FLAG_TEXT_VALID               0x08
#define FLAG_CHANNEL_COMMAND_VALID    0x10
#define FLAG_NICK_COMMAND_VALID       0x20
#define FLAG_SERVER_VALID             0x40

typedef struct _IRC_EVENT_STRUC{
    
    HIRC      hIrc;

    IRC_EVENT IrcEventId;
    IRC_CTCP  IrcCtcpId;
    DWORD     dwNumericId;

    IRC_USER  IrcUser;

    char      szNick[MAX_NICK + 1];
    char      szNickHostDetails[MAX_HOST + 1];
    char      szChannel[MAX_CHANNEL + 1];
    
    char      szText[MAX_CHAT_TEXT + 1];
    char      szChannelCommand[MAX_CHANNEL + 1];
    char      szNickCommand[MAX_NICK + 1];
    char      szServer[MAX_HOST + 1];

    BOOL      bNickIsSelf;
    BOOL      bNickCommandIsSelf;

    DWORD     dwFlags;
    
} IRC_EVENT_STRUC, *PIRC_EVENT_STRUC;


typedef void (*PIRC_EVENT_CALLBACK)(PVOID, PIRC_EVENT_STRUC);

#define IRC_SetAllCallbacks(pEventCallbackList, pfnEventCallback) \
              { UINT uiIndex = 0; \
                for(;uiIndex < IRC_EVENT_MAXIMUM; uiIndex++) \
                { \
                    (pEventCallbackList)->pfnIrcEventCallback[uiIndex] = 	pfnEventCallback; \
               }}

#define IRC_SetCallback(pEventCallbackList, pfnEventCallback, uiCallbackIndex)  \
             (pEventCallbackList)->pfnIrcEventCallback[uiCallbackIndex] = pfnEventCallback;
           

typedef struct _EVENT_CALLBACKS {
    
    PIRC_EVENT_CALLBACK pfnIrcEventCallback[IRC_EVENT_MAXIMUM];		

} EVENT_CALLBACKS, *PEVENT_CALLBACKS;

#define INVALID_IRC_HANDLE (HIRC)NULL

HIRC IRC_ConnectToServer(PCHAR IrcServer, USHORT IrcPort, PIRC_USER pIrcUser, PEVENT_CALLBACKS pIrcCallbacks, PVOID pContext);
IRCSTATUS IRC_ChangeNick(HIRC hIrc, PCHAR pszNick);
IRCSTATUS IRC_SendQuitMessage(HIRC hIrc, PCHAR pszQuitText);
IRCSTATUS IRC_SendPongMessage(HIRC hIrc, PCHAR pszServer);
IRCSTATUS IRC_JoinChannel(HIRC hIrc, PCHAR pszChannel);
IRCSTATUS IRC_PartChannel(HIRC hIrc, PCHAR pszChannel);
IRCSTATUS IRC_NoticeNick(HIRC hIrc, PCHAR pszNick, PCHAR pszText);
IRCSTATUS IRC_MsgNick(HIRC hIrc, PCHAR pszNick, PCHAR pszText);
IRCSTATUS IRC_NoticeChannel(HIRC hIrc, PCHAR Nick, PCHAR pszText);
IRCSTATUS IRC_MsgChannel(HIRC hIrc, PCHAR pszNick, PCHAR pszText);
IRCSTATUS IRC_SendUserData(HIRC hIrc, PIRC_USER pIrcUser);
void IRC_Destroy(HIRC hIrc);

#ifdef __cplusplus
}
#endif


#endif

 