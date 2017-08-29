/***********************************************************************
 * json.h
 *  
 *   
 *
 *
 * Toby Opferman Copyright (c) 2013
 *
 ***********************************************************************/
 
 
 #ifndef __JSON_H__
 #define __JSON_H__
 
 typedef PVOID HJSON;
 typedef PVOID HJSONENUM;
 typedef PVOID HJSONOBJECT;
 
 typedef BOOL (*PFN_JSON_CALLBACK)(HJSONENUM hJsonEnum, UINT ItemIndex, PVOID pContext);
  
 HJSON JSon_Open(UCHAR *pszJsonFile);
 void JSon_Debug_DisplayTree(HJSON hJson);
 void JSon_Close(HJSON hJson);
 BOOL JSon_QueryItemAsString(HJSON hJson, char *pszJsonPath, char *pszItemDataOut, ULONG Length);

 BOOL Json_QueryItemAsBool(HJSON hJson, char *pszJsonPath, BOOL *pBoolOut); 
 BOOL Json_QueryItemAsInteger(HJSON hJson, char *pszJsonPath, ULONG64 *pInt64Out); 
 BOOL Json_QueryItemAsString(HJSON hJson, char *pszJsonPath, char *pszStringOut, ULONG Size); 
 
 
 #endif
 
 
 