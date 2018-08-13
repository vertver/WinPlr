/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio file-reader
**********************************************************
* WinFileReader.cpp
* File type checking  
*********************************************************/

#include "WinFileReader.h"
FILE* fileData;
PCM_DATA pcmData;

/***********************************************
* freadNum():
* Checking for bad number
***********************************************/
template
<typename T> T 
freadNum(
	FILE* f
)
{
	T value;
	DO_EXIT(fread(&value, sizeof(value), 1, f) == 1, "Can't read file");
	return value;	// no endian-swap for now... WAV is LE anyway...
}

/***********************************************
* freadStr():
* Checking for bad string
***********************************************/
std::string 
freadStr(
	FILE* f,
	size_t len
)
{
	std::string s(len, '\0');
	DO_EXIT(fread(&s[0], 1, len, f) == len, "Can't read file");
	return s;
}

/***********************************************
* ReadFmtChunk():
* Reading FMT chunk with information
***********************************************/
PCM_DATA
ReadFmtChunk(
	UINT uChunkLen
)
{
	WORD dwSampleBits = NULL;
	DO_EXIT(uChunkLen >= 16, "Bad file Error: no chunk length");

	WORD fmttag = freadNum<WORD>(fileData);
	DO_EXIT(fmttag == 1 || fmttag == 3, "Bad file Error: No wave type");	// IEEE or PCM

	// check for FMT tag
	switch (fmttag)
	{
	case 1:
		pcmData.isFloat = FALSE;
	case 3:
		pcmData.isFloat = TRUE;
	default:
		break;
	}

	// check channels
	WORD wChannels = freadNum<WORD>(fileData);
	DO_EXIT(wChannels > 0, "Bad file Error: No channels");
	pcmData.wChannels = wChannels;

	// check sample rate
	UINT dwSampleRate = freadNum<UINT>(fileData);
	DO_EXIT(dwSampleRate > 0, "Bad file Error: No sample rate");
	pcmData.dwSamplerate = dwSampleRate;

	// byte and bitrate
	UINT dwByteRate = freadNum<UINT>(fileData);
	WORD wBlockAlign = freadNum<WORD>(fileData);

	// count of bits and bytes
	WORD wBits = freadNum<WORD>(fileData);
	WORD wBytes = wBits / 8;

	// compare all data
	DO_EXIT(dwByteRate == dwSampleRate * wChannels * wBytes, "Bad file Error: Error types");
	DO_EXIT(wBlockAlign == wChannels * wBytes, "Bad file Error: Error block align");

	// 1: PCM  - int
	// 3: IEEE - float
	if (fmttag == 1)
	{
		switch (wBits)
		{
		case 8:	 dwSampleBits = 8;  break;
		case 16: dwSampleBits = 16; break;
		case 24: dwSampleBits = 24; break;
		case 32: dwSampleBits = 32; break;
		}
	}
	else
	{
		DO_EXIT(fmttag == 3, "Bad file Error: No type");
		DO_EXIT(wBits == 32, "Bad file Error: No type");
	}
	if (uChunkLen > 16)
	{
		// compare with extented size
		WORD dwExtentedSize = freadNum<WORD>(fileData);
		DO_EXIT(uChunkLen == 18 + dwExtentedSize, "Bad file Error: No chunk length");
		fseek(fileData, dwExtentedSize, SEEK_CUR);
	}

	// returnt this struct
	pcmData.wBitrate = dwSampleBits; 
	return pcmData;
}

/***********************************************
* GetFullFileType():
* Return current type of file
***********************************************/
FILE_TYPE 
FileReader::GetFullFileType(
	LPCSTR lpString
)
{
	// open file with "read-only" rools
	fileData = fopen(lpString, "r");
	DO_EXIT(fileData, "Can't open file (The path is wrong or file handle is busy)");

	// if file is WAVE PCM - return
	if (freadStr(fileData, 4) == "RIFF")
	{
		UINT wavechunksize = freadNum<UINT>(fileData);
		DO_EXIT((freadStr(fileData, 4) == "WAVE"), "No wave");
		return WAV_FILE;
	}
	return UNKNOWN_FILE;
}

/***********************************************
* ReadChunks():
* Checking valid chunks
***********************************************/
VOID
FileReader::ReadChunks()
{
	while (TRUE)
	{
		// set chunk name and chunk length
		std::string chunkName = " ";
		chunkName = freadStr(fileData, 4);
		UINT chunkLen = freadNum<UINT>(fileData);

		// if chunk include FMT data - read this
		if (chunkName == "fmt ")
		{
			ReadFmtChunk(chunkLen);
		}
		else if (chunkName == "data") break;	// else break cycle
		else
		{
			ASSERT((fseek(fileData, chunkLen, SEEK_CUR) == NULL), "Can't seek file");		// skip chunk
		}
	}
}

/***********************************************
* GetFullFileInfo():
* return full info about audiofile
***********************************************/
PCM_DATA 
FileReader::GetFullFileInfo(
	LPCSTR lpPath
)
{
	FILE_TYPE eType = {};
	ZeroMemory(&eType, sizeof(FILE_TYPE));

	// Getting full information about file
	eType = GetFullFileType(lpPath);

	// if file include all RIFF information - read it chunks
	if (eType == WAV_FILE)
	{
		ReadChunks();
		free(fileData);
		return pcmData;
	}

	DO_EXIT(eType, "Can't check format");
	return pcmData;
}
