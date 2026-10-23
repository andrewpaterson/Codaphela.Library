/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "CheckBoxImages.h"


//////////////////////////////////////////////////////////////////////////
//
// Every cel must be set before the images are used.
//
//////////////////////////////////////////////////////////////////////////
void CCheckBoxImages::Init(void)
{
	PreInit();

	maCels.Init();
	maCels.GrowTo(NUM_CHECK_BOX_STATES * 2);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCheckBoxImages::Class(void)
{
	M_Embedded(maCels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCheckBoxImages::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCheckBoxImages::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCheckBoxImages::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCheckBoxImages::SetCel(bool bTicked, ECheckBoxState eState, Ptr<CImageCel> pCel)
{
	maCels.Set((bTicked ? NUM_CHECK_BOX_STATES : 0) + eState, pCel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CCheckBoxImages::GetCel(bool bTicked, ECheckBoxState eState)
{
	return maCels.Get((bTicked ? NUM_CHECK_BOX_STATES : 0) + eState);
}

