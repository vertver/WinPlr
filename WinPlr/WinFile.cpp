/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio file-system
**********************************************************
* WinFile.cpp
* File-system for WinPlr
*********************************************************/
#include "WinAudio.h"

/*************************************************
* FindSoundChunk():
* Find current chunk and return header 
* if data is presented 
*************************************************/
const RIFFChunk* FindSoundChunk(
	_In_reads_bytes_(sizeBytes) const uint8_t* data,
	_In_ size_t sizeBytes,
	_In_ uint32_t tag)
{
	if (!data)
		return nullptr;

	const uint8_t* ptr = data;
	const uint8_t* end = data + sizeBytes;

	while (end > (ptr + sizeof(RIFFChunk)))
	{
		auto header = reinterpret_cast<const RIFFChunk*>(ptr);
		if (header->tag == tag)
			return header;

		ptrdiff_t offset = header->size + sizeof(RIFFChunk);
		ptr += offset;
	}

	return nullptr;
}

/*************************************************
* LoadFileToBuffer():
* Load data to structs and handles
*************************************************/
HANDLE_DATA
Player::Buffer::LoadFileToBuffer(
	HWND hwnd,
	FILE_DATA dFile,
	PCM_DATA dPCM
)
{
	// set zero for our structs
	ZeroMemory(&dFile, sizeof(FILE_DATA));
	ZeroMemory(&dPCM, sizeof(PCM_DATA));

	// set filedialog struct
	OPENFILENAMEA oFN;
	CHAR szName[MAX_PATH];
	szName[0] = '\0';		// needy for correct filedialog work

	// get params to our struct
	ZeroMemory(&oFN, sizeof(OPENFILENAMEA));
	oFN.lStructSize = sizeof(OPENFILENAMEA);
	oFN.hwndOwner = hwnd;
	oFN.nMaxFile = MAX_PATH;
	oFN.lpstrFile = szName;
	oFN.lpstrFilter = "Audio files (.wav)\0*.wav\0";
	oFN.lpstrTitle = "Open audio file";
	oFN.lpstrFileTitle = NULL;
	oFN.lpstrInitialDir = NULL;
	oFN.nFilterIndex = 1;
	oFN.nMaxFileTitle = 0;
	oFN.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// if we can't open filedialog - exit
	if (!GetOpenFileNameA(&oFN))
	{
		CreateInfoText("The file is empty.");
		ExitProcess(FALSE);
	}

	// create extended handle and copy file to it
	SCOPE_HANDLE hFile(CreateFileA(
		oFN.lpstrFile,
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	));

	DWORD dwSizeWritten = NULL;
	FILE_STANDARD_INFO fileInfo = {};

	// check for corrupted handle and check information from file
	ASSERT(hFile, "Filesystem error! Can't find file!");
	ASSERT(GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)), "FileInfo");

	// file is too big for 32-bit allocation, so reject read
	if (fileInfo.EndOfFile.HighPart > 0)
	{
		DEBUG_MESSAGE("File is too big");
	}

	// need at least enough data to have a valid minimal WAV file
	if (fileInfo.EndOfFile.LowPart < (sizeof(RIFFChunk) * 2 + sizeof(DWORD) + sizeof(WAVEFORMAT)))
	{
		DEBUG_MESSAGE("Lowpart file is invalid");
	}

	BYTE* lpWaveFile = (BYTE*)malloc(fileInfo.EndOfFile.LowPart);

	// reset our pointer and read data to it
	ASSERT(ReadFile(hFile.get(), lpWaveFile, fileInfo.EndOfFile.LowPart, &dwSizeWritten, NULL), "Can't read file");

	// check RIFF tag
	const RIFFChunk* riffChunk = FindSoundChunk(lpWaveFile, dwSizeWritten, FOURCC_RIFF_TAG);

	// if chunk is empty or size smaller than 4 - take message
	if (!riffChunk || riffChunk->size < 4)
	{
		DEBUG_MESSAGE("File is not a RIFF (riffChunk)");
	}

	// get RIFF chunk header info
	const uint8_t* wavEnd = lpWaveFile + dwSizeWritten;
	const RIFFChunkHeader* riffHeader = reinterpret_cast<const RIFFChunkHeader*>(riffChunk);

	// if this file isn't RIFF - take message
	if (riffHeader->riff != FOURCC_WAVE_FILE_TAG && riffHeader->riff != FOURCC_XWMA_FILE_TAG)
	{
		DEBUG_MESSAGE("File is not a RIFF (riffHeader)");
	}

	// locate 'fmt ' at file
	const uint8_t* ptr = reinterpret_cast<const uint8_t*>(riffHeader) + sizeof(RIFFChunkHeader);

	if ((ptr + sizeof(RIFFChunk)) > wavEnd)
	{
		DEBUG_MESSAGE("File is not a RIFF (ptr)");
	}

	// find fmt chunk
	const RIFFChunk* fmtChunk = FindSoundChunk(ptr, riffHeader->size, FOURCC_FORMAT_TAG);

	// if chunk is empty or size smaller than size of PCMWAVEFORMAT - take message
	if (!fmtChunk || fmtChunk->size < sizeof(PCMWAVEFORMAT))
	{
		DEBUG_MESSAGE("File is not a RIFF (riffHeader)");
	}

	// reinterpretate fmt chunk to pointer
	ptr = reinterpret_cast<const uint8_t*>(fmtChunk) + sizeof(RIFFChunk);

	if (ptr + fmtChunk->size > wavEnd)
	{
		DEBUG_MESSAGE("File is not a RIFF (riffHeader)");
	}

	////////////////////////////////////////////////////////

	BOOL isDPDS = FALSE;

	const WAVEFORMAT* wf = reinterpret_cast<const WAVEFORMAT*>(ptr);

	// check formatTag
	switch (wf->wFormatTag)
	{
	case WAVE_FORMAT_PCM:
	case WAVE_FORMAT_IEEE_FLOAT:
		// Can be a PCMWAVEFORMAT (8 bytes) or WAVEFORMATEX (10 bytes)
		// We validiated chunk as at least sizeof(PCMWAVEFORMAT) above
		break;
	default:
	{
		if (fmtChunk->size < sizeof(WAVEFORMATEX))
		{
			DEBUG_MESSAGE("File is not a RIFF (fmtChunk->size < sizeof(WAVEFORMATEX))");
		}
		auto wfx = reinterpret_cast<const WAVEFORMATEX*>(ptr);

		if (fmtChunk->size < (sizeof(WAVEFORMATEX) + wfx->cbSize))
		{
			DEBUG_MESSAGE("File is not a RIFF (fmtChunk->size < (sizeof(WAVEFORMATEX) + wfx->cbSize))");
		}
		switch (wfx->wFormatTag)
		{
		case WAVE_FORMAT_WMAUDIO2:
		case WAVE_FORMAT_WMAUDIO3:
			isDPDS = TRUE;
			break;
		case WAVE_FORMAT_ADPCM:
			if ((fmtChunk->size < (sizeof(WAVEFORMATEX) + 32)) || (wfx->cbSize < 32))
			{
				DEBUG_MESSAGE("File is not a RIFF (fmtChunk->size < (sizeof(WAVEFORMATEX) + 32)) || (wfx->cbSize < 32)");
			}
			break;

		case WAVE_FORMAT_EXTENSIBLE:
			if ((fmtChunk->size < sizeof(WAVEFORMATEXTENSIBLE)) ||
				(wfx->cbSize < (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))))
			{
				DEBUG_MESSAGE("File is not a RIFF (fmtChunk->size < sizeof(WAVEFORMATEXTENSIBLE)) || (wfx->cbSize < (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)))");
			}
			else
			{
				static const GUID s_wfexBase =
				{ 0x00000000, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 };

				auto wfex = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(ptr);

				DO_EXIT(!memcmp(
					reinterpret_cast<const BYTE*>(&wfex->SubFormat) +
					sizeof(DWORD),
					reinterpret_cast<const BYTE*>(&s_wfexBase) + sizeof(DWORD),
					sizeof(GUID) - sizeof(DWORD)
				),
					"Failed to memcpy operation");

				switch (wfex->SubFormat.Data1)
				{
				case WAVE_FORMAT_PCM:
				case WAVE_FORMAT_IEEE_FLOAT:
					break;
				case WAVE_FORMAT_WMAUDIO2:
				case WAVE_FORMAT_WMAUDIO3:
					isDPDS = TRUE;
					break;

				default:
					DEBUG_MESSAGE("File is not a RIFF (default)");
				}
			}
			break;

		default:
			DEBUG_MESSAGE("File is not a RIFF (default)");
		}
	}
	}

	// reinterpretate RIFF header to pointer
	ptr = reinterpret_cast<const uint8_t*>(riffHeader) + sizeof(RIFFChunkHeader);
	ASSERT(!((ptr + sizeof(RIFFChunk)) > wavEnd), "RIFFChunk error");

	// find 'data' chunk
	const RIFFChunk* dataChunk = FindSoundChunk(ptr, riffChunk->size, FOURCC_DATA_TAG);
	ASSERT(!(!dataChunk || !dataChunk->size), "No data chunk or chunk size");

	// reinterpretate 'data' header to pointer
	ptr = reinterpret_cast<const uint8_t*>(dataChunk) + sizeof(RIFFChunk);
	ASSERT(!(ptr + dataChunk->size > wavEnd), "size > wavEnd");

	uint32_t pLoopStart = NULL;
	uint32_t pLoopLength = NULL;

	const RIFFChunk* dlsChunk = FindSoundChunk(ptr, riffChunk->size, FOURCC_DLS_SAMPLE);
	if (dlsChunk)
	{
		ptr = reinterpret_cast<const uint8_t*>(dlsChunk) + sizeof(RIFFChunk);
		ASSERT(!(ptr + dlsChunk->size > wavEnd), "(ptr + dlsChunk->size > wavEnd)");

		if (dlsChunk->size >= sizeof(RIFFDLSSample))
		{
			const RIFFDLSSample* dlsSample = reinterpret_cast<const RIFFDLSSample*>(ptr);

			if (dlsChunk->size >= (dlsSample->size + dlsSample->loopCount * sizeof(DLSLoop)))
			{
				const DLSLoop* loops = reinterpret_cast<const DLSLoop*>(ptr + dlsSample->size);
				for (uint32_t j = 0; j < dlsSample->loopCount; ++j)
				{
					if ((loops[j].loopType == DLSLoop::LOOP_TYPE_FORWARD || loops[j].loopType == DLSLoop::LOOP_TYPE_RELEASE))
					{
						// Return 'forward' loop
						pLoopStart = loops[j].loopStart;
						pLoopLength = loops[j].loopLength;
					}
				}
			}
		} 
	}

	// Locate 'smpl' (Sample Chunk)
	const RIFFChunk* midiChunk = FindSoundChunk(ptr, riffChunk->size, FOURCC_MIDI_SAMPLE);
	if (midiChunk)
	{
		ptr = reinterpret_cast<const uint8_t*>(midiChunk) + sizeof(RIFFChunk);
		ASSERT(!(ptr + midiChunk->size > wavEnd), "(ptr + midiChunk->size > wavEnd)");

		if (midiChunk->size >= sizeof(RIFFMIDISample))
		{
			auto midiSample = reinterpret_cast<const RIFFMIDISample*>(ptr);

			if (midiChunk->size >= (sizeof(RIFFMIDISample) + midiSample->loopCount * sizeof(MIDILoop)))
			{
				const MIDILoop* loops = reinterpret_cast<const MIDILoop*>(ptr + sizeof(RIFFMIDISample));
				for (uint32_t j = 0; j < midiSample->loopCount; ++j)
				{
					if (loops[j].type == MIDILoop::LOOP_TYPE_FORWARD)
					{
						// Return 'forward' loop
						pLoopStart = loops[j].start;
						pLoopLength = loops[j].end + loops[j].start + 1;
					}
				}
			}
		}
	}

	// get params to our structs
	dFile.dwSize = dwSizeWritten;
	dFile.eType = WAV_FILE;
	dFile.lpFile = lpWaveFile;

	// reinterpretate WAVEFORMAT to WAVEFORMATEX
	const WAVEFORMATEX* wfexA = reinterpret_cast<const WAVEFORMATEX*>(wf);

	dPCM.waveFormat.cbSize = sizeof(WAVEFORMATEX);
	dPCM.waveFormat.nAvgBytesPerSec = wfexA->nAvgBytesPerSec;
	dPCM.waveFormat.nBlockAlign = wfexA->nBlockAlign;
	dPCM.waveFormat.nChannels = wfexA->nChannels;
	dPCM.waveFormat.nSamplesPerSec = wfexA->nSamplesPerSec;
	dPCM.waveFormat.wBitsPerSample = wfexA->wBitsPerSample;
	dPCM.waveFormat.wFormatTag = wfexA->wFormatTag;
	dPCM.pLoopLength = pLoopLength;
	dPCM.pLoopStart = pLoopStart;
	dPCM.lpData = lpWaveFile;
	dPCM.lpPath = oFN.lpstrFile;

	HANDLE_DATA hdReturn = { };
	ZeroMemory(&hdReturn, sizeof(HANDLE_DATA));
	hdReturn.dData = dFile;
	hdReturn.dPCM = dPCM;
	hdReturn.hwnd = hwnd;
	return hdReturn;
}

/*************************************************
* CheckBufferFile():
* Checks buffer on validate
*************************************************/
BOOL
Player::Buffer::CheckBufferFile(
	HANDLE_DATA hdData
)
{
	if (!hdData.hwnd && !hdData.dData.lpFile)
		return FALSE;
	else
		return TRUE;
}
