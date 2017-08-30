/***********************************************************************
 * taudio.CPP
 *  
 *    Audio
 *
 *
 *  Supports .WAV files, Very Simplistic Parser
 *
 *
 * Toby Opferman Copyright (c) 2003
 *
 ***********************************************************************/
 
 
 #include <windows.h>
 #include <mmsystem.h>
 #include <taudio.h>
 #include <stdio.h>
 
 
#define Audio_Debug1 

void DbgBreakPoint(void);
 
 /***********************************************************************
  * Internal Structures
  ***********************************************************************/
typedef struct {
    
    UCHAR IdentifierString[4];
    DWORD dwLength;

} RIFF_CHUNK, *PRIFF_CHUNK;


typedef struct {

    WORD  wFormatTag;         // Format category
    WORD  wChannels;          // Number of channels
    DWORD dwSamplesPerSec;    // Sampling rate
    DWORD dwAvgBytesPerSec;   // For buffer estimation
    WORD  wBlockAlign;        // Data block size
    WORD  wBitsPerSample;

} WAVE_FILE_HEADER, *PWAVE_FILE_HEADER;


typedef struct {

     WAVEFORMATEX WaveFormatEx;
     char *pSampleData;
     UINT Index;
     UINT Size;

} WAVE_SAMPLE, *PWAVE_SAMPLE;
 
#define SAMPLE_SIZE    (4000) 
#define PENDING_SAMPLES 10

typedef struct {
     
     HWAVEOUT hWaveOut;
     HANDLE hEvent;
     HANDLE hThread;
     UINT uiNumberOfSamples;
     UINT uiCurrentSample;

     UINT uiSampleList[PENDING_SAMPLES];
     UINT uiNumberOfPendingSamples;
     CRITICAL_SECTION csSampleListLock;

     WAVE_SAMPLE *pWaveSample;
     BOOL bWaveShouldDie;
     DWORD dwFlags;
     BOOL bShouldPlay;
     WAVEHDR WaveHdr[8];
     char AudioBuffer[8][SAMPLE_SIZE];

} TAUDIO, *PTAUDIO;



 /***********************************************************************
  * Internal Functions
  ***********************************************************************/
void CALLBACK TAudio_WaveOutputCallback(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
BOOL TAudio_OpenWaveSample(CHAR *pFileName, PWAVE_SAMPLE pWaveSample);
void TAudio_WaveOpen(HWAVEOUT hWaveOut, PTAUDIO pTAudio);
void TAudio_WaveDone(HWAVEOUT hWaveOut, PTAUDIO pTAudio);
DWORD WINAPI TAudio_AudioThread(PVOID pDataInput);
void TAudio_CreateThread(PTAUDIO pTAudio);
void TAudio_SetupAudio(PTAUDIO pTAudio);
void TAudio_WaveClose(HWAVEOUT hWaveOut, PTAUDIO pTAudio);
BOOL TAudio_AudioMixer(PTAUDIO pTAudio, UINT Index);
void Audio_Debug(char *pszFormatString, ...);

 /***********************************************************************
  * TAudio_Init
  *  
  *    Audio!
  *
  * Parameters
  *     Should pass file name in and then allow multiple files be opened.
  *     This would then have the KMIXER do the mixing of the audio streams.
  *     All streams should have the same PCM format.
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
 HTAUDIO TAudio_Init(char **ppszSoundWave, UINT uiNumberOfSamples, DWORD dwFlags) 
 {
     PTAUDIO pTAudio = NULL;
     UINT Index;
 
     pTAudio = (PTAUDIO)LocalAlloc(LMEM_ZEROINIT, sizeof(TAUDIO));

     if(pTAudio)
     {
         pTAudio->dwFlags           = dwFlags;
         pTAudio->uiNumberOfSamples = uiNumberOfSamples;
         pTAudio->uiCurrentSample   = 0;

         pTAudio->pWaveSample = (WAVE_SAMPLE *)LocalAlloc(LMEM_ZEROINIT, sizeof(WAVE_SAMPLE)*uiNumberOfSamples);

         if(pTAudio->pWaveSample)
         {
            for(Index = 0; Index < uiNumberOfSamples; Index++)
            {
                TAudio_OpenWaveSample(ppszSoundWave[Index], &pTAudio->pWaveSample[Index]);

                if(Index > 0)
                {
                    /* Check Sound Formats are Equal */
                    if(memcmp(&pTAudio->pWaveSample[0].WaveFormatEx, &pTAudio->pWaveSample[Index].WaveFormatEx, sizeof(WAVEFORMATEX)) != 0)
                    {
#ifdef DEBUG					    
                        DbgBreakPoint();
#else
                        ExitProcess(0xFF);						
#endif						
                    }
                }
            }
            InitializeCriticalSection(&pTAudio->csSampleListLock);

            waveOutOpen(&pTAudio->hWaveOut, WAVE_MAPPER, &pTAudio->pWaveSample[0].WaveFormatEx, (ULONG)TAudio_WaveOutputCallback, (ULONG)pTAudio, CALLBACK_FUNCTION);

            TAudio_CreateThread(pTAudio);
         }
     }

     return (HTAUDIO)pTAudio;
 }



  /***********************************************************************
  * TAudio_PlaySound
  *  
  *    Audio!
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void TAudio_PlaySound(HTAUDIO hTAudio, UINT uiEffectNumber)
{
   PTAUDIO pTAudio = (PTAUDIO)hTAudio;
   UINT SampleToPlay;

   EnterCriticalSection(&pTAudio->csSampleListLock);
   if(pTAudio->uiNumberOfPendingSamples < PENDING_SAMPLES)
   {
       SampleToPlay = pTAudio->uiNumberOfPendingSamples + 1;
       pTAudio->uiNumberOfPendingSamples++;

       if(SampleToPlay <= PENDING_SAMPLES)
       {
            pTAudio->uiSampleList[SampleToPlay - 1] = uiEffectNumber;
            pTAudio->bShouldPlay = TRUE;
            SetEvent(pTAudio->hEvent);
       }
   }
   LeaveCriticalSection(&pTAudio->csSampleListLock);
}


 /***********************************************************************
  * TAudio_PauseSound
  *  
  *    Audio!
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void TAudio_PauseSound(HTAUDIO hTAudio)
{
   PTAUDIO pTAudio = (PTAUDIO)hTAudio;
   EnterCriticalSection(&pTAudio->csSampleListLock);

   pTAudio->uiNumberOfPendingSamples = 0;
   pTAudio->bShouldPlay = FALSE;

   LeaveCriticalSection(&pTAudio->csSampleListLock);
}

 /***********************************************************************
  * TAudio_Init
  *  
  *    Audio!
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
 void TAudio_UnInit(HTAUDIO hTAudio)
 {
     PTAUDIO pTAudio = (PTAUDIO)hTAudio;

     pTAudio->bShouldPlay = FALSE;
     pTAudio->bWaveShouldDie = TRUE;

     SetEvent(pTAudio->hEvent);
     WaitForSingleObject(pTAudio->hThread, INFINITE);

     CloseHandle(pTAudio->hEvent);
     CloseHandle(pTAudio->hThread);

     waveOutClose(pTAudio->hWaveOut);

     DeleteCriticalSection(&pTAudio->csSampleListLock);
     LocalFree(pTAudio->pWaveSample);
     LocalFree(pTAudio);
 }
 
 
 /***********************************************************************
  * TAudio_WaveOutputCallback
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/ 
void CALLBACK TAudio_WaveOutputCallback(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    PTAUDIO pTAudio = (PTAUDIO)dwInstance;

    switch(uMsg)
    {
      case WOM_OPEN:
            TAudio_WaveOpen(hwo, pTAudio);
            break;

       case WOM_DONE:
            TAudio_WaveDone(hwo, pTAudio);
            break;

       case WOM_CLOSE:
            TAudio_WaveClose(hwo, pTAudio);
            break;
    }
}



 
 /***********************************************************************
  * TAudio_WaveOpen
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void TAudio_WaveOpen(HWAVEOUT hWaveOut, PTAUDIO pTAudio)
{
  // Do Nothing
}


 /***********************************************************************
  * TAudio_WaveDone
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void TAudio_WaveDone(HWAVEOUT hWaveOut, PTAUDIO pTAudio)
{
    Audio_Debug1("Set Event \n");
    SetEvent(pTAudio->hEvent);
}


 /***********************************************************************
  * TAudio_WaveClose
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void TAudio_WaveClose(HWAVEOUT hWaveOut, PTAUDIO pTAudio)
{
  // Do Nothing
}



 /***********************************************************************
  * TAudio_OpenWaveFile
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
BOOL TAudio_OpenWaveSample(CHAR *pFileName, PWAVE_SAMPLE pWaveSample)
{
    BOOL bSampleLoaded = FALSE;
    HANDLE hFile;
    RIFF_CHUNK RiffChunk = {0};
    DWORD dwBytes;
    WAVE_FILE_HEADER WaveFileHeader;
    DWORD dwIncrementBytes;
    char szIdentifier[5] = {0};

    if((hFile = CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
    {
        SetFilePointer(hFile, 12, NULL, FILE_CURRENT);
        

        ReadFile(hFile, &RiffChunk, sizeof(RiffChunk), &dwBytes, NULL);
        ReadFile(hFile, &WaveFileHeader, sizeof(WaveFileHeader), &dwBytes, NULL);

        pWaveSample->WaveFormatEx.wFormatTag      = WaveFileHeader.wFormatTag;         
        pWaveSample->WaveFormatEx.nChannels       = WaveFileHeader.wChannels;          
        pWaveSample->WaveFormatEx.nSamplesPerSec  = WaveFileHeader.dwSamplesPerSec;    
        pWaveSample->WaveFormatEx.nAvgBytesPerSec = WaveFileHeader.dwAvgBytesPerSec;   
        pWaveSample->WaveFormatEx.nBlockAlign     = WaveFileHeader.wBlockAlign;  
        pWaveSample->WaveFormatEx.wBitsPerSample  = WaveFileHeader.wBitsPerSample;
        pWaveSample->WaveFormatEx.cbSize          = sizeof(pWaveSample->WaveFormatEx);

        dwIncrementBytes = dwBytes;

        do {
             SetFilePointer(hFile, RiffChunk.dwLength - dwIncrementBytes, NULL, FILE_CURRENT);

             ReadFile(hFile, &RiffChunk, sizeof(RiffChunk), &dwBytes, NULL);
             
             dwIncrementBytes = 0;

             memcpy(szIdentifier, RiffChunk.IdentifierString, 4); 

        } while(_stricmp(szIdentifier, "data")) ;

        pWaveSample->pSampleData = (char *)LocalAlloc(LMEM_ZEROINIT, RiffChunk.dwLength);

        pWaveSample->Size = RiffChunk.dwLength;

        ReadFile(hFile, pWaveSample->pSampleData, RiffChunk.dwLength, &dwBytes, NULL);

        CloseHandle(hFile);

        bSampleLoaded = TRUE;
    }

    return bSampleLoaded;
}





 /***********************************************************************
  * TAudio_CreateThread
  *  
  *    Audio Callback 
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void TAudio_CreateThread(PTAUDIO pTAudio)
{
    DWORD dwThreadId;

    pTAudio->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    pTAudio->hThread = CreateThread(NULL, 0, TAudio_AudioThread, pTAudio, 0, &dwThreadId);

}

 /***********************************************************************
  * TAudio_AudioThread
  *  
  *    Audio Thread
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
DWORD WINAPI TAudio_AudioThread(PVOID pDataInput)
{
    PTAUDIO pTAudio = (PTAUDIO)pDataInput;
    DWORD dwReturnValue = 0;
    UINT Index;
    BOOL bLoopComplete;
    UINT SampleToPlay;

    TAudio_SetupAudio(pTAudio);

    if(pTAudio->dwFlags == 0)
    {
        bLoopComplete = TRUE;
    }
    else
    {
        bLoopComplete = FALSE;
    }

    while(!pTAudio->bWaveShouldDie)
    {

        WaitForSingleObject(pTAudio->hEvent, INFINITE);

        if(pTAudio->dwFlags == 0 && bLoopComplete)
        {
            EnterCriticalSection(&pTAudio->csSampleListLock);
            
            SampleToPlay = pTAudio->uiNumberOfPendingSamples;

            if(SampleToPlay > 0)
            {
                if(SampleToPlay > PENDING_SAMPLES)
                {
                    pTAudio->uiNumberOfPendingSamples = PENDING_SAMPLES;
                    SampleToPlay = PENDING_SAMPLES;
                }

                pTAudio->uiNumberOfPendingSamples--;

                bLoopComplete = FALSE;
                pTAudio->uiCurrentSample = pTAudio->uiSampleList[SampleToPlay - 1];

                //SetEvent(pTAudio->hEvent);
                //pTAudio->bShouldPlay = TRUE;
            }
            else
            {
                bLoopComplete = TRUE;
                pTAudio->bShouldPlay = FALSE;
            }
            LeaveCriticalSection(&pTAudio->csSampleListLock);
        }

        Audio_Debug1("Performing the loop, bShouldPlay = %i, bLoopComplete = %i\n", pTAudio->bShouldPlay, bLoopComplete);

        for(Index = 0; Index < 8 && pTAudio->bShouldPlay && !bLoopComplete; Index++)
        {
            if(pTAudio->WaveHdr[Index].dwFlags & WHDR_DONE)
            {
               bLoopComplete = TAudio_AudioMixer(pTAudio, Index);
               waveOutWrite(pTAudio->hWaveOut, &pTAudio->WaveHdr[Index], sizeof(WAVEHDR));
            }
        }
    }

    waveOutReset(pTAudio->hWaveOut);

    return dwReturnValue;
}




 /***********************************************************************
  * TAudio_AudioMixer
  *  
  *    Audio Mixer
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
BOOL TAudio_AudioMixer(PTAUDIO pTAudio, UINT Index)
{
    UINT uiBytesNotUsed = SAMPLE_SIZE;
    BOOL bLoopComplete = FALSE;
    PWAVE_SAMPLE pCurrentWaveSample;

    pCurrentWaveSample = &pTAudio->pWaveSample[pTAudio->uiCurrentSample];
    
    pTAudio->WaveHdr[Index].dwFlags &= ~WHDR_DONE;

    if(pCurrentWaveSample->Size - pCurrentWaveSample->Index < uiBytesNotUsed)
    {
        memcpy(pTAudio->AudioBuffer[Index], pCurrentWaveSample->pSampleData + pCurrentWaveSample->Index, pCurrentWaveSample->Size - pCurrentWaveSample->Index);
        Audio_Debug1("1. Sample %i, Size = %i \n", Index, pCurrentWaveSample->Size - pCurrentWaveSample->Index);

        uiBytesNotUsed -= (pCurrentWaveSample->Size - pCurrentWaveSample->Index);

        if(pTAudio->dwFlags & FLAG_LOOP_SOUND)
        {

            memcpy(pTAudio->AudioBuffer[Index], pCurrentWaveSample->pSampleData, uiBytesNotUsed);

            pCurrentWaveSample->Index = uiBytesNotUsed;
            uiBytesNotUsed = 0;
        }
        else
        {
            bLoopComplete = TRUE;
            pCurrentWaveSample->Index = 0;
        }
    }
    else
    {
       memcpy(pTAudio->AudioBuffer[Index], pCurrentWaveSample->pSampleData + pCurrentWaveSample->Index, uiBytesNotUsed);
       Audio_Debug1("2. Sample %i, Size = %i \n", Index, uiBytesNotUsed);

       pCurrentWaveSample->Index += SAMPLE_SIZE;
       uiBytesNotUsed = 0;
    }

    if(pCurrentWaveSample->Index >= pCurrentWaveSample->Size)
    {
        Audio_Debug1("Index Cleared %i, %i\n", pCurrentWaveSample->Index, pCurrentWaveSample->Size);
        pCurrentWaveSample->Index = 0;
        if((pTAudio->dwFlags & FLAG_LOOP_SOUND) == 0)
        {
            bLoopComplete = TRUE;
        }
    }

    pTAudio->WaveHdr[Index].lpData = pTAudio->AudioBuffer[Index];

    pTAudio->WaveHdr[Index].dwBufferLength = SAMPLE_SIZE - uiBytesNotUsed;
    Audio_Debug1("reported buffer length %i\n", pTAudio->WaveHdr[Index].dwBufferLength);

    return bLoopComplete;
}

 /***********************************************************************
  * TAudio_SetupAudio
  *  
  *    Audio Thread
  *
  * Parameters
  *     
  * 
  * Return Value
  *     Handle To This Audio Session
  *
  ***********************************************************************/
void TAudio_SetupAudio(PTAUDIO pTAudio)
{
    UINT Index = 0;

    for(Index = 0; Index < 8; Index++)
    {
        pTAudio->WaveHdr[Index].dwBufferLength = SAMPLE_SIZE;
        pTAudio->WaveHdr[Index].lpData         = pTAudio->AudioBuffer[Index]; 

        waveOutPrepareHeader(pTAudio->hWaveOut, &pTAudio->WaveHdr[Index], sizeof(WAVEHDR));

        TAudio_AudioMixer(pTAudio, Index);

//		waveOutWrite(pTAudio->hWaveOut, &pTAudio->WaveHdr[Index], sizeof(WAVEHDR));
        pTAudio->WaveHdr[Index].dwFlags |= WHDR_DONE;
    }
}


/***********************************************************************
 * Audio_Debug
 *  
 *    Debug Shit
 *
 *    
 *
 * Parameters
 *     Debug
 *
 * Return Value
 *     Nothing
 *
 ***********************************************************************/
 void Audio_Debug(char *pszFormatString, ...)
 {
     char DebugString[256];
     va_list vl;

     va_start(vl, pszFormatString);
     vsprintf(DebugString, pszFormatString, vl);
     va_end(vl);

     OutputDebugStringA(DebugString);
 } 