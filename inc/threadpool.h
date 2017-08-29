/***********************************************************************
 * 
 *  
 *    
 *
 *
 * Toby Opferman Copyright (c) 2013
 *
 ***********************************************************************/

#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__




typedef PVOID HTHREADPOOL;

typedef BOOL (*PFN_WORKER_CB)(HTHREADPOOL hLocalThreadPool, PVOID pGlobalContext, UINT MessageId, PVOID pWorkItem);
typedef BOOL (*PFN_WORK_COMPLETE_CB)(HTHREADPOOL hThreadPool, PVOID pGlobalContext, UINT MessageId, PVOID pWorkItem, PVOID pContext, BOOL bCanceled);



HTHREADPOOL ThreadPool_Create(UINT uiNumberOfThreads, PFN_WORKER_CB pfnWorkerCB, UINT QueuePoolCacheSize, PVOID pGlobalContext);
BOOL ThreadPool_SwitchThreadToTimer(HTHREADPOOL hThreadPool, PFN_WORKER_CB pfnWorkerCB, ULONG64 Milliseconds);
BOOL ThreadPool_SwitchTimerOff(HTHREADPOOL hThreadPool);
BOOL ThreadPool_SendThreadWorkAsync(HTHREADPOOL hThreadPool, UINT MessageId, PVOID pWorkItem, PFN_WORK_COMPLETE_CB pfnTpWorkCompleteCB, PVOID pContext);
BOOL ThreadPool_SendThreadWorkSync(HTHREADPOOL hThreadPool, UINT MessageId, PVOID pWorkItem);
void ThreadPool_Destroy(HTHREADPOOL hThreadPool);

#endif
