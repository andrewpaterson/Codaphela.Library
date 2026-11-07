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
#ifndef __INPUT_H__
#define __INPUT_H__
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "InputDevices.h"
#include "InputActions.h"
#include "ProgramInput.h"
#include "InputDeviceValue.h"
#include "NativeInput.h"


//The input devices and the actions their chords call.  It is an object: hold it from another (or the root).
class CInput : public CObject
{
CONSTRUCTABLE(CInput);
DESTRUCTABLE(CInput);
protected:
	Ptr<CInputDevices>	mpDevices;  //Nes: freed after this, so the actions (which the devices' chords call) are killed first.
	CInputActions		mcActions;
	uint32				muiSequence;  //Sequence is updated everytime CInput::Update is called.  Not every event.
	uint32				muiTime;  //Milliseconds, given to Update.  The chords time out partial matches by it.

	CNativeInput*		mpcNativeInput;
	CProgramInput		mcProgramInput;

public:
	void 				Init(CNativeInput* pcNativeInput);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	bool 				Read(const char* szDirectory = NULL);
	void 				AddProgramInput(void);
	bool				AddNativeInput(CNativeInput* pcNativeInput);

	void 				Update(void);
	void 				Update(uint32 uiTime);
	void 				Step(void);
	void 				ClearStateFlags(void);
	void 				UpdateRestEvents(void);
	void 				UpdateChords(void);
	void 				ClearEvents(void);
	void 				SortEvents(void);
	void 				UpdateVirtualDevicesEvents(void);
	void 				DumpEvents(void);
	CInputCategory*		GetCategory(const char* szName);

	CProgramInput*		GetProgramInput(void);
	CInputDevices*		GetDevices(void);
	CInputActions*		GetActions(void);
};


#endif // __INPUT_H__

