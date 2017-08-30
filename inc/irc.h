/***************************************************************************** 
 *                         IRC Library                                       *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2001           Toby Opferman                              *
 *****************************************************************************/


#ifndef __IRC_H__
#define __IRC_H__


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

typedef enum { IRC_EVENT_DISCONNECT, IRC_EVENT_EMPTY_MESSAGE,
               IRC_EVENT_JOIN, IRC_EVENT_PART, IRC_EVENT_QUIT,
               IRC_EVENT_NICK, IRC_EVENT_TOPIC, IRC_EVENT_KICK,
               IRC_EVENT_PRIVMSG, IRC_EVENT_NOTICE, IRC_EVENT_PING,
               IRC_EVENT_NOTICE_CTCP, IRC_EVENT_PRIVMSG_CTCP,
               IRC_EVENT_NUMERIC

} IRC_EVENT;


typedef enum {

               IRC_CTCP_UNKNOWN, IRC_CTCP_VERSION,
               IRC_CTCP_PING, IRC_CTCP_SOURCE,
               IRC_CTCP_USERINFO, IRC_CTCP_ERRMSG,
               IRC_CTCP_TIME, IRC_CTCP_DCC_SEND,
               IRC_CTCP_DCC_GET, IRC_CTCP_DCC_CHAT,
               IRC_CTCP_ACTION

} IRC_CTCP;


typedef enum { IRC_SUCCESS, IRC_FAILURE } IRCSTATUS;
typedef PVOID HIRC;



typedef struct {
    
    char Nick[MAX_NICK + 1];
    char RealName[MAX_REALNAME + 1];
    char UserName[MAX_USERNAME + 1];

    /* Can be left blank, Will be filled in */
    char HostName[MAX_HOST + 1];
    char ServerName[MAX_HOST + 1];

} IRC_USER, *PIRC_USER;

typedef struct {
    
    /* Handle to this Irc Instance             */
    HIRC      hIrc;

    /* Response and Command Information        */
    IRC_EVENT IrcEventId;
    IRC_CTCP  IrcCtcpId;
    DWORD     NumericId;

    /* Your User Information                   */
    IRC_USER  IrcUser;

    /* Depends upon Pre-Command Information    */
    char      Nick[MAX_NICK + 1];
    char      NickHostDetails[MAX_HOST + 1];
    char      Channel[MAX_CHANNEL + 1];
    
    /* Depends upon Command                    */
    char      Text[MAX_CHAT_TEXT + 1];
    char      ChannelCommand[MAX_CHANNEL + 1];
    char      NickCommand[MAX_NICK + 1];
    char      Server[MAX_HOST + 1];
    
} IRC_EVENT_STRUC, *PIRC_EVENT_STRUC;


typedef void (*IRC_EVENT_CALLBACK)(PVOID, PIRC_EVENT_STRUC);

#define INVALID_IRC_HANDLE (HIRC)NULL

HIRC IRC_ConnectToServer(PCHAR IrcServer, USHORT IrcPort, PIRC_USER pIrcUser, IRC_EVENT_CALLBACK IrcCallback, PVOID pContext);
IRCSTATUS IRC_ChangeNick(HIRC hIrc, PCHAR Nick);
IRCSTATUS IRC_SendQuitMessage(HIRC hIrc, PCHAR QuitText);
IRCSTATUS IRC_SendPongMessage(HIRC hIrc, PCHAR Server);
IRCSTATUS IRC_JoinChannel(HIRC hIrc, PCHAR Channel);
IRCSTATUS IRC_PartChannel(HIRC hIrc, PCHAR Channel);
IRCSTATUS IRC_NoticeNick(HIRC hIrc, PCHAR Nick, PCHAR Text);
IRCSTATUS IRC_MsgNick(HIRC hIrc, PCHAR Nick, PCHAR Text);
IRCSTATUS IRC_NoticeChannel(HIRC hIrc, PCHAR Nick, PCHAR Text);
IRCSTATUS IRC_MsgChannel(HIRC hIrc, PCHAR Nick, PCHAR Text);
IRCSTATUS IRC_SendUserData(HIRC hIrc, PIRC_USER pIrcUser);
void IRC_Destroy(HIRC hIrc);

#ifdef __cplusplus
}
#endif


#endif

 