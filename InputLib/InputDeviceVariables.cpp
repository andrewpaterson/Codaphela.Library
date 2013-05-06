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
#include "InputChords.h"
#include "InputDeviceVariables.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariables::Init(void)
{
	mpcChords = NULL;
	mbChordThisFrame = FALSE;
	mlcVariables.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariables::Kill(void)
{
	mlcVariables.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariables::SetChords(CInputChords* pcChords)
{
	mpcChords = pcChords;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariables::ClearEvents(void)
{
	mbChordThisFrame = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariables::AddVariablesFromDesc(CInputDeviceDesc* pcDesc)
{
	SSetIterator				sIter;
	CInputDeviceVariableDesc*	pcVariableDesc;
	CInputDeviceVariable*		pcVariable;

	pcVariableDesc = pcDesc->StartVariablesIteration(&sIter);
	while (pcVariableDesc)
	{
		pcVariable = mlcVariables.Add();

		pcVariable->Init(pcVariableDesc);
		AddVariableValuesFromDesc(pcVariableDesc, pcVariable);

		pcVariableDesc = pcDesc->IterateVariables(&sIter);
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariables::AddVariableValuesFromDesc(CInputDeviceVariableDesc* pcVariableDesc, CInputDeviceVariable* pcVariable)
{
	SSetIterator					sIter;
	CInputDeviceVariableValueDesc*	pcValueDesc;
	CInputDeviceVariableValue*		pcValue;

	pcValueDesc = pcVariableDesc->StartValuesIteration(&sIter);
	while (pcValueDesc)
	{
		pcValue = pcVariable->AddValue(pcValueDesc);

		if (pcValueDesc->IsInitial())
		{
			pcVariable->SetValue(pcValue);
		}

		pcValueDesc = pcVariableDesc->IterateValues(&sIter);
	}

	pcValue = pcVariable->StartValuesIteration(&sIter);
	while (pcValue)
	{
		AddVariableConditionsFromDescription(pcValue->GetDesc(), pcValue);
		pcValue = pcVariable->IterateValues(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariables::AddVariableConditionsFromDescription(CInputDeviceVariableValueDesc* pcValueDesc, CInputDeviceVariableValue* pcValue)
{
	int												i;
	CInputDeviceVariableValueConditionDesc*			pcConditionDesc;
	CInputChord*									pcChord;
	CInputDeviceVariableValue*						pcVariableValue;
	CArrayInputDeviceVariableValueConditionDesc*	pcConditions;

	pcConditions = pcValueDesc->GetConditions();
	for (i = 0; i < pcConditions->NumElements(); i++)
	{
		pcConditionDesc = (CInputDeviceVariableValueConditionDesc *)pcConditions->Get(i);
		if (pcConditionDesc->IsChord())
		{
			pcChord = GetChordForDesc(((CInputDeviceVariableValueConditionChordDesc*)pcConditionDesc)->mpcChordDesc);
			pcValue->SetChordCondition(pcChord);
		}
		else if (pcConditionDesc->IsVariable())
		{
			pcVariableValue = GetValueForDesc(((CInputDeviceVariableValueConditionVariableDesc*)pcConditionDesc)->mpcVariableValueDesc);
			pcValue->AddValueCondition(pcVariableValue);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputChord* CInputDeviceVariables::GetChordForDesc(CInputChordDesc* pcChordDesc)
{
	SSetIterator	sIter;
	CInputChord*	pcChord;

	if (pcChordDesc == NULL)
	{
		return NULL;
	}

	pcChord = mpcChords->StartChordsIteration(&sIter);
	while (pcChord)
	{
		if (pcChord->GetDesc() == pcChordDesc)
		{
			return pcChord;
		}
		pcChord = mpcChords->IterateChords(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableValue* CInputDeviceVariables::GetValueForDesc(CInputDeviceVariableValueDesc* pcValueDesc)
{
	SSetIterator				sIter;
	CInputDeviceVariable*		pcVariable;
	SSetIterator				sIter2;
	CInputDeviceVariableValue*	pcValue;

	pcVariable = mlcVariables.StartIteration(&sIter);
	while (pcVariable)
	{
		pcValue = pcVariable->StartValuesIteration(&sIter2);
		while (pcValue)
		{
			if (pcValue->GetDesc() == pcValueDesc)
			{
				return pcValue;
			}
			pcValue = pcVariable->IterateValues(&sIter2);
		}
		pcVariable = mlcVariables.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariables::VariableAction(CInputChord* pcChord)
{
	SSetIterator						sIter;
	CInputDeviceVariable*				pcVariable;
	CArrayInputDeviceVariableValuePtr	apcVariableValues;
	BOOL								bAnyChange;

	if (mbChordThisFrame)
	{
		return;
	}

	apcVariableValues.Init(4);
	pcVariable = mlcVariables.StartIteration(&sIter);
	while (pcVariable)
	{
		VariableAction(pcVariable, pcChord, &apcVariableValues);
		pcVariable = mlcVariables.Iterate(&sIter);
	}

	bAnyChange = UpdateChordVariableValues(&apcVariableValues);
	apcVariableValues.Kill();

	if (bAnyChange)
	{
		mbChordThisFrame = TRUE;
	}

	while (bAnyChange)
	{
		bAnyChange = UpdateNonChordVariableValues();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariables::VariableAction(CInputDeviceVariable* pcVariable, CInputChord* pcChord, CArrayInputDeviceVariableValuePtr* papcVariableValues)
{
	SSetIterator				sIter;
	CInputDeviceVariableValue*	pcValue;
	BOOL						bResult;

	pcValue = pcVariable->StartValuesIteration(&sIter);
	while (pcValue)
	{
		bResult = pcValue->MatchCondition(pcChord);
		if (bResult)
		{
			papcVariableValues->Add(&pcValue);
		}
		pcValue = pcVariable->IterateValues(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariables::UpdateChordVariableValues(CArrayInputDeviceVariableValuePtr* papcVariableValues)
{
	int							i;
	CInputDeviceVariableValue*	pcValue;
	BOOL						bAnyChange;
	BOOL						bResult;

	bAnyChange = FALSE;
	for (i = 0; i < papcVariableValues->NumElements(); i++)
	{
		pcValue = *papcVariableValues->Get(i);
		bResult = pcValue->MakeCurrent();
		if (bResult)
		{
			bAnyChange = TRUE;
		}
	}
	return bAnyChange;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariables::UpdateNonChordVariableValues(void)
{
	SSetIterator			sIter;
	CInputDeviceVariable*	pcVariable;
	BOOL					bResult;
	BOOL					bAnyChange;

	bAnyChange = FALSE;
	pcVariable = mlcVariables.StartIteration(&sIter);
	while (pcVariable)
	{
		bResult = UpdateNonChordVariableValues(pcVariable);
		if (bResult)
		{
			bAnyChange = TRUE;
		}
		pcVariable = mlcVariables.Iterate(&sIter);
	}
	return bAnyChange;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputDeviceVariables::UpdateNonChordVariableValues(CInputDeviceVariable* pcVariable)
{
	SSetIterator				sIter;
	CInputDeviceVariableValue*	pcValue;
	BOOL						bResult;
	BOOL						bAnyChange;

	bAnyChange = FALSE;
	pcValue = pcVariable->StartValuesIteration(&sIter);
	while (pcValue)
	{
		bResult = pcValue->MatchNonChordCondition();
		if (bResult)
		{
			bResult = pcValue->MakeCurrent();
			if (bResult)
			{
				bAnyChange = TRUE;
			}
		}
		pcValue = pcVariable->IterateValues(&sIter);
	}
	return bAnyChange;
}

