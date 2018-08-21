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

VOID WINAPIV CreateXAudioThread(_In_ LPVOID lpFile);

class XAudioPlayer : public IXAudio2VoiceCallback
{
public:
	HANDLE hBufferEndEvent;

	STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32) override
	{
	}
	STDMETHOD_(void, OnVoiceProcessingPassEnd)() override
	{
	}
	STDMETHOD_(void, OnStreamEnd)() override
	{
	}
	STDMETHOD_(void, OnBufferStart)(void*) override
	{
	}
	STDMETHOD_(void, OnBufferEnd)(void*) override
	{
		SetEvent(hBufferEndEvent);
	}
	STDMETHOD_(void, OnLoopEnd)(void*) override
	{
	}
	STDMETHOD_(void, OnVoiceError)(void*, HRESULT) override
	{
	}

	XAudioPlayer() : hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
	~XAudioPlayer() { CloseHandle(hBufferEndEvent); }

	XAUDIO_DATA CreateXAudioDevice(_In_ FILE_DATA dData, _In_ PCM_DATA dPCM);
	VOID CreateXAudioState(_In_ XAUDIO_DATA audioStruct);
};
