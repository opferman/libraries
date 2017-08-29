/***********************************************************************
 * taudio.h
 *  
 *    Audio
 *
 *
 * Toby Opferman Copyright (c) 2003
 *
 ***********************************************************************/


#ifndef __TAUDIO_H__
#define __TAUDIO_H__

#define FLAG_LOOP_SOUND   0x1

typedef PVOID HTAUDIO;

HTAUDIO TAudio_Init(char **ppszSoundWave, UINT uiNumberOfSamples, DWORD dwFlags) ;
void TAudio_PlaySound(HTAUDIO hTAudio, UINT uiEffectNumber);
void TAudio_PauseSound(HTAUDIO hTAudio);
void TAudio_UnInit(HTAUDIO hTAudio);


#endif

