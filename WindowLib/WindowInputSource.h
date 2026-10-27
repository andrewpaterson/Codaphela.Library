/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __WINDOW_INPUT_SOURCE_H__
#define __WINDOW_INPUT_SOURCE_H__
#include "BaseLib/Timer.h"
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/Object.h"
#include "StandardLib/Listener.h"
#include "InputLib/Input.h"


enum EWindowInputEventType
{
	WIET_State,	//A key or button, or a controller's axis or button, changed.
	WIET_Delta,	//Movement, like the mouse's.  Never zero.
	WIET_Chord,	//A chord matched.  Its inputs aren't queued.
};


struct SWindowInputEvent
{
	EWindowInputEventType	eType;
	CInputDevice*			pcDevice;
	CInputSourceDesc*		pcSource;  //Of a chord, its last input.
	float					fValue;
	uint32					uiTime;  //Milliseconds since the source was made.
	char*					szChord;  //The chord's name, NULL if not a chord.
};


typedef CArrayTemplate<SWindowInputEvent>	CArrayWindowInputEvent;


//A chord's action calls its listeners when it matches, though the queue is the way to hear of it.
class CWindowInputChordListener : public CListener
{
CONSTRUCTABLE(CWindowInputChordListener);
public:
	virtual void Chord(CUnknown* pcSource, void* pvContext) {}
};


//The only part of WindowLib that uses InputLib.  Every keyboard, mouse, joystick and gamepad's sources are in one
//virtual device so a chord can be made of any of them; AddDevice adds a device in another category.  Update updates the input and queues what changed: keys and
//buttons, movement other than none and controllers.  An input that can be part of a chord is only queued once the
//chord matches (then only the chord is queued) or stops matching or 0.3 seconds pass.
//
//The queue holds what the last Update found until the next one.  Kill the CInput after the source.
class CWindowInputSource : public CObject
{
CONSTRUCTABLE(CWindowInputSource);
DESTRUCTABLE(CWindowInputSource);
protected:
	CInput*					mpcInput;
	CInputVirtualDevice*	mpcVirtual;
	CArrayInputDevicePtr	mapcDevices;  //Whose sources are in the virtual device.
	CTimer					mcTimer;
	CArrayWindowInputEvent	masEvents;

public:
	void					Init(CInput* pcInput);
	void					Class(void) override;
	void					Free(void) override;
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	void					Update(void);
	size					NumEvents(void);
	SWindowInputEvent*		GetEvent(size uiIndex);

	bool					AddDevice(CInputDevice* pcDevice);
	size					GetDevices(CArrayInputDevicePtr* papcDevices, char* szCategory);
	CInputSourceDesc*		GetSource(CInputDevice* pcDevice, char* szFriendlyName);
	CInputChord*			AddChord(char* szName);
	CInputVirtualDevice*	GetVirtualDevice(void);

protected:
	void					AddDevices(char* szCategory);
	void					Queue(SInputChordsEvent* psChordsEvent);
};


#endif // __WINDOW_INPUT_SOURCE_H__
