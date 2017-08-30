/***************************************************************************** 
 *                             IRC Library                                   *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *    Copyright (c)2001           Toby Opferman                              *
 *****************************************************************************/

#include <windows.h>
#include <winsock.h>
#include <dtl.h>
#include <irc.h>
#include <stdio.h>

/***************************************************************************** 
 * Internal Data Structure                                                   *
 *****************************************************************************/
typedef struct {

    DTL_SOCKET            DtlSocket;
    IRC_EVENT_CALLBACK    IrcCallback;
    PVOID                 pContext;
    HANDLE                hThread;
    DWORD                 ThreadID;
    IRC_USER              IrcUser;
    char                  PendingNick[30];

} IRCSTRUC, *PIRCSTRUC;

#define IRC_BUFFER_SIZE   15000

/*******************************************************************************
 * Internal Prototypes                                                         *
 *******************************************************************************/
DWORD WINAPI IRC_InternalEngine(PVOID pContext);
void IRC_ParseIncommingData(PIRCSTRUC pIrcStruc, PCHAR pBuffer, UINT *Index);
void IRC_ExtractMessageDetails(PIRCSTRUC pIrcStruc, PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
UINT IRC_ExtractNickOrChannel(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractEvent(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractPrivMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractNoticeMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractPingMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractJoinMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractPartMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractNickMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractNumericMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractTopicMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractQuitMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractKickMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractCtcpMsgMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
void IRC_ExtractCtcpNoticeMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);
BOOL IRC_IsNumericId(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc);

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
HIRC IRC_ConnectToServer(PCHAR IrcServer, USHORT IrcPort, PIRC_USER pIrcUser, IRC_EVENT_CALLBACK IrcCallback, PVOID pContext)
{
    PIRCSTRUC pIrcStruc = (PIRCSTRUC)INVALID_IRC_HANDLE;

    if(pIrcStruc = LocalAlloc(LMEM_ZEROINIT, sizeof(IRCSTRUC)))
    {
           if(pIrcStruc->DtlSocket = Dtl_Create_Client(IrcServer, IrcPort, 0))
           {
               pIrcStruc->IrcCallback = IrcCallback;
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
    DWORD ExitCode;

    Dtl_Destroy(pIrcStruc->DtlSocket);
    
    while(GetExitCodeThread(pIrcStruc->hThread, &ExitCode) == STILL_ACTIVE) ;

    LocalFree(pIrcStruc);
}



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
    char IrcBuffer[IRC_BUFFER_SIZE] = {0};
    UINT Index = 0, RetCode;
    IRC_EVENT_STRUC IrcEventStruc = {0};

    IRC_ChangeNick((HIRC)pIrcStruc, pIrcStruc->IrcUser.Nick);
    IRC_SendUserData((HIRC)pIrcStruc, &pIrcStruc->IrcUser);

    while((DtlStatus = Dtl_SocketWaitForRead(pIrcStruc->DtlSocket, 1)) != DTLSOCKET_ERROR)
    {
        if(DtlStatus == DTLSOCKET_DATA)
        {
            RetCode = Dtl_ReadSocketData(pIrcStruc->DtlSocket, IrcBuffer + Index, IRC_BUFFER_SIZE - Index);

            if(RetCode && RetCode != DTL_SOCKET_ERROR)
            {
                Index += RetCode;
                IRC_ParseIncommingData(pIrcStruc, IrcBuffer, &Index);
            }
            else
              Dtl_Destroy(pIrcStruc->DtlSocket);

        }
        else if(Index)
                IRC_ParseIncommingData(pIrcStruc, IrcBuffer, &Index);
    }

    IrcEventStruc.IrcEventId = IRC_EVENT_DISCONNECT;
    IrcEventStruc.hIrc       = (HIRC)pIrcStruc;

    pIrcStruc->IrcCallback(pIrcStruc->pContext, &IrcEventStruc);

    ExitThread(0);
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
void IRC_ParseIncommingData(PIRCSTRUC pIrcStruc, PCHAR pBuffer, UINT *Index)
{
    UINT EndOfMessageIndex = 0;
    IRC_EVENT_STRUC IrcEventStruc = {0};

    memcpy(&IrcEventStruc.IrcUser, &pIrcStruc->IrcUser, sizeof(IRC_USER));

    if(*Index)
    {
       while(pBuffer[EndOfMessageIndex] != '\r' && pBuffer[EndOfMessageIndex] != '\n' && *Index > EndOfMessageIndex)
           EndOfMessageIndex++;

       if(*Index >= (EndOfMessageIndex))
       {
           IrcEventStruc.hIrc = (HIRC)pIrcStruc;
           IRC_ExtractMessageDetails(pIrcStruc, pBuffer, EndOfMessageIndex, &IrcEventStruc);

           if(IrcEventStruc.IrcEventId != IRC_EVENT_EMPTY_MESSAGE)
               pIrcStruc->IrcCallback(pIrcStruc->pContext, &IrcEventStruc);
           
           while((pBuffer[EndOfMessageIndex] == '\r' || pBuffer[EndOfMessageIndex] == '\n') && *Index > EndOfMessageIndex)
               EndOfMessageIndex++;


           if(*Index > (EndOfMessageIndex))
           {
                *Index -= (EndOfMessageIndex);
                strncpy(pBuffer, pBuffer + EndOfMessageIndex, *Index);
           }
           else
             *Index = 0;
       }
    }
}





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
void IRC_ExtractMessageDetails(PIRCSTRUC pIrcStruc, PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;

   pIrcEventStruc->IrcEventId = IRC_EVENT_EMPTY_MESSAGE;

   if(Index = IRC_ExtractNickOrChannel(pBuffer, EndOfMessageIndex, pIrcEventStruc))
      Index++;

   if(Index < EndOfMessageIndex)
        IRC_ExtractEvent(pBuffer + Index, EndOfMessageIndex - Index, pIrcEventStruc);

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
UINT IRC_ExtractNickOrChannel(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0, HoldIndex;

   if(pBuffer[Index] == ':')
   {
       if(pBuffer[Index + 1] == '#' || pBuffer[Index + 1] == '&')
       {
           while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
               Index++;

           strncpy(pIrcEventStruc->Channel, pBuffer + 1, Index - 1);
       }
       else
       {
           while(pBuffer[Index] != ' ' && pBuffer[Index] != '!' && Index < EndOfMessageIndex)
               Index++;

           strncpy(pIrcEventStruc->Nick, pBuffer + 1, Index - 1);

           HoldIndex = Index + 1;

           if(pBuffer[Index] == '!')
           {
               while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
                    Index++;

               strncpy(pIrcEventStruc->NickHostDetails, pBuffer + HoldIndex, Index - HoldIndex);
           }
       }
   }

   return Index;
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
void IRC_ExtractEvent(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;

   while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
       Index++;

   if(!strncmp(pBuffer, "PRIVMSG", Index - 1))
   {
      pIrcEventStruc->IrcEventId = IRC_EVENT_PRIVMSG;
 
      if(EndOfMessageIndex > (Index + 1))
      {
        IRC_ExtractPrivMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);
        IRC_ExtractCtcpMsgMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);
      }
   }
   else
     if(!strncmp(pBuffer, "JOIN", Index - 1))
     {
         pIrcEventStruc->IrcEventId = IRC_EVENT_JOIN;

         if(EndOfMessageIndex > (Index + 1))
           IRC_ExtractJoinMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);
     } 
     else 
      if(!strncmp(pBuffer, "QUIT", Index - 1))
      {
         pIrcEventStruc->IrcEventId = IRC_EVENT_QUIT;
  
         if(EndOfMessageIndex > (Index + 1))
           IRC_ExtractQuitMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);
      }
      else
         if(!strncmp(pBuffer, "PING", Index - 1))
         {
            if(EndOfMessageIndex > (Index + 1))
            {
              IRC_ExtractPingMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);
              IRC_SendPongMessage(pIrcEventStruc->hIrc, pIrcEventStruc->Server);
            }
         }
         else
            if(!strncmp(pBuffer, "NICK", Index - 1))
            {
                pIrcEventStruc->IrcEventId = IRC_EVENT_NICK;

                if(EndOfMessageIndex > (Index + 1))
                   IRC_ExtractNickMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);
            }
            else
               if(!strncmp(pBuffer, "PART", Index - 1))
               {
                    pIrcEventStruc->IrcEventId = IRC_EVENT_PART;

                    if(EndOfMessageIndex > (Index + 1))
                        IRC_ExtractPartMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);

               }
               else
                  if(!strncmp(pBuffer, "TOPIC", Index - 1))
                  {
                      pIrcEventStruc->IrcEventId = IRC_EVENT_TOPIC;
 
                      if(EndOfMessageIndex > (Index + 1))
                         IRC_ExtractTopicMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);
                  }
                  else
                     if(!strncmp(pBuffer, "KICK", Index - 1))
                     {
                          pIrcEventStruc->IrcEventId = IRC_EVENT_KICK;

                          if(EndOfMessageIndex > (Index + 1))
                             IRC_ExtractKickMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);

                     }
                     else
                       if(!strncmp(pBuffer, "NOTICE", Index - 1))
                       {
                            pIrcEventStruc->IrcEventId = IRC_EVENT_NOTICE;

                            if(EndOfMessageIndex > (Index + 1))
                            {
                               IRC_ExtractNoticeMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);
                               IRC_ExtractCtcpNoticeMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);
                            }
                       }
                       else
                          if(IRC_IsNumericId(pBuffer, Index - 1, pIrcEventStruc))
                          {
                              pIrcEventStruc->IrcEventId = IRC_EVENT_NUMERIC;

                              if(EndOfMessageIndex > (Index + 1))
                                 IRC_ExtractNumericMessage(pBuffer + Index + 1, EndOfMessageIndex - (Index + 1), pIrcEventStruc);
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
void IRC_ExtractPrivMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;
   
   if(*pBuffer == ':')
       strncpy(pIrcEventStruc->Text, pBuffer + 1, EndOfMessageIndex - 1);
   else
   {
      while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
         Index++;

      if(*pBuffer == '#' || *pBuffer == '&')
          strncpy(pIrcEventStruc->ChannelCommand, pBuffer, Index);
      else
          strncpy(pIrcEventStruc->NickCommand, pBuffer, Index);

      Index++;

      if(Index < EndOfMessageIndex)
      {
          if(pBuffer[Index] == ':')
             strncpy(pIrcEventStruc->Text, pBuffer + Index + 1, EndOfMessageIndex - Index - 1);
          else
             strncpy(pIrcEventStruc->Text, pBuffer + Index, EndOfMessageIndex - Index);
      }

   }

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
void IRC_ExtractNoticeMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;
   
   if(*pBuffer == ':')
       strncpy(pIrcEventStruc->Text, pBuffer + 1, EndOfMessageIndex - 1);
   else
   {
      while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
         Index++;

      if(*pBuffer == '#' || *pBuffer == '&')
          strncpy(pIrcEventStruc->ChannelCommand, pBuffer, Index);
      else
          strncpy(pIrcEventStruc->NickCommand, pBuffer, Index);

      Index++;

      if(Index < EndOfMessageIndex)
      {
          if(pBuffer[Index] == ':')
             strncpy(pIrcEventStruc->Text, pBuffer + Index + 1, EndOfMessageIndex - Index - 1);
          else
             strncpy(pIrcEventStruc->Text, pBuffer + Index, EndOfMessageIndex - Index);
      }

   }

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
void IRC_ExtractKickMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;
   
   if(*pBuffer == ':')
       strncpy(pIrcEventStruc->Text, pBuffer + 1, EndOfMessageIndex - 1);
   else
   {
      while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
         Index++;

      if(*pBuffer == '#' || *pBuffer == '&')
          strncpy(pIrcEventStruc->ChannelCommand, pBuffer, Index);
      else
          strncpy(pIrcEventStruc->NickCommand, pBuffer, Index);

      Index++;

      while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
         Index++;

      if(*pBuffer == '#' || *pBuffer == '&')
          strncpy(pIrcEventStruc->ChannelCommand, pBuffer, Index);
      else
          strncpy(pIrcEventStruc->NickCommand, pBuffer, Index);


      Index++;

      if(Index < EndOfMessageIndex)
      {
          if(pBuffer[Index] == ':')
             strncpy(pIrcEventStruc->Text, pBuffer + Index + 1, EndOfMessageIndex - Index - 1);
          else
             strncpy(pIrcEventStruc->Text, pBuffer + Index, EndOfMessageIndex - Index);
      }

   }

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
void IRC_ExtractQuitMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;
   
   if(*pBuffer == ':')
       strncpy(pIrcEventStruc->Text, pBuffer + 1, EndOfMessageIndex - 1);
   else
   {
      while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
         Index++;

      if(*pBuffer == '#' || *pBuffer == '&')
          strncpy(pIrcEventStruc->ChannelCommand, pBuffer, Index);
      else
          strncpy(pIrcEventStruc->NickCommand, pBuffer, Index);

      Index++;

      if(Index < EndOfMessageIndex)
      {
          if(pBuffer[Index] == ':')
             strncpy(pIrcEventStruc->Text, pBuffer + Index + 1, EndOfMessageIndex - Index - 1);
          else
             strncpy(pIrcEventStruc->Text, pBuffer + Index, EndOfMessageIndex - Index);
      }

   }

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
void IRC_ExtractTopicMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;
   
   if(*pBuffer == ':')
       strncpy(pIrcEventStruc->Text, pBuffer + 1, EndOfMessageIndex - 1);
   else
   {
      while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
         Index++;

      if(*pBuffer == '#' || *pBuffer == '&')
          strncpy(pIrcEventStruc->ChannelCommand, pBuffer, Index);
      else
          strncpy(pIrcEventStruc->NickCommand, pBuffer, Index);

      Index++;

      if(Index < EndOfMessageIndex)
      {
          if(pBuffer[Index] == ':')
             strncpy(pIrcEventStruc->Text, pBuffer + Index + 1, EndOfMessageIndex - Index - 1);
          else
             strncpy(pIrcEventStruc->Text, pBuffer + Index, EndOfMessageIndex - Index);
      }

   }

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
void IRC_ExtractPartMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;

   if(*pBuffer == ':')
     pBuffer++;
   
   while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
       Index++;

   if(*pBuffer == '#' || *pBuffer == '&')
       strncpy(pIrcEventStruc->ChannelCommand, pBuffer, Index);
   else
       strncpy(pIrcEventStruc->NickCommand, pBuffer, Index);

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
void IRC_ExtractJoinMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;

   if(*pBuffer == ':')
     pBuffer++;
   
   while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
       Index++;


   if(*pBuffer == '#' || *pBuffer == '&')
       strncpy(pIrcEventStruc->ChannelCommand, pBuffer, Index);
   else
       strncpy(pIrcEventStruc->NickCommand, pBuffer, Index);

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
void IRC_ExtractNickMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;

   if(*pBuffer == ':')
     pBuffer++;
   
   while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
       Index++;


   if(*pBuffer == '#' || *pBuffer == '&')
       strncpy(pIrcEventStruc->ChannelCommand, pBuffer, Index);
   else
       strncpy(pIrcEventStruc->NickCommand, pBuffer, Index);

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
void IRC_ExtractPingMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0;
   
   while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
        Index++;

   strncpy(pIrcEventStruc->Server, pBuffer, Index);

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
IRCSTATUS IRC_ChangeNick(HIRC hIrc, PCHAR Nick)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char Message[MAX_MESSAGE_SIZE + 1];

   strcpy(pIrcStruc->PendingNick, Nick);
   sprintf(Message, "NICK %s\r\n", Nick);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, Message, strlen(Message));

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
IRCSTATUS IRC_SendQuitMessage(HIRC hIrc, PCHAR QuitText)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char Message[MAX_MESSAGE_SIZE + 1];

   sprintf(Message, "QUIT :%s\r\n", QuitText);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, Message, strlen(Message));

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
IRCSTATUS IRC_SendPongMessage(HIRC hIrc, PCHAR Server)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char Message[MAX_MESSAGE_SIZE + 1];

   sprintf(Message, "PONG %s\r\n", Server);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, Message, strlen(Message));

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
IRCSTATUS IRC_JoinChannel(HIRC hIrc, PCHAR Channel)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char Message[MAX_MESSAGE_SIZE + 1];

   sprintf(Message, "JOIN %s\r\n", Channel);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, Message, strlen(Message));

   return IRC_SUCCESS;
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
IRCSTATUS IRC_PartChannel(HIRC hIrc, PCHAR Channel)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char Message[MAX_MESSAGE_SIZE + 1];

   sprintf(Message, "PART %s\r\n", Channel);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, Message, strlen(Message));

   return IRC_SUCCESS;
}


/*******************************************************************************
 * IRC_NoticeNick                                                              *
 *                                                                             *
 * DESCRIPTION: Notice Nickname                                                *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Nick, Text                                                 *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_NoticeNick(HIRC hIrc, PCHAR Nick, PCHAR Text)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char Message[MAX_MESSAGE_SIZE + 1], MText[MAX_MESSAGE_SIZE + 1];
   UINT Index = 0, BegIndex = 0, Index2;

   while(Text[Index])
   {
      Index2 = 0;
      while(Text[Index] && Text[Index] != '\n' && Text[Index] != '\r')
      {
         MText[Index2] = Text[Index];
         Index2++;
         Index++;
      }

      MText[Index2] = 0;


      sprintf(Message, "NOTICE %s :%s\r\n", Nick, MText);

      Dtl_SendSocketData(pIrcStruc->DtlSocket, Message, strlen(Message));


      while(Text[Index] == '\r' || Text[Index] == '\n')
          Index++;

      BegIndex = Index;
   }

   return IRC_SUCCESS;
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
IRCSTATUS IRC_NoticeChannel(HIRC hIrc, PCHAR Channel, PCHAR Text)
{
   return IRC_NoticeNick(hIrc, Channel, Text);
}



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
   char Message[MAX_MESSAGE_SIZE + 1];

   sprintf(Message, "USER %s %s %s :%s\r\n", pIrcUser->UserName, pIrcUser->HostName, pIrcUser->ServerName, pIrcUser->RealName);

   Dtl_SendSocketData(pIrcStruc->DtlSocket, Message, strlen(Message));

   return IRC_SUCCESS;
}


/*******************************************************************************
 * IRC_MsgNick                                                                 *
 *                                                                             *
 * DESCRIPTION: Msg Nickname                                                   *
 *                                                                             *
 * INPUT                                                                       *
 *   Handle to IRC, Nick, Text                                                 *
 *                                                                             *
 * OUTPUT                                                                      * 
 *   Irc Status                                                                *
 *                                                                             *
 *******************************************************************************/
IRCSTATUS IRC_MsgNick(HIRC hIrc, PCHAR Nick, PCHAR Text)
{
   PIRCSTRUC pIrcStruc = (PIRCSTRUC)hIrc;
   char Message[MAX_MESSAGE_SIZE + 1], TempChar, TextMessage[MAX_MESSAGE_SIZE + 1];
   UINT Index = 0, BegIndex = 0, Index2;

   while(Text[Index])
   {
      Index2 = 0;

      while(Text[Index] && Text[Index] != '\n' && Text[Index] != '\r')
      {
         TextMessage[Index2] = Text[Index];
         Index++;
         Index2++;
      }


      TextMessage[Index2] = 0;

      sprintf(Message, "PRIVMSG %s :%s\r\n", Nick, TextMessage);

      Dtl_SendSocketData(pIrcStruc->DtlSocket, Message, strlen(Message));



      while(Text[Index] == '\r' || Text[Index] == '\n')
          Index++;

      BegIndex = Index;
   }

   return IRC_SUCCESS;
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
IRCSTATUS IRC_MsgChannel(HIRC hIrc, PCHAR Channel, PCHAR Text)
{
   return IRC_MsgNick(hIrc, Channel, Text);
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
void IRC_ExtractCtcpMsgMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 1;

   if(pIrcEventStruc->Text[0] == 1)
   {
       pIrcEventStruc->IrcEventId = IRC_EVENT_PRIVMSG_CTCP;

       while(pIrcEventStruc->Text[Index] != 1 && pIrcEventStruc->Text[Index] != ' ')
            Index++;

/*       if(!strncmp(pIrcEventStruc->Text + 1, "DCC", Index - 1))
       {
       }
       else*/

   }
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
void IRC_ExtractCtcpNoticeMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 1;

   if(pIrcEventStruc->Text[0] == 1)
   {
       pIrcEventStruc->IrcEventId = IRC_EVENT_NOTICE_CTCP;

       while(pIrcEventStruc->Text[Index] != 1 && pIrcEventStruc->Text[Index] != ' ')
            Index++;

/*       if(!strncmp(pIrcEventStruc->Text + 1, "DCC", Index - 1))
       {
       }
       else*/

   }
}





/*******************************************************************************
 * IRC_IsNumericId                                                             *
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
BOOL IRC_IsNumericId(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
    BOOL bRetVal = FALSE;

    if(EndOfMessageIndex == 2)
       if(isdigit(*pBuffer) && isdigit(*(pBuffer + 1)) && isdigit(*(pBuffer + 2)))
       {
           bRetVal = TRUE;
           pIrcEventStruc->NumericId = (((*pBuffer) - '0')*100) + (((*pBuffer + 1) - '0')*10) + ((*pBuffer + 2) - '0');
       }

    return bRetVal;
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
void IRC_ExtractNumericMessage(PCHAR pBuffer, UINT EndOfMessageIndex, PIRC_EVENT_STRUC pIrcEventStruc)
{
   UINT Index = 0, IndexHolder;
   
   while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
        Index++;


   if(Index < EndOfMessageIndex)
   {

      if(*pBuffer == '#' || *pBuffer == '&')
        strncpy(pIrcEventStruc->ChannelCommand, pBuffer, Index);
      else
        strncpy(pIrcEventStruc->NickCommand, pBuffer, Index);

      Index++;

      if(Index < EndOfMessageIndex)
      {
         if(*(pBuffer + Index) == ':')
            strncpy(pIrcEventStruc->Text, pBuffer + Index + 1, EndOfMessageIndex - Index - 1);
         else
         {
             IndexHolder = Index;

             while(pBuffer[Index] != ' ' && Index < EndOfMessageIndex)
                 Index++;

             if(*(pBuffer + IndexHolder) == '#' || *(pBuffer + IndexHolder) == '&')
                strncpy(pIrcEventStruc->ChannelCommand, pBuffer + IndexHolder, Index - IndexHolder);
             else
                strncpy(pIrcEventStruc->NickCommand, pBuffer + IndexHolder, Index - IndexHolder);

             Index++;

             if(Index < EndOfMessageIndex)
             {  
                if(*(pBuffer + Index) == ':')
                  strncpy(pIrcEventStruc->Text, pBuffer + Index + 1, EndOfMessageIndex - Index - 1);
                else
                  strncpy(pIrcEventStruc->Text, pBuffer + Index, EndOfMessageIndex - Index);
             }

         }
      }
   }
   else if(*pBuffer == ':')
           strncpy(pIrcEventStruc->Text, pBuffer + 1, Index - 1);
        else
           strncpy(pIrcEventStruc->Text, pBuffer, Index);
}



 