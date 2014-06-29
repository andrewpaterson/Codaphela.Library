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
#include "InputDeviceDesc.h"
#include "InputSourceDesc.h"
#include "InputChordDescs.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDescs::Init(void)
{
	mlcChordDescs.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDescs::Kill(void)
{
	mlcChordDescs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordDesc* CInputChordDescs::AddChordDesc(char* szActionName)
{
	CInputChordDesc*	pcChord;

	pcChord = mlcChordDescs.Add();
	pcChord->Init(szActionName);

	return pcChord;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDescs::GetInputSourceDescs(CArrayTypedPointer* apcDest, CInputDeviceDesc* pcDeviceDesc)
{
	CInputChordDesc*		pcChordDesc;
	SSetIterator			sIter;
	
	pcChordDesc = mlcChordDescs.StartIteration(&sIter);
	while (pcChordDesc)
	{
		if (pcChordDesc->GetRootCriteriaDesc())
		{
			pcChordDesc->GetRootCriteriaDesc()->GetInputSourceDescs(apcDest, pcDeviceDesc);
		}
		pcChordDesc = mlcChordDescs.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDescs::CopyChordDescs(CInputChordDescs* pcSource, CInputDeviceCopyContext* pcContext)
{
	CInputChordDesc*	pcSourceChordDesc;
	SSetIterator		sIter;
	CInputChordDesc*	pcDestChordDesc;

	pcSourceChordDesc = pcSource->mlcChordDescs.StartIteration(&sIter);
	while (pcSourceChordDesc)
	{
		pcDestChordDesc = AddChordDesc(pcSourceChordDesc->GetActionName());
		pcContext->mmppChords.Put(pcSourceChordDesc, pcDestChordDesc);

		if (pcSourceChordDesc->GetRootCriteriaDesc())
		{
			CopyChordDesc(pcDestChordDesc, pcSourceChordDesc->GetRootCriteriaDesc(), pcContext);
		}
		pcSourceChordDesc = pcSource->mlcChordDescs.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordDesc* CInputChordDescs::StartChordDescsIteration(SSetIterator* psIter)
{
	return mlcChordDescs.StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChordDesc* CInputChordDescs::IterateChordDescs(SSetIterator* psIter)
{
	return mlcChordDescs.Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDescs::CopyChordDesc(CInputChordDesc* pcDest, CInputChordCriteriaDesc* pcSource, CInputDeviceCopyContext* pcContext)
{
	CInputChordCriteriaDesc*	pcDestCriteria;

	switch (pcSource->GetType())
	{
	case ICCDT_Active:
	case ICCDT_Inactive:
		pcDestCriteria = pcDest->AsActiveOrInactive();
		CopyChordCriteriaDesc(pcDestCriteria, pcSource, pcContext);
		break;
	case ICCDT_Group:
		pcDestCriteria = pcDest->AsGroup();
		CopyChordCriteriaDesc(pcDestCriteria, pcSource, pcContext);
		break;
	case ICCDT_Ordered:
		pcDestCriteria = pcDest->AsOrdered();
		CopyChordCriteriaDesc(pcDestCriteria, pcSource, pcContext);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputChordDescs::CopyChordCriteriaDesc(CInputChordCriteriaDesc* pcDest, CInputChordCriteriaDesc* pcSource, CInputDeviceCopyContext* pcContext)
{
	CInputChordActiveDesc*		pcDestActive;
	CInputChordActiveDesc*		pcSourceActive;
	CInputChordCriteriaDesc*	pcSourceCriteria;
	CInputChordCollectiveDesc*	pcSourceCollective;
	CInputChordCollectiveDesc*	pcDestCollective;
	CInputChordCollectiveDesc*	pcDestCollective2;
	int							i;
	CInputSourceDesc*			pcDestSource;
	CInputChordDesc*			pcDestChord;

	switch (pcSource->GetType())
	{
	case ICCDT_Active:
	case ICCDT_Inactive:
		pcSourceActive = (CInputChordActiveDesc*)pcSource;
		pcDestActive = (CInputChordActiveDesc*)pcDest;

		pcDestChord = (CInputChordDesc*)pcContext->mmppChords.GetWithKey(pcSourceActive->GetChordDesc());
		switch (pcSourceActive->meActiveType)
		{
		case ICCDAT_SpecificSource:
			pcDestSource = pcContext->mpcDestDeviceDesc->GetSource(pcSourceActive->GetSourceDesc()->GetFriendlyName());
			pcDestActive->Init(pcDestSource, pcSourceActive->GetDescriptionID(), pcSourceActive->GetType(), pcDestChord, pcSourceActive->meChordType);
			break;
		case ICCDAT_GenericSource:
			pcDestActive->Init(pcSourceActive->mpcGeneric, pcSourceActive->GetType(), pcDestChord, pcSourceActive->meChordType);
			break;
		case ICCDAT_AnyToggleSource:
			pcDestActive->Init(pcSourceActive->GetType(), pcDestChord);
			break;
		}
		break;
	case ICCDT_Group:
	case ICCDT_Ordered:
		pcSourceCollective = (CInputChordCollectiveDesc*)pcSource;
		pcDestCollective = (CInputChordCollectiveDesc*)pcDest;
		pcDestCollective->Init(pcSourceCollective->GetType(), pcSourceCollective->GetChordDesc());

		for (i = 0; i < pcSourceCollective->macCriteria.NumElements(); i++)
		{
			pcSourceCriteria = (CInputChordCriteriaDesc*)pcSourceCollective->macCriteria.Get(i);
			switch (pcSourceCriteria->GetType())
			{
			case ICCDT_Active:
			case ICCDT_Inactive:
				pcDestActive = pcDestCollective->AddActiveOrInactive();
				CopyChordCriteriaDesc(pcDestActive, pcSourceCriteria, pcContext);
				break;
			case ICCDT_Group:
			case ICCDT_Ordered:
				pcDestCollective2 = pcDestCollective->AddGroupOrOrdered();
				CopyChordCriteriaDesc(pcDestCollective2, pcSourceCriteria, pcContext);
				break;
			}
		}
		break;
	}
}

