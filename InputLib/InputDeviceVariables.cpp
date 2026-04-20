/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
	mbChordThisFrame = false;
	mlcVariables.Init(false);
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
	mbChordThisFrame = false;
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
	bool						bExists;

	bExists = pcDesc->StartVariablesIteration(&sIter, &pcVariableDesc);
	while (bExists)
	{
		pcVariable = mlcVariables.Add();

		pcVariable->Init(pcVariableDesc);
		AddVariableValuesFromDesc(pcVariableDesc, pcVariable);

		bExists = pcDesc->IterateVariables(&sIter, &pcVariableDesc);
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
	bool							bExists;

	bExists = pcVariableDesc->StartValuesIteration(&sIter, &pcValueDesc);
	while (bExists)
	{
		pcValue = pcVariable->AddValue(pcValueDesc);

		if (pcValueDesc->IsInitial())
		{
			pcVariable->SetValue(pcValue);
		}

		bExists = pcVariableDesc->IterateValues(&sIter, &pcValueDesc);
	}

	bExists = pcVariable->StartValuesIteration(&sIter, &pcValue);
	while (bExists)
	{
		AddVariableConditionsFromDescription(pcValue->GetDesc(), pcValue);
		bExists = pcVariable->IterateValues(&sIter, &pcValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceVariables::AddVariableConditionsFromDescription(CInputDeviceVariableValueDesc* pcValueDesc, CInputDeviceVariableValue* pcValue)
{
	size											i;
	CInputDeviceVariableValueConditionDesc*			pcConditionDesc;
	CInputChord*									pcChord;
	CInputDeviceVariableValue*						pcVariableValue;
	CArrayInputDeviceVariableValueConditionDesc*	pcConditions;
	size											uiNumElements;

	pcConditions = pcValueDesc->GetConditions();
	uiNumElements = pcConditions->NumElements();
	for (i = 0; i < uiNumElements; i++)
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
	bool			bExists;

	if (pcChordDesc == NULL)
	{
		return NULL;
	}

	bExists = mpcChords->StartChordsIteration(&sIter, &pcChord);
	while (bExists)
	{
		if (pcChord->GetDesc() == pcChordDesc)
		{
			return pcChord;
		}
		bExists = mpcChords->IterateChords(&sIter, &pcChord);
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
	bool						bExists;
	bool						bExists2;

	bExists = mlcVariables.StartIteration(&sIter, &pcVariable);
	while (bExists)
	{
		bExists2 = pcVariable->StartValuesIteration(&sIter2, &pcValue);
		while (bExists2)
		{
			if (pcValue->GetDesc() == pcValueDesc)
			{
				return pcValue;
			}
			bExists2 = pcVariable->IterateValues(&sIter2, &pcValue);
		}
		bExists = mlcVariables.Iterate(&sIter, &pcVariable);
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
	bool								bAnyChange;
	bool								bExists;

	if (mbChordThisFrame)
	{
		return;
	}

	apcVariableValues.Init();
	bExists = mlcVariables.StartIteration(&sIter, &pcVariable);
	while (bExists)
	{
		VariableAction(pcVariable, pcChord, &apcVariableValues);
		bExists = mlcVariables.Iterate(&sIter, &pcVariable);
	}

	bAnyChange = UpdateChordVariableValues(&apcVariableValues);
	apcVariableValues.Kill();

	if (bAnyChange)
	{
		mbChordThisFrame = true;
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
	bool						bResult;
	bool						bExists;

	bExists = pcVariable->StartValuesIteration(&sIter, &pcValue);
	while (bExists)
	{
		bResult = pcValue->MatchCondition(pcChord);
		if (bResult)
		{
			papcVariableValues->Add(&pcValue);
		}
		bExists = pcVariable->IterateValues(&sIter, &pcValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDeviceVariables::UpdateChordVariableValues(CArrayInputDeviceVariableValuePtr* papcVariableValues)
{
	size						i;
	CInputDeviceVariableValue*	pcValue;
	bool						bAnyChange;
	bool						bResult;
	size						uiNumElements;

	bAnyChange = false;
	uiNumElements = papcVariableValues->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcValue = *papcVariableValues->Get(i);
		bResult = pcValue->MakeCurrent();
		if (bResult)
		{
			bAnyChange = true;
		}
	}
	return bAnyChange;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDeviceVariables::UpdateNonChordVariableValues(void)
{
	SSetIterator			sIter;
	CInputDeviceVariable*	pcVariable;
	bool					bResult;
	bool					bAnyChange;
	bool					bExists;

	bAnyChange = false;
	bExists = mlcVariables.StartIteration(&sIter, &pcVariable);
	while (bExists)
	{
		bResult = UpdateNonChordVariableValues(pcVariable);
		if (bResult)
		{
			bAnyChange = true;
		}
		bExists = mlcVariables.Iterate(&sIter, &pcVariable);
	}
	return bAnyChange;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDeviceVariables::UpdateNonChordVariableValues(CInputDeviceVariable* pcVariable)
{
	SSetIterator				sIter;
	CInputDeviceVariableValue*	pcValue;
	bool						bResult;
	bool						bAnyChange;
	bool						bExists;

	bAnyChange = false;
	bExists = pcVariable->StartValuesIteration(&sIter, &pcValue);
	while (bExists)
	{
		bResult = pcValue->MatchNonChordCondition();
		if (bResult)
		{
			bResult = pcValue->MakeCurrent();
			if (bResult)
			{
				bAnyChange = true;
			}
		}
		bExists = pcVariable->IterateValues(&sIter, &pcValue);
	}
	return bAnyChange;
}

