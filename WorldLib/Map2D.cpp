/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "Map2D.h"
#include "World.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMap2D::Init(CGraphicsObject* pcGraphicsObject)
{
	macActiveSet.Init();
	macCacheSet.Init();
	macActiveSet.KillElements(FALSE);
	macCacheSet.KillElements(FALSE);
	mpcGraphicsObject = pcGraphicsObject;
	mcPosition.x = 0;
	mcPosition.y = 0;
	mcPosition.z = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMap2D::Kill(void)
{
	macActiveSet.KillElements(TRUE);
	macCacheSet.KillElements(TRUE);
	macCacheSet.Kill();
	macActiveSet.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMap2D::Draw(void)
{
	CQuad2D*		pcQuad;
	SInt3			cPos;
	int				iVisible;
	SSetIterator	sIter;

	iVisible = 0;
	pcQuad = macActiveSet.StartIteration(&sIter);
	while (pcQuad)
	{
		cPos = pcQuad->GetPosition();
		pcQuad->MovePosition(mcPosition.x, mcPosition.y);
		if (pcQuad->IsInViewport())
		{
			pcQuad->Draw(mpcGraphicsObject);
			iVisible++;
		}
		pcQuad->SetPosition(&cPos);
		pcQuad = macActiveSet.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMap2D::SetPosition(int x, int y)
{
	mcPosition.x = x;
	mcPosition.y = y;
	mcPosition.z = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CQuad2D* CMap2D::CreateQuad(CGraphicsMaterial* pcGraphicsMaterial, CGraphicsState* pcGraphicsState, CViewportInstance* pcGraphicsViewport, int iWidth, int iHeight, int x, int y)
{
	CQuad2D*	pcQuad;

	if (macCacheSet.NumElements() > 0)
	{
		pcQuad = macCacheSet.Last();
		macCacheSet.RemoveLast();
	}
	else
	{
		pcQuad = gcUnknowns.Add<CQuad2D>();
	}

	if (pcQuad)
	{
		pcQuad->Init(pcGraphicsMaterial, pcGraphicsState, pcGraphicsViewport, iWidth, iHeight, x, y);
	}
	macActiveSet.Add(pcQuad);
	return pcQuad;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMap2D::Clear(void)
{
	if (macActiveSet.NumElements() > 0)
	{
		macCacheSet.AddAll(&macActiveSet);
		macActiveSet.ReInit();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMap2D::NumQuads(void)
{
	return macActiveSet.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CQuad2D* CMap2D::StartIteration(SSetIterator* psIter)
{
	return macActiveSet.StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CQuad2D* CMap2D::Iterate(SSetIterator* psIter)
{
	return macActiveSet.Iterate(psIter);
}
