/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio header	
**********************************************************
* WinAudio.h
* Master include file for WinPlr application.
*********************************************************/
#pragma once

#include <windows.h>
#include <string>
#include <dsound.h>			// for DirectSound
#include <mmreg.h>
#include <mmeapi.h>
#include <VersionHelpers.h>
#include <mmdeviceapi.h>	// minimal MME API
#include <ks.h>
#include <ksmedia.h>
#include <comdef.h>			// for hresult
#include <cguid.h>			// for UI
#include <strsafe.h>
#include <d2d1_3.h>
#include <dxgi.h>
#include <xaudio2.h>
#include <stdio.h>
#include <process.h>

#define DLL_EXPORTS
#ifdef DLL_EXPORTS
#define PLAYER_API decltype(dllexport)
#else
#define PLAYER_API decltype(dllimport)
#endif

struct handle_closer { void operator()(HANDLE h) { if (h) CloseHandle(h); } };
using SCOPE_HANDLE = std::unique_ptr<void, handle_closer>;

LPCWSTR GetUnicodeStringFromAnsi(_In_ LPCSTR lpString);
VOID CreateErrorText(_In_ LPCSTR lpMsgText);
VOID CreateErrorText(_In_ LPCSTR lpMsgText, _In_ HRESULT hr);
VOID CreateInfoText(_In_ LPCSTR lpMsgText);
VOID CreateWarningText(_In_ LPCSTR lpMsgText);
VOID ContinueIfYes(_In_ LPCSTR lpMsgText, _In_ LPCSTR lpMsgTitle);

#define DEBUG_MESSAGE(x)	OutputDebugStringA(x); OutputDebugStringA("\n");
#define _RELEASE(x)			if (x)					{ x->Release(); x = NULL; }	// safety release pointers
#define R_ASSERT2(x, y)		if (!SUCCEEDED(x))		{ CreateErrorText(y, x); }
#define R_ASSERT(x)			if (!SUCCEEDED(x))		{ CreateErrorText("R_ASSERT"); }
#ifndef DEBUG
#define ASSERT(x, y)		if (!x)					{ DEBUG_MESSAGE(y); }
#define R_ASSERT3(x, y)		if (!SUCCEEDED(x))		{ DEBUG_MESSAGE(y); }
#else
#define ASSERT(x, y)		if (!x)					{ DEBUG_MESSAGE(y); __debugbreak(); }
#define R_ASSERT3(x, y)		if (!SUCCEEDED(x))		{ DEBUG_MESSAGE(y); __debugbreak(); }
#endif
#define DO_EXIT(x, y)		if (!(x))				{ CreateErrorText(y); }
#define PLAYER_VERSION		"#PLAYER_VERSION: 0.2.2#"

typedef enum
{
	WAV_FILE = 1,
	AIF_FILE = 2,
	ALAC_FILE = 3,
	FLAC_FILE = 4,
	MPEG4_FILE = 5,
	MPEG3_FILE = 6,
	MPEG2_FILE = 7,
	OGG_FILE = 8,
	OPUS_FILE = 9,
	UNKNOWN_FILE = 10
} FILE_TYPE;

typedef enum
{
	HANN_WINDOW = 1,
	HAMMING_WINDOW = 2,
	BLACKMAN_WINDOW = 3,
	BLACKMANHARRIS_WINDOW = 4
} WINDOW_TYPE;

typedef struct {
	DWORD	dwType;				// thread type
	LPCSTR	lpName;				// thread name
	DWORD	dwThreadID;			// id for thread
	DWORD	dwFlags;			// flags to create thread
} THREAD_NAME; 

typedef struct
{
	WAVEFORMATEX waveFormat;		// wave format info
	BYTE* lpData;					// pointer to allocated memory
	LPCSTR lpPath;					// full path to file
	uint32_t pLoopStart;			// start loop
	uint32_t pLoopLength;			// length of loop
} PCM_DATA, *PCM_DATA_P;

typedef struct
{
	HANDLE hFile;				// handle of file
	BYTE* lpFile;				// pointer to allocated memory
	DWORD dwSize;				// size of file
	FILE_TYPE eType;			// type of file
} FILE_DATA, *FILE_DATA_P;

typedef struct  
{
	FILE_DATA dData;			// FILE structure data
	PCM_DATA dPCM;				// PCM structure data
} HANDLE_DATA, *HANDLE_DATA_P;

typedef struct
{
	PCM_DATA dPCM;										// all PCM data for DirectSound
	LPDIRECTSOUND lpDirectSound;						// DirectSound main object
	LPDIRECTSOUNDBUFFER lpPrimaryDirectBuffer;			// DirectSound buffer
	LPDIRECTSOUNDBUFFER lpSecondaryDirectBuffer;		// DirectSound buffer
	LPDIRECTSOUNDNOTIFY lpDirectNotify;					// DirectSound notify
	BOOL bPlaying;										// display if audio now is playing
} STREAM_DATA, *STREAM_DATA_P;

typedef struct
{
	FILE_DATA dData;			// file data struct
	PCM_DATA dPCM;				// PCM data struct
} AUDIO_FILE, *AUDIO_FILE_P;

typedef struct 
{
	uint32_t tag;				// tag of RIFF chunk
	uint32_t size;				// chunk size
} RIFFChunk;

typedef struct
{
	uint32_t tag;				// tag of RIFF chunk header
	uint32_t size;				// chunk header size
	uint32_t riff;				// RIFF info
} RIFFChunkHeader;

typedef struct 
{
	static const uint32_t LOOP_TYPE_FORWARD = 0x00000000;
	static const uint32_t LOOP_TYPE_RELEASE = 0x00000001;

	uint32_t size;				// chunk size
	uint32_t loopType;			// chunk loop type
	uint32_t loopStart;			// chunk loop start
	uint32_t loopLength;		// chunk length
} DLSLoop;

typedef struct
{
	static const uint32_t OPTIONS_NOTRUNCATION = 0x00000001;
	static const uint32_t OPTIONS_NOCOMPRESSION = 0x00000002;

	uint32_t    size;
	uint16_t    unityNote;
	int16_t     fineTune;
	int32_t     gain;
	uint32_t    options;
	uint32_t    loopCount;
} RIFFDLSSample;

typedef struct 
{
	static const uint32_t LOOP_TYPE_FORWARD = 0x00000000;
	static const uint32_t LOOP_TYPE_ALTERNATING = 0x00000001;
	static const uint32_t LOOP_TYPE_BACKWARD = 0x00000002;

	uint32_t cuePointId;
	uint32_t type;
	uint32_t start;
	uint32_t end;
	uint32_t fraction;
	uint32_t playCount;
} MIDILoop;

typedef struct 
{
	uint32_t        manufacturerId;
	uint32_t        productId;
	uint32_t        samplePeriod;
	uint32_t        unityNode;
	uint32_t        pitchFraction;
	uint32_t        SMPTEFormat;
	uint32_t        SMPTEOffset;
	uint32_t        loopCount;
	uint32_t        samplerData;
} RIFFMIDISample;

static_assert(sizeof(RIFFChunk) == 8, "structure size mismatch");
static_assert(sizeof(RIFFChunkHeader) == 12, "structure size mismatch");
static_assert(sizeof(DLSLoop) == 16, "structure size mismatch");
static_assert(sizeof(RIFFDLSSample) == 20, "structure size mismatch");
static_assert(sizeof(MIDILoop) == 24, "structure size mismatch");
static_assert(sizeof(RIFFMIDISample) == 36, "structure size mismatch");

const uint32_t FOURCC_RIFF_TAG		= MAKEFOURCC('R', 'I', 'F', 'F');
const uint32_t FOURCC_FORMAT_TAG	= MAKEFOURCC('f', 'm', 't', ' ');
const uint32_t FOURCC_DATA_TAG		= MAKEFOURCC('d', 'a', 't', 'a');
const uint32_t FOURCC_WAVE_FILE_TAG = MAKEFOURCC('W', 'A', 'V', 'E');
const uint32_t FOURCC_XWMA_FILE_TAG = MAKEFOURCC('X', 'W', 'M', 'A');
const uint32_t FOURCC_DLS_SAMPLE	= MAKEFOURCC('w', 's', 'm', 'p');
const uint32_t FOURCC_MIDI_SAMPLE	= MAKEFOURCC('s', 'm', 'p', 'l');
const uint32_t FOURCC_XWMA_DPDS		= MAKEFOURCC('d', 'p', 'd', 's');
const uint32_t FOURCC_XMA_SEEK		= MAKEFOURCC('s', 'e', 'e', 'k');

namespace Player
{
	class Buffer
	{
	public:
		Buffer();
		~Buffer();
		HANDLE_DATA LoadFileToBuffer(_In_ FILE_DATA dFile, _In_ PCM_DATA dPCM);
		BOOL CheckBufferFile(_In_ HANDLE_DATA hdData);

	private:
		HANDLE hHeap;
	};
	class Stream
	{
	public:
		STREAM_DATA CreateMMIOStream(_In_ FILE_DATA dData, _In_ PCM_DATA dPCM, _In_ HWND hwnd);
		STREAM_DATA CreateDirectSoundStream(_In_ FILE_DATA dData, _In_ PCM_DATA dPCM, _In_ HWND hwnd);
		VOID PlayBufferSound(_In_ STREAM_DATA streamData);
		VOID StopBufferSound(_In_ STREAM_DATA streamData);
		VOID ReleaseSoundBuffers(_In_ STREAM_DATA streamData);
	};
	class ThreadSystem
	{
	public:
		VOID ThSetNewThreadName(_In_ LPCSTR lpName);
	};
	class Graphics
	{
	public:
	};
}
