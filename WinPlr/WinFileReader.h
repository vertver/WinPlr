/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio file-reader header
**********************************************************
* WinFileReader.h
* File type checking
*********************************************************/
#pragma once
#include "WinAudio.h"

class FileReader
{
public:
	FILE_TYPE GetFullFileType(LPCSTR lpString);
	PCM_DATA GetFullFileInfo(LPCSTR lpPath);
	VOID ReadChunks();
};

