/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "InputChordCriteriaDesc.h"
#include "InputChordDesc.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDesc::Init(char* szActionName)
{
	mszActionName.Init(szActionName);
	mpcRootCriteriaDesc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDesc::Kill(void)
{
	if (mpcRootCriteriaDesc)
	{
		mpcRootCriteriaDesc->Kill();
		mpcRootCriteriaDesc = NULL;
	}
	mszActionName.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordActiveDesc* CInputChordDesc::AsActiveOrInactive(void)
{
	if (mpcRootCriteriaDesc == NULL)
	{
		mpcRootCriteriaDesc = gcUnknowns.Add<CInputChordActiveDesc>();
		return (CInputChordActiveDesc*)mpcRootCriteriaDesc;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDesc::AsActive(CInputSourceDesc* pcSourceDesc, size iDescriptionID)
{
	if (mpcRootCriteriaDesc == NULL)
	{
		mpcRootCriteriaDesc = gcUnknowns.Add<CInputChordActiveDesc>();
		((CInputChordActiveDesc*)mpcRootCriteriaDesc)->Init(pcSourceDesc, iDescriptionID, ICCDT_Active, this, ICT_Toggle);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDesc::AsInactive(CInputSourceDesc* pcSourceDesc, size iDescriptionID)
{
	if (mpcRootCriteriaDesc == NULL)
	{
		mpcRootCriteriaDesc = gcUnknowns.Add<CInputChordActiveDesc>();
		((CInputChordActiveDesc*)mpcRootCriteriaDesc)->Init(pcSourceDesc, iDescriptionID, ICCDT_Inactive, this, ICT_Toggle);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDesc::AsActive(CInputCategoryGeneric* pcGeneric)
{
	if (mpcRootCriteriaDesc == NULL)
	{
		mpcRootCriteriaDesc = gcUnknowns.Add<CInputChordActiveDesc>();
		((CInputChordActiveDesc*)mpcRootCriteriaDesc)->Init(pcGeneric, ICCDT_Active, this, ICT_Toggle);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDesc::AsInactive(CInputCategoryGeneric* pcGeneric)
{
	if (mpcRootCriteriaDesc == NULL)
	{
		mpcRootCriteriaDesc = gcUnknowns.Add<CInputChordActiveDesc>();
		((CInputChordActiveDesc*)mpcRootCriteriaDesc)->Init(pcGeneric, ICCDT_Inactive, this, ICT_Toggle);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordCollectiveDesc* CInputChordDesc::AsOrdered(void)
{
	if (mpcRootCriteriaDesc == NULL)
	{
		mpcRootCriteriaDesc = gcUnknowns.Add<CInputChordCollectiveDesc>();
		((CInputChordCollectiveDesc*)mpcRootCriteriaDesc)->Init(ICCDT_Ordered, this);
		return (CInputChordCollectiveDesc*)mpcRootCriteriaDesc;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordCollectiveDesc* CInputChordDesc::AsGroup(void)
{
	if (mpcRootCriteriaDesc == NULL)
	{
		mpcRootCriteriaDesc = gcUnknowns.Add<CInputChordCollectiveDesc>();
		((CInputChordCollectiveDesc*)mpcRootCriteriaDesc)->Init(ICCDT_Group, this);
		return (CInputChordCollectiveDesc*)mpcRootCriteriaDesc;
	}
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordCriteriaDesc* CInputChordDesc::GetRootCriteriaDesc(void) { return mpcRootCriteriaDesc; }
char* CInputChordDesc::GetActionName(void) { return mszActionName.Text(); }
