/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "Glyph.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGlyph::Init(CImageCel* pcCel, int iStep)
{
	mpcCel = pcCel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGlyph::Kill(void)
{
	mpcCel->Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGlyph::GetFullWidth(void)
{
	return mpcCel->GetSubImage()->GetFullWidth();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGlyph::GetFullHeight(void)
{
	return mpcCel->GetSubImage()->GetFullHeight();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CGlyph::GetFullDestRight(int x)
{
	return mpcCel->GetSubImage()->GetFullRight() + x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGlyph::GetFullDestBounds(int x, int y, CRectangle* pcRectangle)
{
	mpcCel->GetSubImage()->GetFullDestBounds(x, y, pcRectangle);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGlyph::GetImageDestBounds(int x, int y, CRectangle* pcRectangle)
{
	mpcCel->GetSubImage()->GetImageDestBounds(x, y, pcRectangle);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CImage* CGlyph::GetSourceImage(void) {return mpcCel->GetSourceImage();}
CSubImage* CGlyph::GetSubImage(void) {return mpcCel->GetSubImage();}
CImageCel* CGlyph::GetCel(void) {return mpcCel;}
int CGlyph::GetStep(void) {return miStep;}