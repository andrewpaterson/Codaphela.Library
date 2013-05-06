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
#include "InputDeviceDesc.h"
#include "InputChordCriteriaDesc.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordCriteriaDesc::Init(EInputChordCriteriaDescType eType, CInputChordDesc* pcChordDesc)
{
	meType = eType;
	mpcChordDesc = pcChordDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordCriteriaDesc::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EInputChordCriteriaDescType CInputChordCriteriaDesc::GetType(void) { return meType; }
CInputChordDesc* CInputChordCriteriaDesc::GetChordDesc(void) { return mpcChordDesc; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordActiveDesc::Init(CInputSourceDesc* pcSourceDesc, int iDescriptionID, EInputChordCriteriaDescType eType, CInputChordDesc* pcChordDesc, EInputChordType eChordType)
{
	CInputChordCriteriaDesc::Init(eType, pcChordDesc);
	mpcGeneric = NULL;
	meActiveType = ICCDAT_SpecificSource;
	meChordType = eChordType;
	mpcSourceDesc = pcSourceDesc;
	miDescriptionID = iDescriptionID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordActiveDesc::Init(CInputCategoryGeneric* pcSource, EInputChordCriteriaDescType eType, CInputChordDesc* pcChordDesc, EInputChordType eChordType)
{
	CInputChordCriteriaDesc::Init(eType, pcChordDesc);
	mpcSourceDesc = NULL;
	miDescriptionID = -1;
	mpcGeneric = pcSource;
	meActiveType = ICCDAT_GenericSource;
	meChordType = eChordType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordActiveDesc::Init(EInputChordCriteriaDescType eType, CInputChordDesc* pcChordDesc)
{
	CInputChordCriteriaDesc::Init(eType, pcChordDesc);
	mpcSourceDesc = NULL;
	miDescriptionID = -1;
	mpcGeneric = NULL;
	meActiveType = ICCDAT_AnyToggleSource;
	meChordType = ICT_Toggle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordActiveDesc::Kill(void)
{
	CInputChordCriteriaDesc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordActiveDesc::GetInputSourceDescs(CArrayPointer* apcDest, CInputDeviceDesc* pcDeviceDesc)
{
	if (meActiveType == ICCDAT_SpecificSource)
	{
		apcDest->AddIfUnique(mpcSourceDesc, -1);
	}
	else if (meActiveType == ICCDAT_GenericSource)
	{
		pcDeviceDesc->GetSources(apcDest, mpcGeneric);
	}
	else if (meActiveType == ICCDAT_AnyToggleSource)
	{
		pcDeviceDesc->GetSources(apcDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CInputChordActiveDesc::GetSourceDesc(void)
{
	return mpcSourceDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CInputChordActiveDesc::GetDescriptionID(void)
{
	return miDescriptionID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordCollectiveDesc::Init(EInputChordCriteriaDescType eType, CInputChordDesc* pcChordDesc)
{
	CInputChordCriteriaDesc::Init(eType, pcChordDesc);
	macCriteria.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordCollectiveDesc::Kill(void)
{
	macCriteria.Kill();
	CInputChordCriteriaDesc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordActiveDesc* CInputChordCollectiveDesc::AddActiveOrInactive(void)
{
	CInputChordActiveDesc*	pcChordDesc;

	pcChordDesc = macCriteria.Add<CInputChordActiveDesc>();
	return pcChordDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordActiveDesc* CInputChordCollectiveDesc::AddActive(CInputSourceDesc* pcSourceDesc, int iDescriptionID)
{
	CInputChordActiveDesc*	pcChordDesc;

	pcChordDesc = macCriteria.Add<CInputChordActiveDesc>();
	pcChordDesc->Init(pcSourceDesc, iDescriptionID, ICCDT_Active, mpcChordDesc, ICT_Toggle);
	return pcChordDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordActiveDesc* CInputChordCollectiveDesc::AddInactive(CInputSourceDesc* pcSourceDesc, int iDescriptionID)
{
	CInputChordActiveDesc*	pcChordDesc;

	pcChordDesc = macCriteria.Add<CInputChordActiveDesc>();
	pcChordDesc->Init(pcSourceDesc, iDescriptionID, ICCDT_Inactive, mpcChordDesc, ICT_Toggle);
	return pcChordDesc;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordCollectiveDesc* CInputChordCollectiveDesc::AddGroupOrOrdered(void)
{
	CInputChordCollectiveDesc*	pcDesc;

	pcDesc = macCriteria.Add<CInputChordCollectiveDesc>();
	return pcDesc;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordCollectiveDesc* CInputChordCollectiveDesc::AddGroup(void)
{
	CInputChordCollectiveDesc*	pcDesc;

	pcDesc = macCriteria.Add<CInputChordCollectiveDesc>();
	pcDesc->Init(ICCDT_Group, mpcChordDesc);
	return pcDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordCollectiveDesc::RemoveLast(void)
{
	int							iNumElements;

	iNumElements = macCriteria.NumElements();
	if (iNumElements == 0)
	{
		return;
	}
	else
	{
		macCriteria.Remove(iNumElements-1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordCollectiveDesc::GetInputSourceDescs(CArrayPointer* apcDest, CInputDeviceDesc* pcDeviceDesc)
{
	int							i;
	CInputChordCriteriaDesc*	pcCriteriaDesc;

	for (i = 0; i < macCriteria.NumElements(); i++)
	{
		pcCriteriaDesc = (CInputChordCriteriaDesc*)macCriteria.Get(i);
		pcCriteriaDesc->GetInputSourceDescs(apcDest, pcDeviceDesc);
	}
}
