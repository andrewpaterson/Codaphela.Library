/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "StandardLib/Objects.h"
#include "Maps.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaps::Init(Ptr<CImageCelBlitterCache> pCache, Ptr<CImage> pViewport)
{
	ValidatePtr(pCache);
	ValidatePtr(pViewport);

	PreInit();

	maMaps.Init();
	msViewportPosition.Zero();
	mpCache = pCache;
	mpViewport = pViewport;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaps::Free(void)
{
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaps::Class(void)
{
	M_Embedded(maMaps);
	U_2Int32(msViewportPosition);
	M_Pointer(mpCache);
	M_Pointer(mpViewport);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMaps::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMaps::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaps::AddMap(Ptr<CBlockMap> pMap)
{
	maMaps.Add(pMap);
	pMap->SetViewport(mpViewport);
	pMap->SetBlitterCache(mpCache);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaps::SetViewportPosition(int32 x, int32 y)
{
	msViewportPosition.Init(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaps::SetViewportPosition(SInt32Vec2	sViewportPosition)
{
	msViewportPosition.Init(&sViewportPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMaps::Blit(void)
{
	size			uiNumElements;
	size			ui;
	Ptr<CBlockMap>	pMap;
	CRectangle		cRect;
	SInt32Vec2		sBottomRight;
	bool			bResult;

	sBottomRight.Init(mpViewport->GetWidth() + msViewportPosition.x, mpViewport->GetHeight() + msViewportPosition.y);
	cRect.Init(msViewportPosition, sBottomRight);
	uiNumElements = maMaps.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pMap = maMaps.Get(ui);
		bResult = pMap->Blit(&cRect);
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMaps::CreateCelBlitters(void)
{
	size			uiNumElements;
	size			ui;
	Ptr<CBlockMap>	pMap;
	bool			bResult;

	uiNumElements = maMaps.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pMap = maMaps.Get(ui);
		bResult = pMap->CreateCelBlitters();
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}

