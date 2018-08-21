/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio mixer
**********************************************************
* WinAudio.cpp
* Stream implimintation for WINAPI mixer method
*********************************************************/
#include "WinAudio.h"

HANDLE hAudioFile;
#define NOTIFIATINS_POSES 2

/*************************************************
* CreateStreamFromBuffer():
* Create stream from FILE_DATA
* and PCM_DATA struct
*************************************************/
STREAM_DATA
Player::Stream::CreateMMIOStream(
	_In_ FILE_DATA dData,
	_In_ PCM_DATA dPCM,
	_In_ HWND hwnd
)
{
	STREAM_DATA streamData;
	ZeroMemory(&streamData, sizeof(STREAM_DATA));
	streamData.dPCM = dPCM;

	// create wave callback
	WAVEFORMATEX waveFormat = {};
	ZeroMemory(&waveFormat, sizeof(WAVEFORMATEX));
	waveFormat.cbSize = sizeof(WAVEFORMATEX);
	waveFormat.nAvgBytesPerSec = dPCM.waveFormat.nAvgBytesPerSec;
	waveFormat.nBlockAlign = dPCM.waveFormat.nBlockAlign;
	waveFormat.nChannels = dPCM.waveFormat.nChannels;
	waveFormat.nSamplesPerSec = dPCM.waveFormat.nSamplesPerSec;
	waveFormat.wBitsPerSample = dPCM.waveFormat.wBitsPerSample;
	waveFormat.wFormatTag = dPCM.waveFormat.wFormatTag;

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
	case MMSYSERR_ALLOCATED:	CreateWarningText	("Warning! Current resource is already allocated");		break;
	case MMSYSERR_BADDEVICEID:	CreateErrorText		("Stream error! Bad device");							break;
	case MMSYSERR_NODRIVER:		CreateErrorText		("Stream error! No driver detected");					break;
	case MMSYSERR_NOMEM:		CreateErrorText		("Stream error! Unnable to allocate (memory error)");	break;
	case WAVERR_BADFORMAT:		CreateErrorText		("Stream error! Unsupported handle (!wave)");			break;
	case WAVERR_SYNC:			__debugbreak();
	case MMSYSERR_NOERROR:		default:			break;
	}

	streamData.bPlaying = TRUE;
	streamData.lpDirectNotify = NULL;
	streamData.lpDirectSound = NULL;
	streamData.lpPrimaryDirectBuffer = NULL;
	streamData.lpSecondaryDirectBuffer = NULL;
	return streamData;
}

/*************************************************
* CreateStreamFromBuffer():
* Create DirectSound stream from user data
*************************************************/
STREAM_DATA
Player::Stream::CreateDirectSoundStream(
	_In_ FILE_DATA dData,
	_In_ PCM_DATA dPCM,
	_In_ HWND hwnd
)
{
	STREAM_DATA streamData = {};
	DSBUFFERDESC bufferDesc = {};
	WAVEFORMATEX waveFormat = {};
	HRESULT hr = NULL;
	LPDIRECTSOUNDBUFFER tempBuffer = {};
	streamData.bPlaying = FALSE;	// make true before init
	streamData.dPCM = dPCM;

	ZeroMemory(&streamData, sizeof(STREAM_DATA));
	ZeroMemory(&bufferDesc, sizeof(DSBUFFERDESC));
	ZeroMemory(&waveFormat, sizeof(WAVEFORMATEX));
	ZeroMemory(&tempBuffer, sizeof(LPDIRECTSOUNDBUFFER));

	if (dData.dwSize)
	{
		// initialize the direct sound interface 
		if (!SUCCEEDED(DirectSoundCreate(
			NULL,
			&streamData.lpDirectSound,
			NULL)))
		{
			CreateErrorText(
				"Failed to create sound device (DirectSound). Check your audio drivers\nor restart with '-no_direct_sound' argument'."
			);
		}

		// set the cooperative level
		hr = streamData.lpDirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
		R_ASSERT2(hr, "Failed to set cooperative level (DirectSound)");

		{
			// create device caps
			DSCAPS dsCaps;
			ZeroMemory(&dsCaps, sizeof(DSCAPS));
			dsCaps.dwSize = sizeof(DSCAPS);

			// get device caps for direct sound pointer
			hr = streamData.lpDirectSound->GetCaps(&dsCaps);
			R_ASSERT2(hr, "Stream error! Can't get device caps (DirectSound)");
		}

		// set parameters for primary buffer
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
		bufferDesc.guid3DAlgorithm = GUID_NULL;

		// create primary sound buffer with buffer descriptor
		hr = streamData.lpDirectSound->CreateSoundBuffer(
			&bufferDesc,
			&streamData.lpPrimaryDirectBuffer,
			NULL
		);
		R_ASSERT2(hr, "Stream error! Can't create primary sound buffer (DirectSound)");
		waveFormat.nAvgBytesPerSec = dPCM.waveFormat.nAvgBytesPerSec;
		waveFormat.nBlockAlign = dPCM.waveFormat.nBlockAlign;
		waveFormat.nChannels = dPCM.waveFormat.nChannels;
		waveFormat.nSamplesPerSec = dPCM.waveFormat.nSamplesPerSec;
		waveFormat.wBitsPerSample = dPCM.waveFormat.wBitsPerSample;
		waveFormat.wFormatTag = dPCM.waveFormat.wFormatTag;

		// set the primary buffer to be the wave format specified.
		hr = streamData.lpPrimaryDirectBuffer->SetFormat(&waveFormat);
		R_ASSERT2(hr, "Stream error! Can't set wave format for sound buffer (DirectSound)");

		// set parameters for secondary buffer
		bufferDesc.dwSize = sizeof(DSBUFFERDESC);
		bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
		bufferDesc.dwBufferBytes = dData.dwSize;
		bufferDesc.lpwfxFormat = &waveFormat;
		bufferDesc.guid3DAlgorithm = GUID_NULL;

		// create sound buffer
		hr = streamData.lpDirectSound->CreateSoundBuffer(
			&bufferDesc,
			&tempBuffer,
			NULL
		);
		R_ASSERT2(hr, "Stream error! Can't create temp sound buffer (DirectSound)");

		// create query interface
		hr = tempBuffer->QueryInterface(
			IID_IDirectSoundBuffer,
			(LPVOID*)&streamData.lpSecondaryDirectBuffer
		);
		R_ASSERT2(hr, "Stream error! Can't query sound interface (DirectSound)");

		// free temp buffer
		_RELEASE(tempBuffer);

		LPVOID pBuffer = NULL;
		DWORD dwBufferSize = NULL;

		// lock buffer
		hr = streamData.lpSecondaryDirectBuffer->Lock(
			NULL,
			dData.dwSize,
			&pBuffer,
			(LPDWORD)&dwBufferSize,
			NULL,
			NULL,
			NULL
		);
		R_ASSERT2(hr, "Stream error! Can't lock buffer (DirectSound)");

		memcpy(pBuffer, dPCM.lpData, dData.dwSize);

		// unlock buffer
		hr = streamData.lpSecondaryDirectBuffer->Unlock(
			pBuffer,
			dwBufferSize,
			NULL,
			NULL
		);
		R_ASSERT2(hr, "Stream error! Can't unlock buffer (DirectSound)");		
	}
	return streamData;
}

/*************************************************
* PlayBufferSound():
* Play sound from buffer 
*************************************************/
VOID
Player::Stream::PlayBufferSound(
	_In_ STREAM_DATA streamData
)
{
	HRESULT hr = NULL;

	// if secondary buffer is not empty - use DirectSound
	if (streamData.lpSecondaryDirectBuffer)
	{
		// play sound
		hr = streamData.lpSecondaryDirectBuffer->SetCurrentPosition(NULL);
		hr = streamData.lpSecondaryDirectBuffer->SetVolume(-2000);
		hr = streamData.lpSecondaryDirectBuffer->Play(NULL, NULL, NULL);
		R_ASSERT3(hr, "Stream error! Can't start playing");
		streamData.bPlaying = TRUE;
	}
}

/*************************************************
* StopBufferSound():
* Stop playing buffer
*************************************************/
VOID 
Player::Stream::StopBufferSound(
	_In_ STREAM_DATA streamData
)
{
	HRESULT hr = NULL;

	// if secondary buffer is not empty - try to stop
	if (streamData.lpSecondaryDirectBuffer)
	{
		hr = SUCCEEDED(streamData.lpSecondaryDirectBuffer->Stop());
		R_ASSERT3(hr, "Stream error! Can't stop playing");
	}
	streamData.bPlaying = FALSE;
}

/*************************************************
* ReleaseSoundBuffers():
* Release all DirectSound pointers
*************************************************/
VOID
Player::Stream::ReleaseSoundBuffers(
	_In_ STREAM_DATA streamData
)
{
	_RELEASE(streamData.lpDirectNotify);
	_RELEASE(streamData.lpDirectSound);
	_RELEASE(streamData.lpPrimaryDirectBuffer);
	_RELEASE(streamData.lpSecondaryDirectBuffer);
}
