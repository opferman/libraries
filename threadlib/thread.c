/***********************************************************************
 * thread.C
 *  
 *    
 *
 *
 * Toby Opferman Copyright (c) 2013
 *
 ***********************************************************************/
 
 
#include <windows.h>
#include <threadpool.h>



typedef struct _WORK_ITEM
{
    UINT MessageId;
    PVOID pWorkItem;
    BOOL bSetEvent;
    HANDLE hCompleteEvent;
    PFN_WORK_COMPLETE_CB pfnTpWorkCompleteCB;
    PVOID pContext;
    BOOL WorkItemIsCache;
    BOOL WorkItemNeedsFreed;
    BOOL *pbWorkItemCanceled;
    struct _WORK_ITEM *pNext;
    
} WORK_ITEM, *PWORK_ITEM;

typedef struct _INTERNAL_THREAD_POOL
{
   struct _INTERNAL_THREAD_POOL *pMasterThreadPool;
   
   HANDLE *phThreadHandles;
   UINT NumberOfThreads;
   
   BOOL bThreadConvertedToTimer;
   ULONG64 ulMillisecondTimer;
   PFN_WORKER_CB pfnWorkerCb;
   PVOID pGlobalContext;
   HANDLE hTerminateEvent;
   HANDLE hWorkAvailable;
   BOOL bAlive;
   
   CRITICAL_SECTION csPoolCriticalSection;
   PWORK_ITEM pWorkItemInsert;
   PWORK_ITEM pWorkItemRemove ;
   
   PWORK_ITEM pWorkItemFreeList;
   
} INTERNAL_THREAD_POOL, *PINTERNAL_THREAD_POOL;

 /***********************************************************************
  * Prototypes
  ***********************************************************************/
DWORD CALLBACK ThreadPool_WorkerThread(PVOID pContext);
PWORK_ITEM ThreadPool_GetWorkItem(PINTERNAL_THREAD_POOL pThreadPool, BOOL bFailOnCacheMiss);
void ThreadPool_InsertWorkItem(PINTERNAL_THREAD_POOL pThreadPool, PWORK_ITEM pWorkItem);
PWORK_ITEM ThreadPool_AllocWorkItem(BOOL bWorkItemIsCache);
PWORK_ITEM ThreadPool_RemoveWorkItem(PINTERNAL_THREAD_POOL pThreadPool);
void ThreadPool_WorkerTimerThread(PINTERNAL_THREAD_POOL pLocalThreadPool);
void ThreadPool_ReleaseWorkItem(PINTERNAL_THREAD_POOL pThreadPool, PWORK_ITEM pWorkItem, BOOL bCancelWorkItem);

 /***********************************************************************
  * ThreadPool_Create
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
HTHREADPOOL ThreadPool_Create(UINT uiNumberOfThreads, PFN_WORKER_CB pfnWorkerCB, UINT QueuePoolCacheSize, PVOID pGlobalContext)
{
    PINTERNAL_THREAD_POOL pInternalThreadPool = NULL;
    UINT Index;
    PWORK_ITEM pWorkItem;
    
    pInternalThreadPool = (PINTERNAL_THREAD_POOL)LocalAlloc(LMEM_ZEROINIT, sizeof(INTERNAL_THREAD_POOL));
    
    if(pInternalThreadPool)
    {
         pInternalThreadPool->phThreadHandles = (HANDLE *)LocalAlloc(LMEM_ZEROINIT, sizeof(HANDLE)*uiNumberOfThreads);
         
         if(pInternalThreadPool->phThreadHandles)
         {
             InitializeCriticalSection(&pInternalThreadPool->csPoolCriticalSection);
             
             pInternalThreadPool->pGlobalContext  = pGlobalContext;
             pInternalThreadPool->bAlive          = TRUE;
             pInternalThreadPool->pfnWorkerCb     = pfnWorkerCB;
             pInternalThreadPool->hTerminateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
             
             if(pInternalThreadPool->hTerminateEvent != NULL)
             {
                 pInternalThreadPool->hWorkAvailable  = CreateEvent(NULL, TRUE, FALSE, NULL);
             }
             
             if(pInternalThreadPool->hWorkAvailable != NULL)
             {
                 for(Index = 0; Index < QueuePoolCacheSize; Index++)
                 {
                     pWorkItem = ThreadPool_AllocWorkItem(TRUE);
                     if(pWorkItem)
                     {
                         pWorkItem->pNext = pInternalThreadPool->pWorkItemFreeList;
                         pInternalThreadPool->pWorkItemFreeList = pWorkItem;
                     }
                 }
                 
                 for(Index = 0; Index < uiNumberOfThreads; Index++)
                 {		 
                     pInternalThreadPool->phThreadHandles[Index] = CreateThread(NULL, 0, ThreadPool_WorkerThread, pInternalThreadPool, 0, NULL);
                         
                     if(pInternalThreadPool->phThreadHandles[Index] != NULL && pInternalThreadPool->phThreadHandles[Index] != INVALID_HANDLE_VALUE)
                     {
                          pInternalThreadPool->NumberOfThreads++;
                     }
                     else
                     {
                          Index = uiNumberOfThreads;
                     }
                 }
            }
            
            if(pInternalThreadPool->NumberOfThreads == 0)
            {
                 ThreadPool_Destroy((HTHREADPOOL)pInternalThreadPool);
                 pInternalThreadPool = NULL;
            }             			 
        }
        else
        {
           LocalFree(pInternalThreadPool);
           pInternalThreadPool = NULL;		   
        }
    }

    return (HTHREADPOOL)pInternalThreadPool;
}

 /***********************************************************************
  * ThreadPool_Destroy
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
void ThreadPool_Destroy(HTHREADPOOL hThreadPool)
{
    PINTERNAL_THREAD_POOL pInternalThreadPool = (PINTERNAL_THREAD_POOL)hThreadPool;
    PWORK_ITEM pWorkItem;
        
    pInternalThreadPool->bAlive = FALSE;
    
    if(pInternalThreadPool->hTerminateEvent)
    {
       SetEvent(pInternalThreadPool->hTerminateEvent);
       
       if(pInternalThreadPool->NumberOfThreads != 0)
       {
          WaitForMultipleObjects(pInternalThreadPool->NumberOfThreads, pInternalThreadPool->phThreadHandles, TRUE, INFINITE);
       }	   
    }
    
    while(pWorkItem = ThreadPool_RemoveWorkItem(pInternalThreadPool))
    {
        ThreadPool_ReleaseWorkItem(pInternalThreadPool, pWorkItem, TRUE);	
    }
    
    if(pInternalThreadPool->hTerminateEvent)
    {
        CloseHandle(pInternalThreadPool->hTerminateEvent);
    }
    
    if(pInternalThreadPool->hWorkAvailable)
    {
        CloseHandle(pInternalThreadPool->hWorkAvailable);
    }
    
    DeleteCriticalSection(&pInternalThreadPool->csPoolCriticalSection);
     
    while(pInternalThreadPool->pWorkItemFreeList)
    {
         pWorkItem = pInternalThreadPool->pWorkItemFreeList;
         pInternalThreadPool->pWorkItemFreeList = pWorkItem->pNext;
         CloseHandle(pWorkItem->hCompleteEvent);
         LocalFree(pWorkItem);
    }			
                 
    LocalFree(pInternalThreadPool);
    pInternalThreadPool = NULL;
}


/***********************************************************************
  * ThreadPool_SendThreadWorkAsync
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
BOOL ThreadPool_SendThreadWorkAsync(HTHREADPOOL hThreadPool, UINT MessageId, PVOID pWorkItemCtx, PFN_WORK_COMPLETE_CB pfnTpWorkCompleteCB, PVOID pContext)
{
    PINTERNAL_THREAD_POOL pThreadPool = (PINTERNAL_THREAD_POOL)hThreadPool;
    BOOL bMsgSent = FALSE;
    PWORK_ITEM pWorkItem;
    
    if(pThreadPool->pMasterThreadPool)
    {
       pThreadPool = pThreadPool->pMasterThreadPool;
    }
    
    if(pThreadPool->bAlive)
    {
        pWorkItem = ThreadPool_GetWorkItem(pThreadPool, FALSE);
        
        if(pWorkItem)
        {
            bMsgSent = TRUE;
            pWorkItem->MessageId = MessageId;
            pWorkItem->pWorkItem = pWorkItemCtx;
            pWorkItem->pbWorkItemCanceled = NULL;
            pWorkItem->bSetEvent = FALSE;
            pWorkItem->pfnTpWorkCompleteCB = pfnTpWorkCompleteCB;
            pWorkItem->pContext = pContext;
            
            ThreadPool_InsertWorkItem(pThreadPool, pWorkItem);
            
        }
    }

    return bMsgSent;  
}


 /***********************************************************************
  * ThreadPool_SendThreadWorkSync
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
BOOL ThreadPool_SendThreadWorkSync(HTHREADPOOL hThreadPool, UINT MessageId, PVOID pWorkItemCtx)
{
    PINTERNAL_THREAD_POOL pThreadPool = (PINTERNAL_THREAD_POOL)hThreadPool;
    BOOL bMsgSent;
    PWORK_ITEM pWorkItem;
    BOOL bLocalContextUsed = FALSE;
    BOOL bWorkItemCanceled = FALSE;
    WORK_ITEM WorkItem = {0};
    
    bMsgSent = FALSE;
    
    if(pThreadPool->pMasterThreadPool && pThreadPool->bAlive)
    {
       bMsgSent = TRUE;
       /* 
        * Sync Messages can deadlock the system if called on the message threads themselves.
        */
       pThreadPool->pfnWorkerCb((HTHREADPOOL)pThreadPool, pThreadPool->pMasterThreadPool->pGlobalContext, MessageId, pWorkItemCtx);
    }
    else
    {
    
        if(pThreadPool->bAlive)
        {
            bMsgSent = TRUE; 
        
            pWorkItem = ThreadPool_GetWorkItem(pThreadPool, TRUE);
            
            if(pWorkItem == NULL)
            {
                bLocalContextUsed = TRUE;
                pWorkItem = &WorkItem;
                pWorkItem->hCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                
                if(pWorkItem->hCompleteEvent == NULL)
                {
                   bMsgSent = FALSE;
                }
            }
            
            if(bMsgSent)
            {
                pWorkItem->MessageId = MessageId;
                pWorkItem->pWorkItem = pWorkItemCtx;
                pWorkItem->pbWorkItemCanceled = &bWorkItemCanceled;
                pWorkItem->bSetEvent = TRUE;
                
                ThreadPool_InsertWorkItem(pThreadPool, pWorkItem);
                
                WaitForSingleObject(pWorkItem->hCompleteEvent, INFINITE);
                
                bMsgSent = !bWorkItemCanceled;
                
                if(bLocalContextUsed)
                {
                    CloseHandle(WorkItem.hCompleteEvent);
                }
            }
        }
    }

    return bMsgSent;  
}

 /***********************************************************************
  * ThreadPool_InsertWorkItem
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
void ThreadPool_InsertWorkItem(PINTERNAL_THREAD_POOL pThreadPool, PWORK_ITEM pWorkItem)
{
    EnterCriticalSection(&pThreadPool->csPoolCriticalSection);
    
    if(pThreadPool->pWorkItemInsert)
    {
        pThreadPool->pWorkItemInsert->pNext = pWorkItem;
        pThreadPool->pWorkItemInsert = pWorkItem;
    }
    else
    {
        pThreadPool->pWorkItemRemove = pWorkItem;
        pThreadPool->pWorkItemInsert = pWorkItem;    
    }
                     
    SetEvent(pThreadPool->hWorkAvailable);
    LeaveCriticalSection(&pThreadPool->csPoolCriticalSection);
}


 /***********************************************************************
  * ThreadPool_InsertWorkItem
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
PWORK_ITEM ThreadPool_RemoveWorkItem(PINTERNAL_THREAD_POOL pThreadPool)
{
    PWORK_ITEM pWorkItem = NULL;
    
    EnterCriticalSection(&pThreadPool->csPoolCriticalSection);

    if(pThreadPool->pWorkItemRemove)
    {
        pWorkItem = pThreadPool->pWorkItemRemove;
        
        pThreadPool->pWorkItemRemove = pWorkItem->pNext;
                     
        if(pWorkItem == pThreadPool->pWorkItemInsert)
        {
            pThreadPool->pWorkItemInsert = pThreadPool->pWorkItemRemove;
            ResetEvent(pThreadPool->hWorkAvailable);
        }
        
        pWorkItem->pNext = NULL;
    }
    else
    {
        ResetEvent(pThreadPool->hWorkAvailable);
    }
    
    LeaveCriticalSection(&pThreadPool->csPoolCriticalSection);
    
    return pWorkItem;
}

 



 /***********************************************************************
  * ThreadPool_GetWorkItem
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
PWORK_ITEM ThreadPool_GetWorkItem(PINTERNAL_THREAD_POOL pThreadPool, BOOL bFailOnCacheMiss)
{
    PWORK_ITEM pWorkItem = NULL;
    
    EnterCriticalSection(&pThreadPool->csPoolCriticalSection);
    
    if(pThreadPool->pWorkItemFreeList)
    {
        pWorkItem = pThreadPool->pWorkItemFreeList;
        pThreadPool->pWorkItemFreeList = pThreadPool->pWorkItemFreeList->pNext;
        pWorkItem->pNext = NULL;
    }
    
    LeaveCriticalSection(&pThreadPool->csPoolCriticalSection);
    
    if(pWorkItem == NULL && bFailOnCacheMiss == FALSE)
    {
       pWorkItem = ThreadPool_AllocWorkItem(FALSE);
    }
    
    return pWorkItem;
}

 /***********************************************************************
  * ThreadPool_AllocWorkItem
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
PWORK_ITEM ThreadPool_AllocWorkItem(BOOL bWorkItemIsCache)
{
    PWORK_ITEM pWorkItem;
    
    pWorkItem = (PWORK_ITEM)LocalAlloc(LMEM_ZEROINIT, sizeof(WORK_ITEM));
    
    if(pWorkItem)
    {
        pWorkItem->WorkItemIsCache    = bWorkItemIsCache;
        pWorkItem->WorkItemNeedsFreed = !bWorkItemIsCache;
        pWorkItem->hCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        
        if(pWorkItem->hCompleteEvent == NULL)
        {
           LocalFree(pWorkItem);
           pWorkItem = NULL;
        }
    }
    
    return pWorkItem;
}

 /***********************************************************************
  * ThreadPool_SwitchThreadToTimer
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
BOOL ThreadPool_SwitchThreadToTimer(HTHREADPOOL hThreadPool, PFN_WORKER_CB pfnWorkerCB, ULONG64 Milliseconds)
{
    PINTERNAL_THREAD_POOL pInternalThreadPool = (PINTERNAL_THREAD_POOL)hThreadPool;
    BOOL bSwitchedThreadToTimer = FALSE;
    
    if(pInternalThreadPool->pMasterThreadPool && pInternalThreadPool->bThreadConvertedToTimer == FALSE)
    {
        pInternalThreadPool->ulMillisecondTimer      = Milliseconds;
        pInternalThreadPool->bThreadConvertedToTimer = TRUE;
        pInternalThreadPool->pfnWorkerCb             = pfnWorkerCB;
        bSwitchedThreadToTimer                       = TRUE;
    }
    
    return bSwitchedThreadToTimer;
}


 /***********************************************************************
  * ThreadPool_ReleaseWorkItem
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
void ThreadPool_ReleaseWorkItem(PINTERNAL_THREAD_POOL pThreadPool, PWORK_ITEM pWorkItem, BOOL bCancelWorkItem)
{
     /*
      * Work Item is one of the following:
      *
      *   1. A cached work item and needs to be put back on the work item queue.
      *   2. An item that was allocated by the caller and we need to free it.
      *   3. An item that is on the caller's stack and it will be freed once we complete the event.
      *
      *   There are rules based on the origin of the work item in that once the item is "completed" we either
      *   can or cannot access the variable any more. Once it is on the cached work item list we cannot access it
      *   once the lock is released for example.  If it is on the caller's stack we cannot access it after we complete the event.
      */ 
      
     if(pWorkItem->pbWorkItemCanceled)
     {
         *pWorkItem->pbWorkItemCanceled = bCancelWorkItem;
     }
     
     if(pWorkItem->WorkItemIsCache)
     { 
        EnterCriticalSection(&pThreadPool->csPoolCriticalSection);
         
        if(pWorkItem->bSetEvent)
        {
           SetEvent(pWorkItem->hCompleteEvent);
        }
        else
        {
            if(pWorkItem->pfnTpWorkCompleteCB)
            {
                  pWorkItem->pfnTpWorkCompleteCB((HTHREADPOOL)pThreadPool, pThreadPool->pGlobalContext, pWorkItem->MessageId, pWorkItem->pWorkItem, pWorkItem->pContext, bCancelWorkItem);
            }
        }
        
        pWorkItem->MessageId           = 0;
        pWorkItem->pfnTpWorkCompleteCB = NULL;
        pWorkItem->pContext            = NULL;
        pWorkItem->bSetEvent           = FALSE;
        pWorkItem->pbWorkItemCanceled  = NULL;
         
        pWorkItem->pNext  = pThreadPool->pWorkItemFreeList;
        pThreadPool->pWorkItemFreeList = pWorkItem;
        LeaveCriticalSection(&pThreadPool->csPoolCriticalSection);				 
     }
     else
     {
         if(pWorkItem->WorkItemNeedsFreed)
         {
                if(pWorkItem->bSetEvent)
                {
                   SetEvent(pWorkItem->hCompleteEvent);
                }
                else
                {
                    if(pWorkItem->pfnTpWorkCompleteCB)
                    {
                          pWorkItem->pfnTpWorkCompleteCB((HTHREADPOOL)pThreadPool, pThreadPool->pGlobalContext, pWorkItem->MessageId, pWorkItem->pWorkItem, pWorkItem->pContext, bCancelWorkItem);
                    }
                }

                LocalFree(pWorkItem);
         }
         else
         {
                if(pWorkItem->bSetEvent)
                {
                   SetEvent(pWorkItem->hCompleteEvent);
                }
                else
                {
                    if(pWorkItem->pfnTpWorkCompleteCB)
                    {
                          pWorkItem->pfnTpWorkCompleteCB((HTHREADPOOL)pThreadPool, pThreadPool->pGlobalContext, pWorkItem->MessageId, pWorkItem->pWorkItem, pWorkItem->pContext, bCancelWorkItem);
                    }
                }
         }		 
     }
}

/***********************************************************************
  * ThreadPool_AllocWorkItem
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
BOOL ThreadPool_SwitchTimerOff(HTHREADPOOL hThreadPool)
{
    PINTERNAL_THREAD_POOL pInternalThreadPool = (PINTERNAL_THREAD_POOL)hThreadPool;
    BOOL bSwitchedTimerOff = FALSE;
    
    if(pInternalThreadPool->pMasterThreadPool && pInternalThreadPool->bThreadConvertedToTimer != FALSE)
    {
    
        pInternalThreadPool->bThreadConvertedToTimer = FALSE;
        bSwitchedTimerOff                            = TRUE;
    }
    
    return bSwitchedTimerOff;
}


 /*******************************k****************************************
  * ThreadPool_WorkerThread
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
DWORD CALLBACK ThreadPool_WorkerThread(PVOID pContext)
{
    INTERNAL_THREAD_POOL LocalThreadHandle = {0};
    PINTERNAL_THREAD_POOL pThreadPool = (PINTERNAL_THREAD_POOL)pContext;
    HANDLE hEvents[2];
    PWORK_ITEM pWorkItem;
    
    LocalThreadHandle.pMasterThreadPool = pThreadPool;
    
    hEvents[0] = pThreadPool->hTerminateEvent;
    hEvents[1] = pThreadPool->hWorkAvailable;
    
    while(pThreadPool->bAlive)
    {
         if(LocalThreadHandle.bThreadConvertedToTimer)
         {
             ThreadPool_WorkerTimerThread(&LocalThreadHandle);
         }
         else
         {
             WaitForMultipleObjects(2, &hEvents[0], FALSE, INFINITE);
             
             if(pThreadPool->bAlive)
             {
                 pWorkItem = ThreadPool_RemoveWorkItem(pThreadPool);
                 
                 if(pWorkItem)
                 {
                     pThreadPool->pfnWorkerCb((HTHREADPOOL)&LocalThreadHandle, pThreadPool->pGlobalContext, pWorkItem->MessageId, pWorkItem->pWorkItem);
                     
                     ThreadPool_ReleaseWorkItem(pThreadPool, pWorkItem, FALSE);
                }
             }
         }
         
    }
    
    return 0;
}



 /*******************************k****************************************
  * ThreadPool_WorkerTimerThread
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
void ThreadPool_WorkerTimerThread(PINTERNAL_THREAD_POOL pLocalThreadPool)
{
    WaitForSingleObject(pLocalThreadPool->pMasterThreadPool->hTerminateEvent, (ULONG)pLocalThreadPool->ulMillisecondTimer);
             
    if(pLocalThreadPool->pMasterThreadPool->bAlive)
    {
        pLocalThreadPool->pfnWorkerCb((HTHREADPOOL)pLocalThreadPool, pLocalThreadPool->pMasterThreadPool->pGlobalContext, 0, NULL);
    }
}
 