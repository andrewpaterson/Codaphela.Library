/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "NativeWindowFactory.h"
#include "NativeCanvas.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeCanvas::Init(CCanvas* pcCanvas, CNativeWindowFactory* pcWindowFactory)
{
	CNativeComponent::Init(pcWindowFactory);
	mpcCanvas = pcCanvas;
	msSize.Init(-1, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeCanvas::Kill(void)
{
	mpcCanvas = NULL;
	CNativeComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Sets every channel to zero; black, and transparent if the canvas has an
// opacity channel.  Native canvases without an image must override this.
//
//////////////////////////////////////////////////////////////////////////
void CNativeCanvas::Clear(void)
{
	Ptr<CImage>	pImage;

	pImage = GetImageOrNull();
	if (pImage.IsNotNull())
	{
		pImage->Black();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CNativeCanvas::GetSize(void)
{
	return msSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeCanvas::SetSize(int x, int y)
{
	msSize.Init(x, y);
}

