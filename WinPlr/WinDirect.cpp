/*********************************************************
* Copyright (C) VERTVER, 2018. All rights reserved.
* WinPlr - open-source WINAPI audio player.
* MIT-License
**********************************************************
* Module Name: WinAudio 2D Graphic
**********************************************************
* WinDirect.cpp
* Direct2D implementation
*********************************************************/

#include "WinAudio.h"

/************************************************
* DEPRECATED CODE:
************************************************/
VOID 
Player::DirectGraphic::SetDirectWindow(
	HWND hwnd,
	int x,
	int y
)
{
	WORD uIndex = NULL;

	if (!pRT)
	{
		// get window rect
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

		// if render target is success
		if (SUCCEEDED(pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, size),
			&pRT)))
		{
			const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0);

			if (SUCCEEDED(pRT->CreateSolidColorBrush(color, &pBrush)))
			{
				D2D1_SIZE_F size = pRT->GetSize();
				const float x = size.width / 2;
				const float y = size.height / 2;
				const float radius = min(x, y);
				// here code to drow
			}
		}
	}
}
