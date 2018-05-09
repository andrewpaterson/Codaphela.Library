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
#include "InputDevice.h"
#include "InputDeviceValue.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SInputDeviceValue::Init(CInputDevice* pcDevice, CInputSourceDesc* pcSource, float fValue)
{
	this->pcDevice = pcDevice;
	this->pcSource = pcSource;
	this->fValue = fValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SInputDeviceValue::ToString(CChars* psz)
{
	psz->Append(pcDevice->GetDesc()->GetFriendlyName());
	psz->Append(".");
	psz->Append(pcSource->GetFriendlyName());
	psz->Append(": ");
	psz->Append(fValue, 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceValue::Init(CInputDevice* pcDevice, CInputSourceDesc* pcSource, float fValue, unsigned int uiSequence, BOOL bRestEmitted, int iOrder)
{
	SInputDeviceValue::Init(pcDevice, pcSource, fValue);
	this->uiSequence = uiSequence;
	this->iOrder = iOrder;
	this->bRestEmitted = bRestEmitted;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceValues::Init(void)
{
	mlcValues.Init(INPUT_VALUES_CHUNK_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceValues::Kill(void)
{
	mlcValues.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceValues::Add(CInputDevice* pcDevice, CInputSourceDesc* pcSource, float fValue, unsigned int uiSequence, BOOL bRestEmitted, int iOrder)
{
	CInputDeviceValue*	psInputEvent;

	psInputEvent = mlcValues.Add();
	psInputEvent->Init(pcDevice, pcSource, fValue, uiSequence, bRestEmitted, iOrder);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceValues::Clear(void)
{
	mlcValues.Kill();
	mlcValues.Init(INPUT_VALUES_CHUNK_SIZE);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SortInputEventFunc(const void* pv1, const void* pv2)
{
	return SortInputEventPtrFunc(&pv1, &pv2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SortInputEventPtrFunc(const void* pv1, const void* pv2)
{
	CInputDeviceValue*	psEvent1;
	CInputDeviceValue*	psEvent2;

	psEvent1 = *((CInputDeviceValue**)pv1);
	psEvent2 = *((CInputDeviceValue**)pv2);

	//Might be backwards.
	if (psEvent1->uiSequence < psEvent2->uiSequence)
	{
		return -1;
	}
	if (psEvent1->uiSequence > psEvent2->uiSequence)
	{
		return 1;
	}
	
	if (psEvent1->iOrder < psEvent2->iOrder)
	{
		return -1;
	}
	if (psEvent1->iOrder > psEvent2->iOrder)
	{
		return 1;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceValues::Sort(void)
{
	mlcValues.BubbleSort(&SortInputEventFunc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceValue* CInputDeviceValues::StartIteration(void)
{
	return mlcValues.GetHead();;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceValue* CInputDeviceValues::Iterate(CInputDeviceValue* pcInputDeviceValue)
{
	return mlcValues.GetNext(pcInputDeviceValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceValues::ToString(CChars* psz)
{
	CInputDeviceValue*	pcValue;

	pcValue = mlcValues.GetHead();
	while (pcValue)
	{
		pcValue->ToString(psz);
		psz->AppendNewLine();
		pcValue = mlcValues.GetNext(pcValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceValues::Dump(void)
{
	CChars	sz;

	sz.Init();
	ToString(&sz);
	sz.Dump();
	sz.Kill();
}

