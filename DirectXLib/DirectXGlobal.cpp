/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela DirectXLib

Codaphela DirectXLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela DirectXLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela DirectXLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX and associated libraries are Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "DirectXGlobal.h"
#include "Direct3DGlobal.h"
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/ErrorTypes.h"


void InitDirectX(void)
{
	gcD3D.Init();
}


BOOL InitDirectX(int iAd, D3DDEVTYPE devType, HWND hWndScreen, int dx, int dy, D3DFORMAT fmt, BOOL bStencil, BOOL bExclusive, HWND hWndInput)
{
    ReturnOnFalse(gcD3D.Init(devType));
	ReturnOnFalse(gcD3D.InitDevice(iAd, hWndScreen, dx, dy, fmt, bStencil));
	return TRUE;
}


void KillDirectX(void)
{
	gcD3D.Kill();
}
