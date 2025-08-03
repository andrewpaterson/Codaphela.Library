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
#include "Input.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::Init(CNativeInput* pcNativeInput)
{
	muiSequence = 0;
	mcDevices.Init(&mcActions);
	mpcNativeInput = pcNativeInput;
	mcProgramInput.Init();
	mcActions.Init(&mcDevices);

	if (mpcNativeInput)
	{
		mpcNativeInput->AddDevicesTo(&mcDevices);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::Kill(void)
{
	mcActions.Kill();
	mcDevices.Kill();
	mcProgramInput.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInput::Read(char* szDirectory)
{
	if (!szDirectory)
	{
		szDirectory = "../../CommonLibraries/InputLib/xml";
	}
	return mcDevices.Read(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::AddProgramInput(void)
{
	mcProgramInput.AddFormat(&mcDevices);
	mcProgramInput.AddCategory(&mcDevices);
	mcProgramInput.AddDevicesTo(&mcDevices);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::Step(void)
{
	muiSequence++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::Update(void)
{
	ClearStateFlags();
	ClearEvents();
	if (mpcNativeInput)
	{
		mpcNativeInput->Update(&mcDevices, muiSequence);
	}
	mcProgramInput.Update(&mcDevices, muiSequence);
	UpdateRestEvents();
	SortEvents();

	mcDevices.UpdateVirtualDevicesEvents();

	CallListeners();
	Step();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::ClearEvents(void)
{
	CInputDevice*	pcInputDevice;
	SSetIterator	sIter;

	pcInputDevice = mcDevices.mlcDevices.StartIteration(&sIter);
	while (pcInputDevice)
	{
		pcInputDevice->ClearEvents();
		pcInputDevice = mcDevices.mlcDevices.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::SortEvents(void)
{
	CInputDevice*	pcInputDevice;
	SSetIterator	sIter;

	pcInputDevice = mcDevices.mlcDevices.StartIteration(&sIter);
	while (pcInputDevice)
	{
		pcInputDevice->SortEvents();
		pcInputDevice = mcDevices.mlcDevices.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::DumpEvents()
{
	CInputDevice*	pcInputDevice;
	SSetIterator	sIter;

	pcInputDevice = mcDevices.mlcDevices.StartIteration(&sIter);
	while (pcInputDevice)
	{
		pcInputDevice->DumpEvents();
		pcInputDevice = mcDevices.mlcDevices.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::ClearStateFlags(void)
{
	CInputDevice*	pcInputDevice;
	SSetIterator	sIter;

	pcInputDevice = mcDevices.mlcDevices.StartIteration(&sIter);
	while (pcInputDevice)
	{
		pcInputDevice->ClearStateFlags();
		pcInputDevice = mcDevices.mlcDevices.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::UpdateRestEvents(void)
{
	CInputDevice*	pcInputDevice;
	SSetIterator	sIter;

	pcInputDevice = mcDevices.mlcDevices.StartIteration(&sIter);
	while (pcInputDevice)
	{
		pcInputDevice->UpdateRestEvents(muiSequence);
		pcInputDevice = mcDevices.mlcDevices.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::CallListeners(void)
{
	CInputVirtualDevice*	pcVirtual;
	SSetIterator			sIter;

	pcVirtual = mcDevices.mlcVirtuals.StartIteration(&sIter);
	while (pcVirtual)
	{
		pcVirtual->CallListeners();
		pcVirtual = mcDevices.mlcVirtuals.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputCategory* CInput::GetCategory(char* szName)
{
	return mcDevices.GetCategory(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CProgramInput* CInput::GetProgramInput(void) { return &mcProgramInput; }
CInputDevices* CInput::GetDevices(void) { return &mcDevices; }
CInputActions* CInput::GetActions(void) { return &mcActions; }

