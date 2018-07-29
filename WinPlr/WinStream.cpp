/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio mixer
**********************************************************
* WinStream.cpp
* Stream implimintation for WINAPI mixer method
*********************************************************/
#include "WinAudio.h"

HANDLE hAudioFile;
Player::ErrorHandler hStreamErr;

/*************************************************
* CreateStreamFromBuffer():
* Create stream from FILE_DATA struct info
*************************************************/
VOID
Player::Stream::CreateSimpleStreamFromBuffer(
	FILE_DATA dData,
	PCM_DATA dPCM,
	HWND hwnd
)
{
	HANDLE hWaveData;
	hAudioFile = dData.hFile;

	// create wave callback
	WAVEFORMATEX waveFormat = {};
	waveFormat.nSamplesPerSec = dPCM.dwSamplerate;
	waveFormat.wBitsPerSample = dPCM.wBits;
	waveFormat.nChannels = dPCM.wChannels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.cbSize = NULL;
	// check for current format
	switch (dData.eType)
	{
	case WAV_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
		break;
	case ALAC_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_ALAC;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case FLAC_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_FLAC;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case MPEG3_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case MPEG4_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_MPEG_ADTS_AAC;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case OGG_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_OGG_VORBIS_MODE_1;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case OPUS_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_OPUS;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case MPEG2_FILE:
	case AIF_FILE:
	case UNKNOWN_FILE:
	default:
		waveFormat.wFormatTag = WAVE_FORMAT_UNKNOWN;
		break;
	}
	
	// open default wave out
	MMRESULT uWave = waveOutOpen(
		NULL,
		WAVE_MAPPER,
		&waveFormat,
		NULL,
		NULL,
		CALLBACK_THREAD
	);
	switch (uWave)
	{
	case MMSYSERR_ALLOCATED:
		hStreamErr.CreateWarningText(
			"Warning! Current resource is already allocated"
		);
		break;
	case MMSYSERR_BADDEVICEID:
		hStreamErr.CreateErrorText(
			"Stream error! Bad device"
		);
		break;
	case MMSYSERR_NODRIVER:
		hStreamErr.CreateErrorText(
			"Stream error! No driver detected"
		);
		break;
	case MMSYSERR_NOMEM:
		hStreamErr.CreateErrorText(
			"Stream error! Unnable to allocate (memory error)"
		);
		break;
	case WAVERR_BADFORMAT:
		hStreamErr.CreateErrorText(
			"Stream error! Unsupported handle (!wave)"
		);
		break;
	case WAVERR_SYNC:
		__debugbreak();
	case MMSYSERR_NOERROR:
	default:
		break;
	}
}

/*************************************************
* CreateStreamFromBuffer():
* Create DirectSound stream by user data
*************************************************/
VOID 
Player::Stream::CreateDirectSoundStream(
	FILE_DATA dData,
	PCM_DATA dPCM,
	HWND hwnd
)
{
	STREAM_DATA dStream = {};
	DSBUFFERDESC bufferDesc = {};
	WAVEFORMATEX waveFormat = {};
	IDirectSoundBuffer* tempBuffer = {};
	dStream.bPlaying = FALSE;	// make true before init
	dStream.dPCM = dPCM;

	// initialize the direct sound interface 
	if (!SUCCEEDED(DirectSoundCreate(
		NULL,
		&dStream.lpDirectSound,
		NULL)))
	{
		hStreamErr.CreateErrorText("Failed to create sound device (DirectSound)");
	}

	// set the cooperative level
	HRESULT hr = dStream.lpDirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
	if (!SUCCEEDED(hr))
	{
		hStreamErr.CreateErrorText("Failed to set cooperative level (DirectSound)", hr);
	}

	// create device caps
	DSCAPS dsCaps;
	dsCaps.dwSize = sizeof(DSCAPS);

	// get device caps for direct sound pointer
	hr = dStream.lpDirectSound->GetCaps(&dsCaps);
	if (!SUCCEEDED(hr))
	{
		hStreamErr.CreateErrorText("Stream error! Can't get device caps (DirectSound)", hr);
	}

	// set parameters for primary buffer
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;
	
	// create primary sound buffer with buffer descriptor
	hr = dStream.lpDirectSound->CreateSoundBuffer(&bufferDesc,
		&dStream.lpPrimaryDirectBuffer,
		NULL
	);
	if (!SUCCEEDED(hr))
	{
		hStreamErr.CreateErrorText("Stream error! Can't create primary sound buffer (DirectSound)", hr);
	}

	waveFormat.nSamplesPerSec = dPCM.dwSamplerate;
	waveFormat.wBitsPerSample = dPCM.wBits;
	waveFormat.nChannels = dPCM.wChannels;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.cbSize = NULL;
	// check for current format
	switch (dData.eType)
	{
	case WAV_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_PCM; 
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
		break;
	case ALAC_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_ALAC;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case FLAC_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_FLAC;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case MPEG3_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case MPEG4_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_MPEG_ADTS_AAC;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case OGG_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_OGG_VORBIS_MODE_1;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case OPUS_FILE:
		waveFormat.wFormatTag = WAVE_FORMAT_OPUS;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	case MPEG2_FILE:
	case AIF_FILE:
	case UNKNOWN_FILE:
	default:
		waveFormat.wFormatTag = WAVE_FORMAT_UNKNOWN;
		waveFormat.nAvgBytesPerSec = (dPCM.wBitrate / 8);
		break;
	}

	// set the primary buffer to be the wave format specified.
	hr = dStream.lpPrimaryDirectBuffer->SetFormat(&waveFormat);
	if (!SUCCEEDED(hr))
	{
		hStreamErr.CreateErrorText("Stream error! Can't set wave format for sound buffer (DirectSound)", hr);
	}

	// set parameters for secondary buffer
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
	bufferDesc.dwBufferBytes = dData.dwSize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// create sound buffer
	hr = dStream.lpDirectSound->CreateSoundBuffer(
		&bufferDesc,
		&tempBuffer,
		NULL
	);
	if (!SUCCEEDED(hr))
	{
		hStreamErr.CreateErrorText("Stream error! Can't create temp sound buffer (DirectSound)", hr);
	}

	// create query interface
	hr = tempBuffer->QueryInterface(
		IID_IDirectSoundBuffer,
		(LPVOID*)&dStream.lpSecondaryDirectBuffer
	);
	if (!SUCCEEDED(hr))
	{
		hStreamErr.CreateErrorText("Stream error! Can't query sound interface (DirectSound)", hr);
	}

	// free temp buffer
	_RELEASE(tempBuffer);

	BYTE* pBuffer;
	BYTE* waveData = NULL;
	DWORD dwBufferSize;

	// lock buffer
	hr = dStream.lpSecondaryDirectBuffer->Lock(
		NULL,
		dData.dwSize,
		(void**)&pBuffer,
		(DWORD*)&dwBufferSize,
		NULL,
		NULL,
		NULL
	);
	if (!SUCCEEDED(hr))
	{
		hStreamErr.CreateErrorText("Stream error! Can't lock buffer (DirectSound)", hr);
	}

	memcpy(pBuffer, dPCM.lpData, dData.dwSize);

	// unlock buffer
	hr = dStream.lpSecondaryDirectBuffer->Unlock(
		(LPVOID)pBuffer,
		dwBufferSize,
		NULL,
		NULL
	);
	if (!SUCCEEDED(hr))
	{
		hStreamErr.CreateErrorText("Stream error! Can't unlock buffer (DirectSound)", hr);
	}

	// delete buffer
	delete[] waveData;
	waveData = NULL;

	// play sound
	hr = dStream.lpSecondaryDirectBuffer->SetCurrentPosition(NULL);
	hr = dStream.lpSecondaryDirectBuffer->SetVolume(DSBVOLUME_MAX);
	hr = dStream.lpSecondaryDirectBuffer->Play(NULL, NULL, NULL);
	if (!SUCCEEDED(hr))
	{
		hStreamErr.CreateErrorText("Stream error! Can't start playing", hr);
	}
	dStream.bPlaying = TRUE;
}
