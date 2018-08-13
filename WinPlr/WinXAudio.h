/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio XAudio header
**********************************************************
* WinXAudio.h
* Master include file for WinXAudio.cpp file
*********************************************************/

#pragma once

#include "WinAudio.h"

#define STREAMING_BUFFER_SIZE 65536
#define MAX_BUFFER_COUNT 3
typedef struct  
{
	IXAudio2* lpXAudio;
	IXAudio2SourceVoice* lpXAudioSourceVoice;
	IXAudio2MasteringVoice* lpXAudioMasterVoice;
	XAUDIO2_VOICE_STATE voiceState;
	XAUDIO2_VOICE_SENDS voiceSends;
} XAUDIO_DATA, *XAUDIO_DATA_P;

VOID WINAPIV CreateXAudioThread(AUDIO_FILE* xAudioFile);

class XAudioPlayer
{
public:


	XAUDIO_DATA CreateXAudioDevice(FILE_DATA dData, PCM_DATA dPCM);
	VOID CreateXAudioState(XAUDIO_DATA audioStruct);
};