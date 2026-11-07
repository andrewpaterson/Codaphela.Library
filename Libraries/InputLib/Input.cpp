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
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "Input.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::Init(CNativeInput* pcNativeInput)
{
	PreInit();

	muiSequence = 0;
	muiTime = 0;
	mpDevices = OMalloc<CInputDevices>(&mcActions);
	mpcNativeInput = pcNativeInput;
	mcProgramInput.Init();
	mcActions.Init(&mpDevices);

	if (mpcNativeInput)
	{
		mpcNativeInput->AddDevicesTo(&mpDevices);
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::Class(void)
{
	Nes(M_Pointer(mpDevices));
	U_Unknown(CInputActions, mcActions);
	U_UInt32(muiSequence);
	U_UInt32(muiTime);
	U_Pointer(mpcNativeInput);
	U_Unknown(CProgramInput, mcProgramInput);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInput::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInput::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::Free(void)
{
	//The devices are freed after this (they are Nes).
	mcActions.Kill();
	mcProgramInput.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInput::Read(const char* szDirectory)
{
	if (!szDirectory)
	{
		szDirectory = "../../CommonLibraries/Libraries/InputLib/xml";
	}
	return mpDevices->Read(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::AddProgramInput(void)
{
	mcProgramInput.AddFormat(&mpDevices);
	mcProgramInput.AddCategory(&mpDevices);
	mcProgramInput.AddDevicesTo(&mpDevices);
}


//////////////////////////////////////////////////////////////////////////
//
// For native input whose devices are described in the XML given to Read.
// Init(pcNativeInput) adds the devices before Read can be called.
//
//////////////////////////////////////////////////////////////////////////
bool CInput::AddNativeInput(CNativeInput* pcNativeInput)
{
	if (mpcNativeInput)
	{
		return gcLogger.Error2(__METHOD__, " Native input is already set.", NULL);
	}

	mpcNativeInput = pcNativeInput;
	mpcNativeInput->AddDevicesTo(&mpDevices);
	return true;
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
		mpcNativeInput->Update(&mpDevices, muiSequence);
	}
	mcProgramInput.Update(&mpDevices, muiSequence);
	UpdateRestEvents();
	SortEvents();

	mpDevices->UpdateVirtualDevicesEvents();

	UpdateChords();
	Step();
}


//////////////////////////////////////////////////////////////////////////
//
// uiTime is milliseconds from any start, it only needs to go forward.
// Update without it keeps the last time, so partial chords never age.
//
//////////////////////////////////////////////////////////////////////////
void CInput::Update(uint32 uiTime)
{
	muiTime = uiTime;
	Update();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInput::ClearEvents(void)
{
	CInputDevice*	pcInputDevice;
	SSetIterator	sIter;
	bool			bExists;

	bExists = mpDevices->mlcDevices.StartIteration(&sIter, &pcInputDevice);
	while (bExists)
	{
		pcInputDevice->ClearEvents();
		bExists = mpDevices->mlcDevices.Iterate(&sIter, &pcInputDevice);
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
	bool			bExists;

	bExists = mpDevices->mlcDevices.StartIteration(&sIter, &pcInputDevice);
	while (bExists)
	{
		pcInputDevice->SortEvents();
		bExists = mpDevices->mlcDevices.Iterate(&sIter, &pcInputDevice);
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
	bool			bExists;

	bExists = mpDevices->mlcDevices.StartIteration(&sIter, &pcInputDevice);
	while (bExists)
	{
		pcInputDevice->DumpEvents();
		bExists = mpDevices->mlcDevices.Iterate(&sIter, &pcInputDevice);
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
	bool			bExists;

	bExists = mpDevices->mlcDevices.StartIteration(&sIter, &pcInputDevice);
	while (bExists)
	{
		pcInputDevice->ClearStateFlags();
		bExists = mpDevices->mlcDevices.Iterate(&sIter, &pcInputDevice);
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
	bool			bExists;

	bExists = mpDevices->mlcDevices.StartIteration(&sIter, &pcInputDevice);
	while (bExists)
	{
		pcInputDevice->UpdateRestEvents(muiSequence);
		bExists = mpDevices->mlcDevices.Iterate(&sIter, &pcInputDevice);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gives each virtual device's chords the inputs its devices have just
// given.
//
//////////////////////////////////////////////////////////////////////////
void CInput::UpdateChords(void)
{
	CInputVirtualDevice*	pcVirtual;
	SSetIterator			sIter;
	bool					bExists;

	bExists = mpDevices->mlcVirtuals.StartIteration(&sIter, &pcVirtual);
	while (bExists)
	{
		pcVirtual->GetChords()->BeginUpdate(muiTime);
		pcVirtual->UpdateChords();
		pcVirtual->GetChords()->EndUpdate();
		bExists = mpDevices->mlcVirtuals.Iterate(&sIter, &pcVirtual);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputCategory* CInput::GetCategory(const char* szName)
{
	return mpDevices->GetCategory(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CProgramInput* CInput::GetProgramInput(void) { return &mcProgramInput; }
CInputDevices* CInput::GetDevices(void) { return &mpDevices; }
CInputActions* CInput::GetActions(void) { return &mcActions; }

