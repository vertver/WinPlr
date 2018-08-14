/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio XAudio impl.
**********************************************************
* WinXAudio.cpp
* XAudio implementation
*********************************************************/

#include "WinXAudio.h"

Player::ThreadSystem sysThread;

/*************************************************
* CreateXAudioDevice():
* Checks buffer on validate
*************************************************/
XAUDIO_DATA
XAudioPlayer::CreateXAudioDevice(
	FILE_DATA dData,
	PCM_DATA dPCM
)
{
	HRESULT hr = NULL;
	XAUDIO_DATA audioStruct = {};
	WAVEFORMATEX waveFormat = {};
	XAUDIO_DATA xaudioData = {};
	ZeroMemory(&xaudioData, sizeof(XAUDIO_DATA));
	ZeroMemory(&audioStruct, sizeof(XAUDIO_DATA));
	ZeroMemory(&waveFormat, sizeof(WAVEFORMATEX));

	if (dData.dwSize)
	{
		// set data to struct
		waveFormat.cbSize = sizeof(WAVEFORMATEX);
		waveFormat.nAvgBytesPerSec = dPCM.waveFormat.nAvgBytesPerSec;
		waveFormat.nBlockAlign = dPCM.waveFormat.nBlockAlign;
		waveFormat.nChannels = dPCM.waveFormat.nChannels;
		waveFormat.nSamplesPerSec = dPCM.waveFormat.nSamplesPerSec;
		waveFormat.wBitsPerSample = dPCM.waveFormat.wBitsPerSample;
		waveFormat.wFormatTag = dPCM.waveFormat.wFormatTag;

		// use default XAudio processor
		hr = XAudio2Create(&audioStruct.lpXAudio);
		R_ASSERT2(hr, "Can't create XAudio device. Please, use DirectSound method.");

		// create default mastering voice
		hr = audioStruct.lpXAudio->CreateMasteringVoice(&audioStruct.lpXAudioMasterVoice);
		R_ASSERT2(hr, "Can't create XAudio Mastering voice.");

		// create XAudio descriptor (pOutput is MasteringVoice)
		XAUDIO2_SEND_DESCRIPTOR descAudio = {};
		descAudio.Flags = NULL;
		descAudio.pOutputVoice = audioStruct.lpXAudioMasterVoice;

		audioStruct.voiceSends.SendCount = 1;
		audioStruct.voiceSends.pSends = &descAudio;

		// create default source voice
		hr = audioStruct.lpXAudio->CreateSourceVoice(
			&audioStruct.lpXAudioSourceVoice,
			&waveFormat,
			NULL,
			2.0f,
			NULL,
			&audioStruct.voiceSends,
			NULL
		);
		R_ASSERT2(hr, "Can't create XAudio Source voice.");

		// create XAudio buffer
		XAUDIO2_BUFFER audioXBuffer = {};
		ZeroMemory(&audioXBuffer, sizeof(XAUDIO2_BUFFER));
		audioXBuffer.AudioBytes = dData.dwSize;
		audioXBuffer.pAudioData = dData.lpFile;
		audioXBuffer.Flags = XAUDIO2_END_OF_STREAM; 

		// if loop length bigger then 0 - set our pcm looplength
		if (dPCM.pLoopLength > NULL)
		{
			audioXBuffer.LoopLength = dPCM.pLoopLength;
			audioXBuffer.LoopBegin = dPCM.pLoopStart;
			audioXBuffer.LoopCount = 1;
		}

		// submit our buffer
		hr = audioStruct.lpXAudioSourceVoice->SubmitSourceBuffer(&audioXBuffer);
		R_ASSERT3(hr, "Can't submit buffer (buffer overflow");
		if (!SUCCEEDED(hr))
		{
			_RELEASE(audioStruct.lpXAudio);
		}
	}
	return audioStruct;
}

/*************************************************
* CreateAudioState():
* Create audio state with count of 
* played samples
*************************************************/
VOID
XAudioPlayer::CreateXAudioState(
	XAUDIO_DATA audioStruct
)
{
	HRESULT hr = NULL;

	if (audioStruct.lpXAudio)
	{
		// get start playing
		hr = audioStruct.lpXAudioSourceVoice->Start(NULL);
		R_ASSERT3(hr, "Can't start playing");

		/*************************************************
		* SamplesPlayed - is count of played samples.
		* In one second minimal count of samples is 44100.
		* To get position of our buffer, we must to
		* know about how much samples be played.
		*************************************************/
		BOOL isRunning = TRUE;
		while (SUCCEEDED(hr) && isRunning)
		{
			XAUDIO2_VOICE_STATE state;
			audioStruct.lpXAudioSourceVoice->GetState(&state);
			isRunning = (state.BuffersQueued > 0) != 0;

			// wait till the escape key is pressed
			if (GetAsyncKeyState(VK_ESCAPE))
				break;

			Sleep(10);
		}

		// wait till the escape key is released
		while (GetAsyncKeyState(VK_ESCAPE))
			Sleep(10);
	}

	_RELEASE(audioStruct.lpXAudio);
}

/*************************************************
* CreateXAudioThread:
* Method for create XAudio2 thread
*************************************************/
VOID
WINAPIV
CreateXAudioThread(
	AUDIO_FILE* xAudioFile
)
{
	XAudioPlayer xPlayer = {};
	XAUDIO_DATA xData = {};
	ZeroMemory(&xData, sizeof(XAUDIO_DATA));

	xData = xPlayer.CreateXAudioDevice(xAudioFile->dData, xAudioFile->dPCM);
	xPlayer.CreateXAudioState(xData);
}
