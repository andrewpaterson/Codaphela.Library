/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "InputSourceDesc.h"
#include "InputDeviceDesc.h"
#include "InputDeviceState.h"
#include "InputDeviceValue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceDesc::Init(CInputDeviceDesc* pcDeviceDesc, EInputSourceType eType, char* szFriendlyName, int iStateIndex)
{
	meType = eType;
	mszFriendlyName.Init(szFriendlyName);
	mlcValues.Init();
	mapcGenerics.Init(1);
	mfRestValue = 0.0f;
	mbHasRestValue = FALSE;
	mbEmitRestEvent = FALSE;
	mpcDeviceDesc = pcDeviceDesc;
	miStateIndex = iStateIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceDesc::Kill(void)
{
	mapcGenerics.Kill();
	mlcValues.Kill();
	mszFriendlyName.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceDesc::Process(void* pvData, CInputDeviceState* pcState, CInputDeviceValues* pcEvents, unsigned int uiSequence)
{
	CInputSourceValue*	pcSourceValue;
	float				fValue;
	BOOL				bValid;
	BOOL				bNewValue;
	SSetIterator		sIter;

	pcSourceValue = mlcValues.StartIteration(&sIter);
	while (pcSourceValue)
	{
		bValid = pcSourceValue->Process(pvData, &fValue);
		if (bValid)
		{
			bNewValue = pcState->SetValue(this, fValue);
			if (bNewValue)
			{
				pcEvents->Add(pcState->mpcDevice, this, fValue, uiSequence, FALSE, pcSourceValue->GetOrder());
			}
		}
		pcSourceValue = mlcValues.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceValue* CInputSourceDesc::AddValue(int iValueChannel)
{
	CInputSourceValue* pcvalue;

	pcvalue = mlcValues.Add();
	pcvalue->Init(this, mpcDeviceDesc->GetDataFormat(), iValueChannel);
	return pcvalue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceValue* CInputSourceDesc::AddValue(float fValueNumber)
{
	CInputSourceValue* pcvalue;

	pcvalue = mlcValues.Add();
	pcvalue->Init(this, mpcDeviceDesc->GetDataFormat(), fValueNumber);
	return pcvalue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceDesc::CopyValues(CInputSourceDesc* pcInputSourceDesc)
{
	CInputSourceValue*	pcInputSourceValue;
	CInputSourceValue*	pcInputDestValue;
	SSetIterator		sIter;

	pcInputSourceValue = pcInputSourceDesc->StartValuesIteration(&sIter);
	while (pcInputSourceValue)
	{
		pcInputDestValue = mlcValues.Add();
		if (pcInputSourceValue->GetValueType() == ISVT_Numeric)
		{
			pcInputDestValue->Init(this, pcInputSourceValue->GetDataFormat(), pcInputSourceValue->GetNumber());
		}
		else if (pcInputSourceValue->GetValueType() == ISVT_Channel)
		{
			pcInputDestValue->Init(this, pcInputSourceValue->GetDataFormat(), pcInputSourceValue->GetChannel());
		}
		
		if (pcInputSourceDesc->GetType() == ISET_State)
		{
			pcInputDestValue->SetStateDetail(pcInputSourceValue->GetMaxValue(), pcInputSourceValue->GetMinValue());
		}
		else if (pcInputSourceDesc->GetType() == ISET_Delta)
		{
			pcInputDestValue->SetDeltaDetail(pcInputSourceValue->GetSensitivity(), pcInputSourceValue->GetOffset());
		}

		pcInputDestValue->CopyChannels(pcInputSourceValue);
		pcInputSourceValue = pcInputSourceDesc->IterateValues(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceDesc::CopyActions(CInputSourceDesc* pcInputSourceDesc)
{
	mapcGenerics.Copy(&pcInputSourceDesc->mapcGenerics);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceDesc::SetRest(float fRestValue, BOOL bEmitRestEvent, BOOL bHasRestValue)
{
	mbHasRestValue = bHasRestValue;
	mfRestValue = fRestValue;
	mbEmitRestEvent = bEmitRestEvent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputSourceDesc::Is(char* szFriendlyName)
{
	return mszFriendlyName.Equals(szFriendlyName);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceValue* CInputSourceDesc::StartValuesIteration(SSetIterator* psIter) { return mlcValues.StartIteration(psIter); }
CInputSourceValue* CInputSourceDesc::IterateValues(SSetIterator* psIter) { return mlcValues.Iterate(psIter); }
float CInputSourceDesc::GetRestValue(void) { return mfRestValue; }
BOOL CInputSourceDesc::HasRestValue(void) { return mbHasRestValue; }
CInputDeviceDesc* CInputSourceDesc::GetDeviceDesc(void) { return mpcDeviceDesc; }
CArrayInputCategoryGenericPtr* CInputSourceDesc::GetGenerics(void) { return &mapcGenerics; }
EInputSourceType CInputSourceDesc::GetType(void) { return meType; }
int CInputSourceDesc::GetStateIndex(void) { return miStateIndex; }
char* CInputSourceDesc::GetFriendlyName(void) { return mszFriendlyName.Text(); }
BOOL CInputSourceDesc::GetEmitRestEvent(void) { return mbEmitRestEvent; }
void CInputSourceDesc::SetFriendlyName(char* szFriendlyName) { mszFriendlyName.Set(szFriendlyName); }

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceDesc::ToString(CChars* psz)
{
	int					i;
	SSetIterator		sIter;
	CInputSourceValue*	pcInputSourceValue;

	psz->Append(" --- CInputSourceDesc ---\n");
	psz->Append("Name: ");
	psz->Append(mszFriendlyName.Text());
	psz->AppendNewLine();
	psz->Append("Rest Value: ");
	psz->Append(mfRestValue, 2);
	psz->AppendNewLine();
	psz->Append("Emit Rest Event: ");
	psz->AppendBool(mbEmitRestEvent);
	psz->AppendNewLine();
	psz->Append("Has Rest Value: ");
	psz->AppendBool(mbHasRestValue);
	psz->AppendNewLine();
	psz->Append("State Index: ");
	psz->Append(miStateIndex);
	psz->AppendNewLine();

	psz->Append(" - CArrayChars : mapcGenerics -\n");
	for (i = 0; i < mapcGenerics.NumElements(); i++)
	{
	}

	psz->Append(" - CSetInputSourceValue : mlcValues -\n");
	pcInputSourceValue = mlcValues.StartIteration(&sIter);
	while (pcInputSourceValue)
	{
		pcInputSourceValue->ToString(psz, meType);
		pcInputSourceValue = mlcValues.Iterate(&sIter);
	}
}

