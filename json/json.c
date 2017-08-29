/***********************************************************************
 * json.C
 *  
 *    Quick and Dirty JSON Parser for game engine
 *
 *
 * Toby Opferman Copyright (c) 2013
 *
 ***********************************************************************/
 
 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <json.h>



typedef enum _JSON_STATE_ENUM
{ 
   JsonStateError,
   JsonStateEmpty,
   JsonStateStart,
   JsonStateItemNameStart,
   JsonStateDelimeter,
   JsonStateObjectData,
   JsonStateItemIntStart,
   JsonStateItemStringStart,
   JsonStateComma,
   JsonStateArray,
   JsonSubStateStart,
   JsonStateItemTrueStart_r, 
   JsonStateItemTrueStart_u,
   JsonStateItemTrueStart_e,
   JsonStateItemFalseStart_a,
   JsonStateItemFalseStart_l,
   JsonStateItemFalseStart_s,
   JsonStateItemFalseStart_e,
   
   JsonStateMax
   
} JSON_STATE_ENUM, *PJSON_STATE_ENUM;

char *g_pszStringNames[] =
{
   "JsonStateError",
   "JsonStateEmpty",
   "JsonStateStart",
   "JsonStateItemNameStart",
   "JsonStateDelimeter",
   "JsonStateObjectData",
   "JsonStateItemIntStart",
   "JsonStateItemStringStart",
   "JsonStateComma",
   "JsonStateArray",
   "JsonSubStateStart",
   "JsonStateItemTrueStart_r", 
   "JsonStateItemTrueStart_u",
   "JsonStateItemTrueStart_e",
   "JsonStateItemFalseStart_a",
   "JsonStateItemFalseStart_l",
   "JsonStateItemFalseStart_s",
   "JsonStateItemFalseStart_e",
   
   "JsonStateMax"
};

typedef struct _STATE_TRANSITION
{
    char *TokenArray;
	PJSON_STATE_ENUM JsonState;
	char *IgnoreArray;
	
} STATE_TRANSITION, *PSTATE_TRANSITION;

typedef struct _JSON_STATE_MACHINE
{
     JSON_STATE_ENUM CurrentState;
	 UCHAR *pBufferStart;
	 PSTATE_TRANSITION pStateTransition;
	
} JSON_STATE_MACHINE, *PJSON_STATE_MACHINE;

JSON_STATE_ENUM g_JsonEmptyState[] = { JsonStateStart, JsonStateError };
JSON_STATE_ENUM g_JsonStartState[] = { JsonStateItemNameStart, JsonStateEmpty };
JSON_STATE_ENUM g_JsonItemNameStartState[] = { JsonStateDelimeter, JsonStateError };
JSON_STATE_ENUM g_JsonDelimterState[]      = { JsonStateObjectData, JsonStateError };
JSON_STATE_ENUM g_JsonObjectDataState[]    = { JsonSubStateStart, JsonStateArray, JsonStateItemStringStart, JsonStateItemIntStart, JsonStateItemIntStart, JsonStateItemIntStart, JsonStateItemIntStart, JsonStateItemIntStart, JsonStateItemIntStart, JsonStateItemIntStart, JsonStateItemIntStart, JsonStateItemIntStart, JsonStateItemIntStart, JsonStateItemTrueStart_r, JsonStateItemFalseStart_a  };
JSON_STATE_ENUM g_JsonItemStringStartState[] = { JsonStateComma,  JsonStateError };
JSON_STATE_ENUM g_JsonItemIntStartState[]    = {  JsonStateComma, JsonStateStart,  JsonStateComma, JsonStateComma, JsonStateEmpty };
JSON_STATE_ENUM g_JsonCommaState[]         = { JsonStateStart, JsonStateEmpty };
JSON_STATE_ENUM g_JsonSubStartState[]         = { JsonStateStart, JsonStateEmpty };
JSON_STATE_ENUM g_JsonArrayState[]         = { JsonStateStart, JsonStateEmpty };


JSON_STATE_ENUM g_JsonTrue1State[]         = { JsonStateItemTrueStart_u, JsonStateEmpty };
JSON_STATE_ENUM g_JsonTrue2State[]         = { JsonStateItemTrueStart_e, JsonStateEmpty };
JSON_STATE_ENUM g_JsonTrue3State[]         = { JsonStateComma, JsonStateEmpty };
JSON_STATE_ENUM g_JsonFalse1State[]         = { JsonStateItemFalseStart_l, JsonStateEmpty };
JSON_STATE_ENUM g_JsonFalse2State[]         = { JsonStateItemFalseStart_s, JsonStateEmpty };
JSON_STATE_ENUM g_JsonFalse3State[]         = { JsonStateItemFalseStart_e, JsonStateEmpty };
JSON_STATE_ENUM g_JsonFalse4State[]         = { JsonStateComma, JsonStateEmpty };

#define DEFAULT_WHITE_SPACE " \r\n"
#define LOWER_CASE_ALPHA    "abcdefghijklmnopqrstuvwxyz"
#define UPPER_CASE_ALPHA    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ALL_NUMBERS         "0123456789"
#define SPECIAL_CHARACTERS  "-_+=~`!@#$%^&*()\'\\/?.,<>;:"
#define SPACE_CHARACTER     " "
#define ITEM_NAME           LOWER_CASE_ALPHA UPPER_CASE_ALPHA ALL_NUMBERS SPECIAL_CHARACTERS SPACE_CHARACTER




STATE_TRANSITION g_pStateTransition[] =
{
   /* Error State: Nothing to do */
     { "",    NULL, "" },                           
	 
   /* Empty State: Where the state machine starts */
	 { "{",   &g_JsonEmptyState[0], DEFAULT_WHITE_SPACE },           
	 
   /* Start State: Looking for either the end or an item name */
	{ "\"}", &g_JsonStartState[0], DEFAULT_WHITE_SPACE},        
	 
   /* Item Name Start State: Consuming the item name */
 	{ "\"",  &g_JsonItemNameStartState[0], ITEM_NAME }, 
	 
    /* Item Delimeter State: Finding the content of the item */	 
  	{ ":",  &g_JsonDelimterState[0], DEFAULT_WHITE_SPACE },           
	 
    /* Object data state: Parsing the item data */	 	 
  	{ "{[\"0123456789tf",  &g_JsonObjectDataState[0], DEFAULT_WHITE_SPACE },
	 
    /* Item Integer State: Parsing Item Integer Data */
	{ " ,\r\n}",  &g_JsonItemIntStartState[0], ALL_NUMBERS },    
	
    /* Item String State: Parsing Item Integer Data */
	{ "\"",  &g_JsonItemStringStartState[0], ITEM_NAME },   
	
    /* Comma State: Parsing Until finding the comma or exiting */
	{ ",}",  &g_JsonCommaState[0], DEFAULT_WHITE_SPACE },
		
    /* Array: Parsing the end of the array */
	{ ",}",  &g_JsonArrayState[0], DEFAULT_WHITE_SPACE },
	
    /* Substate: Parsing Until finding the } */
	{ ",}",  &g_JsonSubStartState[0], DEFAULT_WHITE_SPACE },
	
    { "r",  &g_JsonTrue1State[0], DEFAULT_WHITE_SPACE },	
	{ "u",  &g_JsonTrue2State[0], DEFAULT_WHITE_SPACE },
	{ "e",  &g_JsonTrue3State[0], DEFAULT_WHITE_SPACE },
	{ "a",  &g_JsonFalse1State[0], DEFAULT_WHITE_SPACE },
	{ "l",  &g_JsonFalse2State[0], DEFAULT_WHITE_SPACE },
	{ "s",  &g_JsonFalse3State[0], DEFAULT_WHITE_SPACE },
	{ "e",  &g_JsonFalse4State[0], DEFAULT_WHITE_SPACE }
	
};

   
JSON_STATE_ENUM g_JsonStartStateArray[] = { JsonSubStateStart, JsonStateItemStringStart, JsonStateItemIntStart,JsonStateItemIntStart,JsonStateItemIntStart,JsonStateItemIntStart,JsonStateItemIntStart,JsonStateItemIntStart,JsonStateItemIntStart,JsonStateItemIntStart,JsonStateItemIntStart,JsonStateItemIntStart,JsonStateItemIntStart,JsonStateItemTrueStart_r, JsonStateItemFalseStart_a, JsonStateEmpty };
JSON_STATE_ENUM g_JsonCommaStateArray[] = { JsonStateStart, JsonStateEmpty };
JSON_STATE_ENUM g_JsonSubStartStateArray[] = { JsonStateStart, JsonStateEmpty };
JSON_STATE_ENUM g_JsonItemIntStartStateArray[]    = {  JsonStateComma, JsonStateStart,  JsonStateEmpty, JsonStateComma, JsonStateComma };
JSON_STATE_ENUM g_JsonItemStringStartStateArray[] = { JsonStateComma,  JsonStateError };

STATE_TRANSITION g_pStateTransitionForArray[] =
{
   /* Error State: Nothing to do */
     { "",    NULL, "" },                           
	 
   /* Empty State: Where the state machine starts */
	 { "",   NULL, "" },           
	 
   /* Start State: Looking for either the end or an item name */
	 { "{\"0123456789tf]",   &g_JsonStartStateArray[0], DEFAULT_WHITE_SPACE },         
	 
   /* Item Name Start State: Consuming the item name */
 	{ "", NULL, ""},        
	 
    /* Item Delimeter State: Finding the content of the item */	 
  	{ "", NULL, ""},        
	 
    /* Object data state: Parsing the item data */	 	 
  	{ "", NULL, ""}, 
	 
    /* Item Integer State: Parsing Item Integer Data */
	{ " ,]\r\n",  &g_JsonItemIntStartStateArray[0], ALL_NUMBERS },    
	
    /* Item String State: Parsing Item Integer Data */
	{ "\"",  &g_JsonItemStringStartStateArray[0], ITEM_NAME },   
	
    /* Comma State: Parsing Until finding the comma or exiting */
	{ ",]",  &g_JsonCommaStateArray[0], DEFAULT_WHITE_SPACE },
		
    /* Array: Parsing the end of the array */
	{ "]",  &g_JsonArrayState[0], DEFAULT_WHITE_SPACE },
	
    /* Substate: Parsing Until finding the } */
	{ ",]",  &g_JsonSubStartStateArray[0], DEFAULT_WHITE_SPACE }	
};

typedef enum _ARRAY_TYPE
{
   ArrayNone,
   ArrayInteger,
   ArrayString,
   ArrayBoolean,
   ArrayObject
   
} ARRAY_TYPE, *PARRAY_TYPE;

#define FLAGS_ELEMENT_INTEGER  0x8
#define FLAGS_ELEMENT_STRING  0x10
#define FLAGS_ELEMENT_BOOLEAN 0x20

typedef struct _JSON_ARRAY_ELEMENT
{
    DWORD Flags;
	
	char szItemString[256];
	ULONG64 ItemInteger64;
	BOOL bItemBoolean;
	
} JSON_ARRAY_ELEMENT, *PJSON_ARRAY_ELEMENT;



typedef struct _JSON_ARRAY
{
    
	JSON_ARRAY_ELEMENT ArrayElement;
    struct _JSON_ITEM  *pObject;
	struct _JSON_ARRAY *pNext;
	
} JSON_ARRAY, *PJSON_ARRAY;

/*
 * Item Flags
 */
#define FLAGS_ITEM_OBJECT   0x2
#define FLAGS_ITEM_ARRAY    0x4
#define FLAGS_ITEM_INTEGER  0x8
#define FLAGS_ITEM_STRING  0x10
#define FLAGS_ITEM_BOOLEAN 0x20

typedef struct _JSON_ITEM
{
    DWORD Flags;
	
	char szItemName[256];
	
	char szItemString[256];
	ULONG64 ItemInteger64;
	BOOL bItemBoolean;
	
	UCHAR *szStartOfItem;
		
	struct _JSON_ITEM *pSubEntries;
    struct _JSON_ARRAY *pJsonArray;
    struct _JSON_ITEM *pJsonNext;	
	
} JSON_ITEM, *PJSON_ITEM;

/*
 * No Windowing support, Maps the full file.
 */

typedef struct _JSON_INTERNAL
{
    HANDLE hFile;
	HANDLE hMemoryMapping;
	UCHAR *pStartFileView;
	UCHAR *pEndFileView;
	PJSON_ITEM pJsonItem;
	
} JSON_INTERNAL, *PJSON_INTERNAL;

typedef struct _JSON_INTERNAL_ENUM
{
     PJSON_INTERNAL pJsonInternal;
	 JSON_ITEM JsonItem;	 
	
} JSON_INTERNAL_ENUM, *PJSON_INTERNAL_ENUM;


typedef struct _JSON_QUERY
{
     char *pszPath;
     BOOL bFoundData;
	 JSON_ARRAY_ELEMENT  ArrayElement;
	
} JSON_QUERY, *PJSON_QUERY;



/*
 * Internal Prototypes
 */
JSON_STATE_ENUM JsonState_FindNextStateToken(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal);
BOOL JsonState_AdvanceStateMachinePointer(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal);
BOOL JsonState_AdvanceIgnoreSpace(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal);
PJSON_ITEM JsonState_BuildItem(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal);
BOOL Json_BuildTree(PJSON_INTERNAL pJsonInternal);
BOOL JsonState_DecodeItemName(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal, PJSON_ITEM pJsonItem);
BOOL JsonState_DecodeItemString(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal, PJSON_ITEM pJsonItem);
BOOL JsonState_DecodeItemInt64(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal, PJSON_ITEM pJsonItem);
PJSON_ITEM JsonState_BuildSubItems(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal);
PJSON_ARRAY JsonState_BuildArrayItems(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal);
void Json_TraverseItem(char *pszItemPath, PJSON_ITEM pJsonItem);
void Json_TraverseArray(char *pszItemPath, PJSON_ARRAY pJsonArray);
void Json_QueryTraverseItem(PJSON_QUERY pQueryItem, char *pszItemPath, PJSON_ITEM pJsonItem);
void Json_QueryTraverseArray(PJSON_QUERY pQueryItem, char *pszItemPath, PJSON_ARRAY pJsonArray);
void Json_FreeItem(PJSON_ITEM pJsonItem);
void Json_FreeArray(PJSON_ARRAY pJsonArray);
  
 /***********************************************************************
  * JSon_Open
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
HJSON JSon_Open(UCHAR *pszJsonFile)
{
    PJSON_INTERNAL pJsonInternal = NULL;
	DWORD dwFileSizeLow;
	DWORD dwFileSizeHigh;
	BOOL bCreated;
	
	bCreated = FALSE;
	pJsonInternal = (PJSON_INTERNAL)LocalAlloc(LMEM_ZEROINIT, sizeof(JSON_INTERNAL));
	
	if(pJsonInternal)
	{
	     pJsonInternal->hFile = CreateFile(pszJsonFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		 
		 if(pJsonInternal->hFile != NULL && pJsonInternal->hFile != INVALID_HANDLE_VALUE)
		 {
				dwFileSizeLow = GetFileSize(pJsonInternal->hFile, &dwFileSizeHigh);
				
				if(dwFileSizeHigh == 0 && dwFileSizeLow != 0)
				{
					pJsonInternal->hMemoryMapping =  CreateFileMapping(pJsonInternal->hFile, NULL, PAGE_READONLY, dwFileSizeHigh, dwFileSizeLow, NULL);
					
					if(pJsonInternal->hMemoryMapping != NULL && pJsonInternal->hMemoryMapping != INVALID_HANDLE_VALUE)
					{
						pJsonInternal->pStartFileView = MapViewOfFile(pJsonInternal->hMemoryMapping, FILE_MAP_READ, 0, 0, 0);
						
						if(pJsonInternal->pStartFileView)
						{
							pJsonInternal->pEndFileView = pJsonInternal->pStartFileView  + dwFileSizeLow;
							bCreated = Json_BuildTree(pJsonInternal);
						}
					}
				}
		}
		
		if(bCreated == FALSE)
		{
            JSon_Close((HJSON)pJsonInternal);
			pJsonInternal = NULL;
		}
	}
	
	return (HJSON)pJsonInternal;
}

 /***********************************************************************
  * JSon_Debug_DisplayTree
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
 void JSon_Debug_DisplayTree(HJSON hJson)
 {
    PJSON_INTERNAL pJsonInternal = (PJSON_INTERNAL)hJson;
	
	if(pJsonInternal->pJsonItem)
	{
	    Json_TraverseItem("root", pJsonInternal->pJsonItem);
	}	
 }
 
 
 /***********************************************************************
  * JSon_Close
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/  
void Json_FreeItem(PJSON_ITEM pJsonItem)
{
    PJSON_ITEM pJsonFreeItem;
 
     while(pJsonItem)
	 {
		 
		 if(pJsonItem->pJsonArray)
		 {
            Json_FreeArray(pJsonItem->pJsonArray);	
		 }
		 
		 if(pJsonItem->pSubEntries)
		 {
			Json_FreeItem(pJsonItem->pSubEntries);
		 }
		 
		 pJsonFreeItem = pJsonItem;		 
	     pJsonItem = pJsonItem->pJsonNext;
		 LocalFree(pJsonFreeItem);
	 }
 }

 
  /***********************************************************************
  * JSon_Close
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/ 
void Json_FreeArray(PJSON_ARRAY pJsonArray)
 {
     PJSON_ARRAY pJsonFreeArray;
	 
     while(pJsonArray)
	 {	 
		 if(pJsonArray->pObject)
		 {
			Json_FreeItem(pJsonArray->pObject);
		 }
		 
		 pJsonFreeArray = pJsonArray;
		 
	     pJsonArray = pJsonArray->pNext;
		 LocalFree(pJsonFreeArray);
	 }
 }
 
 /***********************************************************************
  * JSon_Close
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/ 
 void Json_TraverseItem(char *pszItemPath, PJSON_ITEM pJsonItem)
 {
     char szItemPath[1024] = {0};
	 
     while(pJsonItem)
	 {
	     sprintf(szItemPath, "%s.%s", pszItemPath, pJsonItem->szItemName);
	     printf("ItemName: %s.%s\n", pszItemPath, pJsonItem->szItemName);
		 
		 if(pJsonItem->Flags & FLAGS_ITEM_INTEGER)
		 {
		     printf("   Type: Integer, %i\n", pJsonItem->ItemInteger64);
		 }
		 
		 if(pJsonItem->Flags & FLAGS_ITEM_STRING)
		 {
		     printf("   Type: String, '%s'\n", pJsonItem->szItemString);
		 }
		 
		 if(pJsonItem->Flags & FLAGS_ITEM_BOOLEAN)
		 {
		     printf("   Type: Boolean, %s\n", pJsonItem->bItemBoolean ? "true" : "false");
		 }
		 
		 if(pJsonItem->Flags & FLAGS_ITEM_ARRAY)
		 {
		    printf("Begin Array\n");		
            Json_TraverseArray(szItemPath, pJsonItem->pJsonArray);	
			printf("End Array\n");		 
		 }
		 
		 if(pJsonItem->Flags & FLAGS_ITEM_OBJECT)
		 {
		    printf("Begin Object\n");
			Json_TraverseItem(szItemPath, pJsonItem->pSubEntries);
			printf("End Object\n");
		 }
		 
	     pJsonItem = pJsonItem->pJsonNext;
	 }
 }
 
  /***********************************************************************
  * JSon_Close
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/ 
 void Json_TraverseArray(char *pszItemPath, PJSON_ARRAY pJsonArray)
 {
     char szItemPath[1024] = {0};
	 UINT Index = 0;
	 
     while(pJsonArray)
	 {	 
	 
  	     sprintf(szItemPath, "%s[%i]", pszItemPath, Index);
		 
		 printf("      ItemName: %s\n", szItemPath);
		 
		 if(pJsonArray->ArrayElement.Flags & FLAGS_ELEMENT_INTEGER)
		 {
		     printf("   Type: Integer, %i\n", pJsonArray->ArrayElement.ItemInteger64);
		 }
		 
		 if(pJsonArray->ArrayElement.Flags & FLAGS_ELEMENT_STRING)
		 {
		     printf("   Type: String, '%s'\n", pJsonArray->ArrayElement.szItemString);
		 }
		 
		 if(pJsonArray->ArrayElement.Flags & FLAGS_ELEMENT_BOOLEAN)
		 {
		     printf("   Type: Boolean, %s\n", pJsonArray->ArrayElement.bItemBoolean ? "true" : "false");
		 }
		 
		 if(pJsonArray->pObject)
		 {
		    printf("Begin Object\n");		 
			Json_TraverseItem(szItemPath, pJsonArray->pObject);
			printf("End Object\n");		 
		 }
		 	 
	     pJsonArray = pJsonArray->pNext;
		 
		 Index++;
	 }
 }
 
 
 /***********************************************************************
  * JSon_Close
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/ 
 void Json_QueryTraverseItem(PJSON_QUERY pQueryItem, char *pszItemPath, PJSON_ITEM pJsonItem)
 {
     char szItemPath[1024] = {0};
	 
     while(pJsonItem && pQueryItem->bFoundData == FALSE)
	 {
	     sprintf(szItemPath, "%s.%s", pszItemPath, pJsonItem->szItemName);

		 if(_stricmp(pQueryItem->pszPath, szItemPath) == 0)
		 {
			 if(pJsonItem->Flags & FLAGS_ITEM_INTEGER)
			 {
			     pQueryItem->ArrayElement.Flags |= FLAGS_ELEMENT_INTEGER;
				 pQueryItem->ArrayElement.ItemInteger64 = pJsonItem->ItemInteger64;
				 pQueryItem->bFoundData = TRUE;				 
			 }
			 
			 if(pJsonItem->Flags & FLAGS_ITEM_STRING)
			 {
			    pQueryItem->ArrayElement.Flags |= FLAGS_ELEMENT_STRING;
				strcpy(pQueryItem->ArrayElement.szItemString, pJsonItem->szItemString);
				pQueryItem->bFoundData = TRUE;
				
			 }
			 
			 if(pJsonItem->Flags & FLAGS_ITEM_BOOLEAN)
			 {
			    pQueryItem->ArrayElement.Flags |= FLAGS_ELEMENT_BOOLEAN;
				pQueryItem->ArrayElement.bItemBoolean = pJsonItem->bItemBoolean;
				pQueryItem->bFoundData = TRUE;
			 }
		}

		 if(pJsonItem->Flags & FLAGS_ITEM_ARRAY)
		 {
			Json_QueryTraverseArray(pQueryItem, szItemPath, pJsonItem->pJsonArray);	
		 }
		
		 
		 if(pJsonItem->Flags & FLAGS_ITEM_OBJECT)
		 {
			Json_QueryTraverseItem(pQueryItem, szItemPath, pJsonItem->pSubEntries);
		 }
		 
	     pJsonItem = pJsonItem->pJsonNext;
	 }
 }
 
  /***********************************************************************
  * JSon_Close
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/ 
 void Json_QueryTraverseArray(PJSON_QUERY pQueryItem, char *pszItemPath, PJSON_ARRAY pJsonArray)
 {
     char szItemPath[1024] = {0};
	 UINT Index = 0;
	 
     while(pJsonArray && pQueryItem->bFoundData == FALSE)
	 { 
  	     sprintf(szItemPath, "%s[%i]", pszItemPath, Index);
		 
	     if(_stricmp(pQueryItem->pszPath, szItemPath) == 0)
		 {
			 if(pJsonArray->ArrayElement.Flags & FLAGS_ELEMENT_INTEGER)
			 {
			     pQueryItem->ArrayElement.Flags |= FLAGS_ELEMENT_INTEGER;
				 pQueryItem->ArrayElement.ItemInteger64 = pJsonArray->ArrayElement.ItemInteger64;
				 pQueryItem->bFoundData = TRUE;				 
			 }
			 
			 if(pJsonArray->ArrayElement.Flags & FLAGS_ELEMENT_STRING)
			 {
			    pQueryItem->ArrayElement.Flags |= FLAGS_ELEMENT_STRING;
				strcpy(pQueryItem->ArrayElement.szItemString, pJsonArray->ArrayElement.szItemString);
				pQueryItem->bFoundData = TRUE;
				
			 }
			 
			 if(pJsonArray->ArrayElement.Flags & FLAGS_ELEMENT_BOOLEAN)
			 {
			    pQueryItem->ArrayElement.Flags |= FLAGS_ELEMENT_BOOLEAN;
				pQueryItem->ArrayElement.bItemBoolean = pJsonArray->ArrayElement.bItemBoolean;
				pQueryItem->bFoundData = TRUE;
			 }
		 }
		 
		 if(pJsonArray->pObject)
		 {
			Json_QueryTraverseItem(pQueryItem, szItemPath, pJsonArray->pObject);
		 }
		 
	     pJsonArray = pJsonArray->pNext;
		 Index++;
	 }
 }
 
  /***********************************************************************
  * Json_QueryItemAsBool
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
 BOOL Json_QueryItemAsBool(HJSON hJson, char *pszJsonPath, BOOL *pBoolOut)
 {
     JSON_QUERY JsonQuery = {0};
	 PJSON_INTERNAL pJsonInternal = (PJSON_INTERNAL)hJson;
	 BOOL bFoundData = FALSE;
	 
	 JsonQuery.pszPath = pszJsonPath;
	 
	 Json_QueryTraverseItem(&JsonQuery, "root", pJsonInternal->pJsonItem);
	 
	 if(JsonQuery.bFoundData)
	 {
	     if(JsonQuery.ArrayElement.Flags & FLAGS_ELEMENT_BOOLEAN)
		 {
		     *pBoolOut = JsonQuery.ArrayElement.bItemBoolean;
			 bFoundData = TRUE;
		 }
	 }
	 
	 return bFoundData;
 }
 
 
  /***********************************************************************
  * Json_QueryItemAsInteger
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/  
 BOOL Json_QueryItemAsInteger(HJSON hJson, char *pszJsonPath, ULONG64 *pInt64Out)
 {
     JSON_QUERY JsonQuery = {0};
	 PJSON_INTERNAL pJsonInternal = (PJSON_INTERNAL)hJson;
	 BOOL bFoundData = FALSE;
	 
	 JsonQuery.pszPath = pszJsonPath;
	 
	 Json_QueryTraverseItem(&JsonQuery, "root", pJsonInternal->pJsonItem);
	 
	 if(JsonQuery.bFoundData)
	 {
	     if(JsonQuery.ArrayElement.Flags & FLAGS_ELEMENT_INTEGER)
		 {
		     *pInt64Out = JsonQuery.ArrayElement.ItemInteger64;
			 bFoundData = TRUE;
		 }
	 }
	 
	 return bFoundData;
 }
 
  /***********************************************************************
  * Json_QueryItemAsString
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/   
 BOOL Json_QueryItemAsString(HJSON hJson, char *pszJsonPath, char *pszStringOut, ULONG Size)
 {
     JSON_QUERY JsonQuery = {0};
	 PJSON_INTERNAL pJsonInternal = (PJSON_INTERNAL)hJson;
	 BOOL bFoundData = FALSE;
	 
	 JsonQuery.pszPath = pszJsonPath;
	 
	 Json_QueryTraverseItem(&JsonQuery, "root", pJsonInternal->pJsonItem);
	 
	 if(JsonQuery.bFoundData)
	 {
	     if(JsonQuery.ArrayElement.Flags & FLAGS_ELEMENT_STRING)
		 {
		     strcpy(pszStringOut, JsonQuery.ArrayElement.szItemString);
			 bFoundData = TRUE;
		 }
	 }
	 
	 return bFoundData;
 }
 





 /***********************************************************************
  * JSon_Close
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
void JSon_Close(HJSON hJson)
{
   PJSON_INTERNAL pJsonInternal = (PJSON_INTERNAL)hJson;
   
   if(pJsonInternal->pJsonItem)
   {
       Json_FreeItem(pJsonInternal->pJsonItem);
   }
	  
   if(pJsonInternal->pStartFileView)
   {
       UnmapViewOfFile(pJsonInternal->pStartFileView);
	   pJsonInternal->pStartFileView = NULL;
   }
   
   if(pJsonInternal->hMemoryMapping != NULL && pJsonInternal->hMemoryMapping != INVALID_HANDLE_VALUE)
   {
      CloseHandle(pJsonInternal->hMemoryMapping);
   }

   if(pJsonInternal->hFile != NULL && pJsonInternal->hFile != INVALID_HANDLE_VALUE)
   {
      CloseHandle(pJsonInternal->hFile);
   }
 
   LocalFree(pJsonInternal);
}

 /***********************************************************************
  * Json_BuildTree
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
BOOL Json_BuildTree(PJSON_INTERNAL pJsonInternal)
{
   PJSON_ITEM pJsonItemCurrent;
   BOOL bTreeBuild;
   BOOL bError;
   JSON_STATE_MACHINE JsonStateMachine = {0};
   PJSON_ITEM pJsonItem;
   
   bTreeBuild = FALSE;
   bError = FALSE;
   
   JsonStateMachine.CurrentState    = JsonStateEmpty;
   JsonStateMachine.pBufferStart    = pJsonInternal->pStartFileView;
   JsonStateMachine.pStateTransition = &g_pStateTransition[0];
   
   JsonStateMachine.CurrentState = JsonState_FindNextStateToken(&JsonStateMachine, pJsonInternal);
   
   while(bTreeBuild == FALSE && bError == FALSE)
   {
	   switch(JsonStateMachine.CurrentState)
	   {
		   case JsonStateError:
				bError    = TRUE;
				break;
				
		   case JsonStateEmpty:
				bTreeBuild = TRUE;
				break;
			
		   case JsonStateItemNameStart:
				pJsonItem = JsonState_BuildItem(&JsonStateMachine, pJsonInternal);
				
				if(pJsonItem)
				{
				   if(pJsonInternal->pJsonItem)
				   {
					   pJsonItemCurrent->pJsonNext = pJsonItem; 
				   }
				   else
				   { 
					  pJsonInternal->pJsonItem = pJsonItem;
				   }					   
				   
				   pJsonItemCurrent = pJsonItem;
				}
				else
				{
				   bError = TRUE;
				}				
				break;
            default:
                JsonStateMachine.CurrentState = JsonState_FindNextStateToken(&JsonStateMachine, pJsonInternal);			   
				
	   }
   }
   
   if(pJsonInternal->pJsonItem == NULL)
   {
      bTreeBuild = FALSE;
   }
   
   return bTreeBuild;
}



 /***********************************************************************
  * JsonState_BuildSubItems
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
PJSON_ITEM JsonState_BuildSubItems(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal)
{
   PJSON_ITEM  pJsonItem = NULL;
   PJSON_ITEM pJsonNextItem;
   PJSON_ITEM pJsonItemCurrent;
   BOOL bTreeBuild;
   BOOL bError;
   JSON_STATE_MACHINE JsonStateMachine = {0};
   
   bTreeBuild = FALSE;
   bError = FALSE;
   
   JsonStateMachine.CurrentState    = JsonStateStart;
   JsonStateMachine.pBufferStart    = pJsonStateMachine->pBufferStart;
   JsonStateMachine.pStateTransition = &g_pStateTransition[0];
   JsonStateMachine.CurrentState = JsonState_FindNextStateToken(&JsonStateMachine, pJsonInternal);
   
   while(bTreeBuild == FALSE && bError == FALSE)
   {
	   switch(JsonStateMachine.CurrentState)
	   {
		   case JsonStateError:
				bError    = TRUE;
				break;
				
		   case JsonStateEmpty:
		        pJsonStateMachine->pBufferStart = JsonStateMachine.pBufferStart;
				bTreeBuild = TRUE;
				break;
			
		   case JsonStateItemNameStart:
				pJsonItemCurrent = JsonState_BuildItem(&JsonStateMachine, pJsonInternal);
				
				if(pJsonItemCurrent)
				{
				   if(pJsonItem)
				   {
					  pJsonNextItem->pJsonNext = pJsonItemCurrent; 
				   }
				   else
				   { 
					  pJsonItem = pJsonItemCurrent;
				   }					   
				   
				   pJsonNextItem = pJsonItemCurrent;
				}
				else
				{
				   bError = TRUE;
				}				
				break;
				
            default:
                JsonStateMachine.CurrentState = JsonState_FindNextStateToken(&JsonStateMachine, pJsonInternal);			   
				
	   }
   }	
   
   if(bError)
   {
      Json_FreeItem(pJsonItem);
	  pJsonItem = NULL;
   }
	
   return pJsonItem;
}

/***********************************************************************
  * JsonState_BuildArrayItems
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
PJSON_ARRAY JsonState_BuildArrayItems(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal)
{
   PJSON_ARRAY pJsonArray = NULL;
   PJSON_ARRAY pJsonNewArray = NULL;
   PJSON_ARRAY pJsonArrayCurrent = NULL;
   PJSON_ITEM pJsonItem;
   BOOL bTreeBuild;
   BOOL bError;
   JSON_STATE_MACHINE JsonStateMachine = {0};
   ARRAY_TYPE ArrayType = ArrayNone;
   JSON_ITEM JsonItem;
   
   bTreeBuild = FALSE;
   bError = FALSE;
   
   JsonStateMachine.CurrentState    = JsonStateStart;
   JsonStateMachine.pBufferStart    = pJsonStateMachine->pBufferStart;
   JsonStateMachine.pStateTransition = &g_pStateTransitionForArray[0];
   
   JsonStateMachine.CurrentState = JsonState_FindNextStateToken(&JsonStateMachine, pJsonInternal);
   
   while(bTreeBuild == FALSE && bError == FALSE)
   {
	   

	   switch(JsonStateMachine.CurrentState)
	   {
		   case JsonStateError:
				bError    = TRUE;
				break;
				
		   case JsonStateEmpty:
		        pJsonStateMachine->pBufferStart = JsonStateMachine.pBufferStart;
				bTreeBuild = TRUE;
				break;
				
		   case JsonSubStateStart:
		        if(ArrayType == ArrayNone || ArrayType == ArrayObject)
				{
				    ArrayType = ArrayObject;
					pJsonItem = JsonState_BuildSubItems(&JsonStateMachine, pJsonInternal);
					
					if(pJsonItem)
					{
					   pJsonNewArray = (PJSON_ARRAY)LocalAlloc(LMEM_ZEROINIT, sizeof(JSON_ARRAY));
					   
					   if(pJsonNewArray)
					   {
						  pJsonNewArray->pObject = pJsonItem; 
						  
						  if(pJsonArrayCurrent)
						  {
							  pJsonArrayCurrent->pNext = pJsonNewArray;
						  }
						  else
						  {
							  pJsonArray = pJsonNewArray;
						  }
						  
						  pJsonArrayCurrent = pJsonNewArray;
						  JsonStateMachine.CurrentState = JsonState_FindNextStateToken(&JsonStateMachine, pJsonInternal);
					   }
					   else
					   { 
						  bError = TRUE;
					   }					   
					}
					else
					{
					   bError = TRUE;
					}
				}
				else
				{
				   bError = TRUE;
				}
		        break;
		 case JsonStateItemStringStart:
		 	  if(ArrayType == ArrayNone || ArrayType == ArrayString)
			  {
			      ArrayType = ArrayString;
				  memset(&JsonItem, 0, sizeof(JsonItem));
				  if(JsonState_DecodeItemString(&JsonStateMachine, pJsonInternal, &JsonItem))
				  {
                      pJsonNewArray = (PJSON_ARRAY)LocalAlloc(LMEM_ZEROINIT, sizeof(JSON_ARRAY));
					   
					  if(pJsonNewArray)
					  {
					    strcpy(pJsonNewArray->ArrayElement.szItemString, JsonItem.szItemString);
					    pJsonNewArray->ArrayElement.Flags |= FLAGS_ELEMENT_STRING;
						
						if(pJsonArrayCurrent)
						{
						   pJsonArrayCurrent->pNext = pJsonNewArray;
						 }
						 else
						 {
							  pJsonArray = pJsonNewArray;
						 }
						  
						 pJsonArrayCurrent = pJsonNewArray;
						  
					  }
					  else
					  { 
						  bError = TRUE;
					  }					  
				  }
				  else
				  { 
					  bError = TRUE;
				  }					  
			  }
   			  else
			  {
			     bError = TRUE;
			  }			  
			  break;
		 case JsonStateItemIntStart:
		 	  if(ArrayType == ArrayNone || ArrayType == ArrayInteger)
			  {
			      ArrayType = ArrayInteger;
				  memset(&JsonItem, 0, sizeof(JsonItem));
				  if(JsonState_DecodeItemInt64(&JsonStateMachine, pJsonInternal, &JsonItem))
				  {
                      pJsonNewArray = (PJSON_ARRAY)LocalAlloc(LMEM_ZEROINIT, sizeof(JSON_ARRAY));
					   
					  if(pJsonNewArray)
					  {
					    strcpy(pJsonNewArray->ArrayElement.szItemString, JsonItem.szItemString);
						pJsonNewArray->ArrayElement.ItemInteger64 = JsonItem.ItemInteger64;
						
					    pJsonNewArray->ArrayElement.Flags |= FLAGS_ELEMENT_INTEGER;
						
						if(pJsonArrayCurrent)
						{
						   pJsonArrayCurrent->pNext = pJsonNewArray;
						 }
						 else
						 {
							  pJsonArray = pJsonNewArray;
						 }
						  
						 pJsonArrayCurrent = pJsonNewArray;
						  
					  }
					  else
					  { 
						  bError = TRUE;
					  }					  
				  }
				  else
				  { 
					  bError = TRUE;
				  }					  
			  }
   			  else
			  {
			     bError = TRUE;
			  }			  
			  break;
			  
		 case  JsonStateItemTrueStart_r:
		 	  if(ArrayType == ArrayNone || ArrayType == ArrayBoolean)
			  {
			        do
					{
					    JsonStateMachine.CurrentState = JsonState_FindNextStateToken(&JsonStateMachine, pJsonInternal);
					} while(JsonStateMachine.CurrentState != JsonStateItemTrueStart_e && JsonStateMachine.CurrentState != JsonStateError);
					
					if(JsonStateMachine.CurrentState != JsonStateError)
					{
						ArrayType = ArrayBoolean;
						
						pJsonNewArray = (PJSON_ARRAY)LocalAlloc(LMEM_ZEROINIT, sizeof(JSON_ARRAY));
						   
						if(pJsonNewArray)
						{
							pJsonNewArray->ArrayElement.bItemBoolean = TRUE; 
							pJsonNewArray->ArrayElement.Flags |= FLAGS_ELEMENT_BOOLEAN;
							 
							if(pJsonArrayCurrent)
							{
							   pJsonArrayCurrent->pNext = pJsonNewArray;
							 }
							 else
							 {
								  pJsonArray = pJsonNewArray;
							 }
							  
							 pJsonArrayCurrent = pJsonNewArray;
							  
						  }
						  else
						  { 
							  bError = TRUE;
						  }					   
					}
					else
					{
					    bError = TRUE;
					}
			   }
			   else
			   {
				   bError = TRUE;
			   }			   
			   break;
			  
		 case  JsonStateItemFalseStart_a:
		 	  if(ArrayType == ArrayNone || ArrayType == ArrayBoolean)
			  {
			  
  			        do
					{
					    JsonStateMachine.CurrentState = JsonState_FindNextStateToken(&JsonStateMachine, pJsonInternal);
					} while(JsonStateMachine.CurrentState != JsonStateItemFalseStart_e && JsonStateMachine.CurrentState != JsonStateError);

			        ArrayType = ArrayBoolean;
					
					if(JsonStateMachine.CurrentState != JsonStateError)
					{
					
						pJsonNewArray = (PJSON_ARRAY)LocalAlloc(LMEM_ZEROINIT, sizeof(JSON_ARRAY));
						   
						if(pJsonNewArray)
						{
							pJsonNewArray->ArrayElement.bItemBoolean = FALSE; 
							pJsonNewArray->ArrayElement.Flags |= FLAGS_ELEMENT_BOOLEAN;
							 
							if(pJsonArrayCurrent)
							{
							   pJsonArrayCurrent->pNext = pJsonNewArray;
							 }
							 else
							 {
								  pJsonArray = pJsonNewArray;
							 }
							  
							 pJsonArrayCurrent = pJsonNewArray;
							  
						  }
						  else
						  { 
							  bError = TRUE;
						  }					   
					}
					else
					{
					    bError = TRUE;					
					}
			   }
			   else
			   {
				   bError = TRUE;
			   }	
			   break;				

               default:
                   JsonStateMachine.CurrentState = JsonState_FindNextStateToken(&JsonStateMachine, pJsonInternal);			   
				
	   }
   }	
   
   if(bError)
   {
      Json_FreeArray(pJsonArray);
	  pJsonArray = NULL;
   }
	
   return pJsonArray;
}




 /***********************************************************************
  * JsonState_BuildItem
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
PJSON_ITEM JsonState_BuildItem(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal)
{
   PJSON_ITEM pJsonItem = NULL;
   PJSON_ITEM pJsonNextItem = NULL;
   PJSON_ARRAY pJsonArrayItem = NULL;
   BOOL bItemBuilt = FALSE;
   
   pJsonItem = (PJSON_ITEM)LocalAlloc(LMEM_ZEROINIT, sizeof(JSON_ITEM));
   
   if(pJsonItem)
   {
        /*
		 * Decode the name of the Item
		 */
        pJsonItem->szStartOfItem = (pJsonStateMachine->pBufferStart-1);
		
		if(JsonState_DecodeItemName(pJsonStateMachine, pJsonInternal, pJsonItem))
		{
		        /*
     		     * Skip the Delimter
	         	 */
	            pJsonStateMachine->CurrentState = JsonState_FindNextStateToken(pJsonStateMachine, pJsonInternal);
   	            if(pJsonStateMachine->CurrentState != JsonStateError)
	            {
				
					/*
					 * Decode the Item's Data
					 */
					 
					 pJsonStateMachine->CurrentState = JsonState_FindNextStateToken(pJsonStateMachine, pJsonInternal);
					 if(pJsonStateMachine->CurrentState != JsonStateError)
					 {
					 
						 switch(pJsonStateMachine->CurrentState)
						 {
							 case JsonSubStateStart:
							      pJsonNextItem = JsonState_BuildSubItems(pJsonStateMachine, pJsonInternal);
								  if(pJsonNextItem)
								  {
								      bItemBuilt = TRUE;
									  pJsonItem->pSubEntries = pJsonNextItem;
									  pJsonItem->Flags |= FLAGS_ITEM_OBJECT;
								  }
								  break;
							 case JsonStateArray:
							      pJsonArrayItem = JsonState_BuildArrayItems(pJsonStateMachine, pJsonInternal);
								  if(pJsonArrayItem)
								  {
								      bItemBuilt = TRUE;
									  pJsonItem->pJsonArray = pJsonArrayItem;
									  pJsonItem->Flags |= FLAGS_ITEM_ARRAY;
								  }							 
								  break;
								  
							 case JsonStateItemStringStart:
							      if(JsonState_DecodeItemString(pJsonStateMachine, pJsonInternal, pJsonItem))
								  {
								      pJsonItem->Flags |= FLAGS_ITEM_STRING;
								      bItemBuilt = TRUE;
								  }
								  break;
							 case JsonStateItemIntStart:
							      if(JsonState_DecodeItemInt64(pJsonStateMachine, pJsonInternal, pJsonItem))
								  {
								      pJsonItem->Flags |= FLAGS_ITEM_INTEGER;
								      bItemBuilt = TRUE;
								  }
								  break;
								  
						     case  JsonStateItemTrueStart_r:
							      do
									{
										pJsonStateMachine->CurrentState = JsonState_FindNextStateToken(pJsonStateMachine, pJsonInternal);
									} while(pJsonStateMachine->CurrentState != JsonStateItemTrueStart_e && pJsonStateMachine->CurrentState != JsonStateError);
								   pJsonItem->bItemBoolean = TRUE;
								   pJsonItem->Flags |= FLAGS_ITEM_BOOLEAN;
								   bItemBuilt = TRUE;
								   break;
								  
                             case  JsonStateItemFalseStart_a:
							      do
									{
										pJsonStateMachine->CurrentState = JsonState_FindNextStateToken(pJsonStateMachine, pJsonInternal);
									} while(pJsonStateMachine->CurrentState != JsonStateItemFalseStart_e && pJsonStateMachine->CurrentState != JsonStateError);							 
							       pJsonItem->bItemBoolean = FALSE;
								   pJsonItem->Flags |= FLAGS_ITEM_BOOLEAN;
							       bItemBuilt = TRUE;
								   break;						
						}
					}
		  
	            }   

		}
    }
	
	if(bItemBuilt == FALSE)
	{
	    LocalFree(pJsonItem);
		pJsonItem = NULL;
	}
	
	
   return pJsonItem;
}

 /***********************************************************************
  * JsonState_DecodeItemName
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
BOOL JsonState_DecodeItemName(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal, PJSON_ITEM pJsonItem)
{
    BOOL bItemNameDecoded = FALSE;
	
	pJsonStateMachine->CurrentState = JsonState_FindNextStateToken(pJsonStateMachine, pJsonInternal);
    if(pJsonStateMachine->CurrentState != JsonStateError)
    {
        memcpy(pJsonItem->szItemName, pJsonItem->szStartOfItem+1, (pJsonStateMachine->pBufferStart-1) - (pJsonItem->szStartOfItem+1));
		bItemNameDecoded = TRUE;
	}
	
	return bItemNameDecoded;
}

 /***********************************************************************
  * JsonState_DecodeItemString
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
BOOL JsonState_DecodeItemString(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal, PJSON_ITEM pJsonItem)
{
    BOOL bItemStringDecoded = FALSE;
	UCHAR *pStartOfString;
	
	pStartOfString = pJsonStateMachine->pBufferStart;
	
	pJsonStateMachine->CurrentState = JsonState_FindNextStateToken(pJsonStateMachine, pJsonInternal);
   
    if(pJsonStateMachine->CurrentState != JsonStateError)
    {
        memcpy(pJsonItem->szItemString, pStartOfString, (pJsonStateMachine->pBufferStart-1) - (pStartOfString));
		bItemStringDecoded = TRUE;
	}
	
	return bItemStringDecoded;
}

 /***********************************************************************
  * JsonState_DecodeItemInt64
  *  
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
BOOL JsonState_DecodeItemInt64(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal, PJSON_ITEM pJsonItem)
{
    BOOL bItemIntDecoded = FALSE;
	UCHAR *pStartOfInteger;
	
	pStartOfInteger = pJsonStateMachine->pBufferStart-1;
	
	pJsonStateMachine->CurrentState = JsonState_FindNextStateToken(pJsonStateMachine, pJsonInternal);
   
    if(pJsonStateMachine->CurrentState != JsonStateError)
    {
        memcpy(pJsonItem->szItemString, pStartOfInteger, (pJsonStateMachine->pBufferStart-1) - (pStartOfInteger));
		pJsonItem->ItemInteger64 = atoi(pJsonItem->szItemString);
		bItemIntDecoded = TRUE;
	}
	
	return bItemIntDecoded;
}






 /***********************************************************************
  * JsonState_HandleStart
  *
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
JSON_STATE_ENUM JsonState_FindNextStateToken(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal)
{
    JSON_STATE_ENUM JsonState = JsonStateError;
	BOOL bFoundMatch;
	UINT Index;
	PSTATE_TRANSITION pStateTransition = &pJsonStateMachine->pStateTransition[pJsonStateMachine->CurrentState];
	
    if(JsonState_AdvanceIgnoreSpace(pJsonStateMachine, pJsonInternal))
	{
	   bFoundMatch = FALSE;
	   for(Index = 0; pStateTransition->TokenArray[Index] != '\0' && bFoundMatch == FALSE; Index++)
	   {
		   if(*pJsonStateMachine->pBufferStart == pStateTransition->TokenArray[Index])
		   {
			  if(JsonState_AdvanceStateMachinePointer(pJsonStateMachine, pJsonInternal))
			  { 
				 JsonState = pStateTransition->JsonState[Index];
				 //printf("**********************************\n");
				 //printf("Transition (%s -> %s) ***%s*** \n\n", g_pszStringNames[pJsonStateMachine->CurrentState], g_pszStringNames[JsonState], pJsonStateMachine->pBufferStart);
				 bFoundMatch = TRUE;
			  }
		   }
	   }
	}
	
	return JsonState;
}


 
 /***********************************************************************
  * JsonState_AdvanceWhiteSpace
  *
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
BOOL JsonState_AdvanceIgnoreSpace(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal)
{
    BOOL bFoundNonIgnoreSpace;
    BOOL bComplete;
	UINT Index;
	PSTATE_TRANSITION pStateTransition = &pJsonStateMachine->pStateTransition[pJsonStateMachine->CurrentState];
	
	bFoundNonIgnoreSpace = FALSE;
	bComplete = TRUE;
	
    while(bComplete != FALSE && bFoundNonIgnoreSpace == FALSE)
	{
	    bFoundNonIgnoreSpace = TRUE;
	    for(Index = 0; pStateTransition->IgnoreArray[Index]; Index++)
		{
	       if(*pJsonStateMachine->pBufferStart == pStateTransition->IgnoreArray[Index])
		   {
		      bFoundNonIgnoreSpace = FALSE;
		   }
		}
		
		if(bFoundNonIgnoreSpace == FALSE)
		{
	       bComplete = JsonState_AdvanceStateMachinePointer(pJsonStateMachine, pJsonInternal);
		}
	}
	
	return bComplete;
}

 /***********************************************************************
  * JsonState_AdvanceStateMachinePointer
  *
  *    
  *
  * Parameters
  *     
  * 
  * Return Value
  *     
  *
  ***********************************************************************/
BOOL JsonState_AdvanceStateMachinePointer(PJSON_STATE_MACHINE pJsonStateMachine, PJSON_INTERNAL pJsonInternal)
{
    BOOL bPointerAdvanced;
	
	bPointerAdvanced = FALSE;
	
    if((pJsonStateMachine->pBufferStart + 1) < pJsonInternal->pEndFileView)
	{
	    pJsonStateMachine->pBufferStart += 1;
		bPointerAdvanced = TRUE;		
	}
	
	return bPointerAdvanced;
}
