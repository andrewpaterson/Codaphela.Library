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
#include "BaseLib/Numbers.h"
#include "InputSourceDesc.h"
#include "InputSourceEvaluator.h"
#include "InputCategory.h"
#include "InputDeviceValue.h"
#include "InputVirtualDevice.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceEvaluator::Init(void)
{
	meEvaluatorType = IET_Unknown;
	meChordType = ICT_Unknown;

	memset(&muConfig, 0, sizeof(UInputConfig));

	mfStateDeadzone = 0.1f;
	mfDeltaSensitivity = 1.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceEvaluator::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceEvaluator::Copy(CInputSourceEvaluator* pcSource, CInputDevice* pcDevice)
{
	Init();

	mfStateDeadzone = pcSource->mfStateDeadzone;
	mfDeltaSensitivity = pcSource->mfDeltaSensitivity;
	meEvaluatorType = pcSource->meEvaluatorType;
	meChordType = pcSource->meChordType;

	if (meEvaluatorType == IET_SpecificSource)
	{
		muConfig.sDeviceSource.pcDevice = pcDevice;
		muConfig.sDeviceSource.pcSource = pcSource->muConfig.sDeviceSource.pcSource;
	}
	else if (meEvaluatorType == IET_GenericSource)
	{
		muConfig.sDeviceGeneric.pcGeneric = pcSource->muConfig.sDeviceGeneric.pcGeneric;
	}
	else if (meEvaluatorType == IET_AnyToggleSource)
	{
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceEvaluator::SetSpecificSource(CInputDevice* pcDevice, CInputSourceDesc* pcSource, EInputChordType eType)
{
	meEvaluatorType = IET_SpecificSource;
	meChordType = eType;
	muConfig.sDeviceSource.pcDevice = pcDevice;
	muConfig.sDeviceSource.pcSource = pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceEvaluator::SetSpecificSource(CInputVirtualDeviceSource* pcSource, EInputChordType eType)
{
	SetSpecificSource(pcSource->GetSourceDevice(), pcSource->GetSourceDesc(), eType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceEvaluator::SetGenericSource(CInputCategoryGeneric* pcGeneric, EInputChordType eType)
{
	meEvaluatorType = IET_GenericSource;
	meChordType = eType;
	muConfig.sDeviceGeneric.pcGeneric = pcGeneric;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceEvaluator::SetAnyToggleSource(void)
{
	meEvaluatorType = IET_AnyToggleSource;
	meChordType = ICT_Toggle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputSourceEvaluator::Evaluate(SInputDeviceValue* psEvent, float* pfValue)
{
	if (meEvaluatorType == IET_SpecificSource)
	{
		return EvaluateSpecificSource(pfValue, psEvent, &muConfig.sDeviceSource);
	}
	else if (meEvaluatorType == IET_GenericSource)
	{
		return EvaluateGenericSource(pfValue, psEvent, &muConfig.sDeviceGeneric);
	}
	else if (meEvaluatorType == IET_AnyToggleSource)
	{
		return EvaluateAnyToggleSource(pfValue, psEvent, &muConfig.sDeviceAny);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputSourceEvaluator::EvaluateSpecificSource(float* pfValue, SInputDeviceValue* psEvent, SInputConfigSpecificSource* psConfig)
{
	if ((psEvent->pcDevice == psConfig->pcDevice) && (psEvent->pcSource == psConfig->pcSource))
	{
		return EvaluateMatching(pfValue, psEvent);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputSourceEvaluator::EvaluateGenericSource(float* pfValue, SInputDeviceValue* psEvent, SInputConfigGenericSource* psConfig)
{
	if (psEvent->pcSource->GetGenerics()->Contains(&psConfig->pcGeneric))
	{
		return EvaluateMatching(pfValue, psEvent);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputSourceEvaluator::EvaluateAnyToggleSource(float* pfValue, SInputDeviceValue* psEvent, SInputConfigAnyToggleSource* psConfig)
{
	if (meChordType == ICT_Toggle)
	{
		return EvaluateMatching(pfValue, psEvent);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputSourceEvaluator::EvaluateMatching(float* pfValue, SInputDeviceValue* psEvent)
{
	float	fValue;

	if (psEvent->pcSource->GetType() == ISET_State)
	{
		if (meChordType == ICT_Toggle)
		{
			if (psEvent->fValue >= mfStateDeadzone)
			{
				*pfValue = 1.0f;
			}
			else
			{
				*pfValue = 0.0f;
			}
			return TRUE;
		}
		else if (meChordType == ICT_Range)
		{
			if (psEvent->fValue >= mfStateDeadzone)
			{
				*pfValue = psEvent->fValue;
			}
			else
			{
				*pfValue = 0.0f;
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else if (psEvent->pcSource->GetType() == ISET_Delta)
	{
		fValue = psEvent->fValue * mfDeltaSensitivity;

		//ISET_Delta intentionally does not understand negative fValues.
		//Use <Sensitivity>-1.0</Sensitivity> to correct.
		if (meChordType == ICT_Toggle)
		{
			if (fValue >= SMALL_NUMBER)
			{
				*pfValue = 1.0f;
			}
			else
			{
				*pfValue = 0.0f;
			}
			return TRUE;
		}
		else if (meChordType == ICT_Range)
		{
			if (fValue >= SMALL_NUMBER)
			{
				*pfValue = fValue;
			}
			else
			{
				*pfValue = 0.0f;
			}
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EInputEvaluatorType CInputSourceEvaluator::GetEvaluatorType(void) { return meEvaluatorType; }
EInputChordType CInputSourceEvaluator::GetChordType(void) { return meChordType; }
UInputConfig* CInputSourceEvaluator::GetConfig(void) { return &muConfig; }
float CInputSourceEvaluator::GetStateDeadzone(void) { return mfStateDeadzone; }
float CInputSourceEvaluator::GetDeltaSensitivity(void) { return mfDeltaSensitivity; }
SInputConfigSpecificSource* CInputSourceEvaluator::GetDeviceSource(void) { return &muConfig.sDeviceSource; }
SInputConfigGenericSource* CInputSourceEvaluator::GetDeviceGeneric(void) { return &muConfig.sDeviceGeneric; }
SInputConfigAnyToggleSource*  CInputSourceEvaluator::GetDeviceAny(void) { return &muConfig.sDeviceAny; }



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceEvaluator::ToString(CChars* psz)
{
	psz->Append(" --- CInputSourceEvaluator ---\n");
	psz->Append("Config Type: ");
	switch (meEvaluatorType)
	{
	case IET_Unknown:
		psz->Append("ICT_Unknown");
		break;
	case IET_SpecificSource:
		psz->Append("IET_SpecificSource");
		break;
	case IET_GenericSource:
		psz->Append("IET_GenericSource");
		break;
	case IET_AnyToggleSource:
		psz->Append("IET_AnyToggleSource");
		break;
	}
	psz->AppendNewLine();

	psz->Append("Action Type: ");
	switch (meChordType)
	{
	case ICT_Unknown:
		psz->Append("ICT_Unknown");
		break;	
	case ICT_Toggle:
		psz->Append("ICT_Toggle");
		break;	
	case ICT_Range:
		psz->Append("ICT_Range");
		break;
	}
	psz->AppendNewLine();

	//UInputConfig		muConfig;

	psz->Append("State Deadzone: ");
	psz->Append(mfStateDeadzone, 2);
	psz->AppendNewLine();

	psz->Append("Delta Sensitivity: ");
	psz->Append(mfDeltaSensitivity, 2);
	psz->AppendNewLine();
}
