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
#include "TileMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::Init(void)
{
	PreInit();

	CBlockMap::Init();
	maTileLayers.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::Free(void)
{
	CBlockMap::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::Class(void)
{
	CBlockMap::Class();
	M_Embedded(maTileLayers);
}	


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileMap::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTileMap::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::AddLayer(Ptr<CTileLayer> pLayer)
{
	maTileLayers.Add(pLayer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::SetBlitterCache(Ptr<CImageCelBlitterCache> pCache)
{
	size				uiNumElements;
	size				i;
	Ptr<CTileLayer>		pTileLayer;

	uiNumElements = maTileLayers.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pTileLayer = maTileLayers.Get(i);
		pTileLayer->SetBlitterCache(pCache);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTileMap::SetViewport(Ptr<CImage> pViewport)
{
	size				uiNumElements;
	size				i;
	Ptr<CTileLayer>		pTileLayer;

	uiNumElements = maTileLayers.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pTileLayer = maTileLayers.Get(i);
		pTileLayer->SetViewport(pViewport);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMap::Blit(CRectangle* pcViewportRect, bool bEnsureBlitters)
{
	size				uiNumElements;
	size				i;
	Ptr<CTileLayer>		pTileLayer;
	bool				bResult;

	uiNumElements = maTileLayers.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pTileLayer = maTileLayers.Get(i);
		bResult = pTileLayer->Blit(pcViewportRect, bEnsureBlitters);
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
bool CTileMap::CreateCelBlitters(void)
{
	size				uiNumElements;
	size				i;
	Ptr<CTileLayer>		pTileLayer;
	bool				bResult;

	uiNumElements = maTileLayers.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pTileLayer = maTileLayers.Get(i);
		if (pTileLayer->UsesCelBlitters())
		{
			bResult = pTileLayer->CreateCelBlitters();
			if (!bResult)
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTileMap::HasCelBlitters(void)
{
	size				uiNumElements;
	size				i;
	Ptr<CTileLayer>		pTileLayer;
	bool				bResult;

	uiNumElements = maTileLayers.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pTileLayer = maTileLayers.Get(i);
		bResult = pTileLayer->HasCelBlitters();
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
bool CTileMap::UsesCelBlitters(void)
{
	size				uiNumElements;
	size				i;
	Ptr<CTileLayer>		pTileLayer;
	bool				bResult;

	uiNumElements = maTileLayers.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pTileLayer = maTileLayers.Get(i);
		bResult = pTileLayer->UsesCelBlitters();
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
Ptr<CTileLayer> CTileMap::GetTileLayer(int iIdentifier)
{
	size				uiNumElements;
	size				i;
	Ptr<CTileLayer>		pTileLayer;

	uiNumElements = maTileLayers.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pTileLayer = maTileLayers.Get(i);
		if (pTileLayer->IsIdentifier(iIdentifier))
		{
			return pTileLayer;
		}
	}
	return NULL;
}

