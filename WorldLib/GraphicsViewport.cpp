/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "GraphicsViewport.h"
#include "World.h"
#include "DirectXLib/Direct3DGlobal.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewportInstance::Init(void)
{
	mpcWorld = NULL;
	msViewport.Zero();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewportInstance::Init(CWorld* pcWorld)
{
	mpcWorld = pcWorld;
	gcD3D.CreateViewport(&msViewport);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewportInstance::Kill(void)
{
	gcD3D.ReleaseViewport(&msViewport);
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewportInstance::Draw(void)
{
	gcD3D.SetViewport(&msViewport);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewportInstance::Invalidate(void)
{
	//Invalidates the current one rather than this one.  Meh.
	gcD3D.InvalidateViewport();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CViewportInstance::Set(int iX, int iY, int iWidth, int iHeight)
{
	msViewport.sD3DViewport.X = iX;
	msViewport.sD3DViewport.Y = iY;
	msViewport.sD3DViewport.Width = iWidth;
	msViewport.sD3DViewport.Height = iHeight;
	Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CViewportInstance::Contains(int iLeft, int iTop, int iRight, int iBottom)
{
	if ((int)msViewport.sD3DViewport.X >= iRight)
	{
		return FALSE;
	}
	if ((int)msViewport.sD3DViewport.X + (int)msViewport.sD3DViewport.Width < iLeft)
	{
		return FALSE;
	}
	if ((int)msViewport.sD3DViewport.Y >= iBottom)
	{
		return FALSE;
	}
	if ((int)msViewport.sD3DViewport.Y + (int)msViewport.sD3DViewport.Height < iTop)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CViewportInstance::Contains(CRectangle* pcRect)
{
	return Contains(pcRect->miLeft, pcRect->miTop, pcRect->miRight, pcRect->miBottom);
}

