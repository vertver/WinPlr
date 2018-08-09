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
	ZeroMemory(&audioStruct, sizeof(XAUDIO_DATA));
	ZeroMemory(&waveFormat, sizeof(WAVEFORMATEX));

	// set data to struct
	waveFormat.nSamplesPerSec = dPCM.dwSamplerate;
	waveFormat.wBitsPerSample = dPCM.wBitrate;
	waveFormat.nChannels = dPCM.wChannels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.cbSize = sizeof(WAVEFORMATEX);
	// check for current format
	switch (dData.eType)
	{
	case WAV_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
		break;
	case ALAC_FILE:
	case FLAC_FILE:
	case MPEG3_FILE:
	case MPEG4_FILE:
	case OGG_FILE:
	case OPUS_FILE:
	case MPEG2_FILE:
	case AIF_FILE:
	case UNKNOWN_FILE:
	default:
		waveFormat.wFormatTag = WAVE_FORMAT_UNKNOWN;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	}

	// use default XAudio processor
	hr = XAudio2Create(&audioStruct.lpXAudio);
	R_ASSERT2(hr, "Can't create XAudio device. Please, use DirectSound method.");

	// create default mastering voice
	hr = audioStruct.lpXAudio->CreateMasteringVoice(&audioStruct.lpXAudioMasterVoice);
	R_ASSERT2(hr, "Can't create XAudio Mastering voice.");

	XAUDIO2_SEND_DESCRIPTOR descAudio;
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
	XAUDIO2_BUFFER audioXBuffer;
	ZeroMemory(&audioXBuffer, sizeof(XAUDIO2_BUFFER));
	audioXBuffer.AudioBytes = XAUDIO2_MAX_BUFFER_BYTES;
	audioXBuffer.Flags = NULL;
	audioXBuffer.pAudioData = static_cast<BYTE*>(dPCM.lpData);

	// submit our buffer
	hr = audioStruct.lpXAudioSourceVoice->SubmitSourceBuffer(&audioXBuffer);
	R_ASSERT2(hr, "Can't submit buffer");

	// get start playing
	hr = audioStruct.lpXAudioSourceVoice->Start(NULL);
	hr = audioStruct.lpXAudioSourceVoice->SetVolume(XAudio2AmplitudeRatioToDecibels(0.2f));
	R_ASSERT2(hr, "Can't start playing");

	XAUDIO_DATA xaudioData;
	xaudioData.lpXAudio = audioStruct.lpXAudio;
	xaudioData.lpXAudioMasterVoice = audioStruct.lpXAudioMasterVoice;
	xaudioData.lpXAudioSourceVoice = audioStruct.lpXAudioSourceVoice;
	xaudioData.voiceState = audioStruct.voiceState;
	return xaudioData;
}

/*************************************************
* CreateAudioState():
* Create audio state with count of 
* played samples
*************************************************/
XAUDIO_DATA
XAudioPlayer::CreateXAudioState(XAUDIO_DATA audioStruct)
{
	HRESULT hr = NULL;
	audioStruct.lpXAudioSourceVoice->GetState(&audioStruct.voiceState);

	/*************************************************
	* SamplesPlayed - is count of played samples.
	* In one second minimal count of samples is 44100.
	* To get position of our buffer, we must to
	* know about how much samples be played.
	*************************************************/

	XAUDIO_DATA xaudioData;
	xaudioData.lpXAudio = audioStruct.lpXAudio;
	xaudioData.lpXAudioMasterVoice = audioStruct.lpXAudioMasterVoice;
	xaudioData.lpXAudioSourceVoice = audioStruct.lpXAudioSourceVoice;
	xaudioData.voiceState = audioStruct.voiceState;
	return xaudioData;
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
	XAudioPlayer xPlayer;
	XAUDIO_DATA xData;

	xData = xPlayer.CreateXAudioDevice(xAudioFile->dData, xAudioFile->dPCM);
	xPlayer.CreateXAudioState(xData);
}
