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
#ifndef __INPUT_ACTION_H__
#define __INPUT_ACTION_H__
#include "BaseLib/Chars.h"
#include "StandardLib/Object.h"
#include "StandardLib/MemberCall.h"
#include "ComponentInputEvent.h"


//Which inputs an action is called for.  Keys have no modifiers; a key with a modifier held is a chord.
enum EInputActionFilter
{
	IAF_AnyKeyDown,
	IAF_AnyKeyUp,
	IAF_KeyDown,	//The key or button named, like "A" or "Button 1".
	IAF_KeyUp,
	IAF_AnyChord,
	IAF_Chord,		//The chord named.
	IAF_Printable,
	IAF_AnyCommand,
	IAF_Command,	//The command given.
	IAF_Movement,
	IAF_Wheel,
};


///////////////////////////////////////////////////////////////////////////
//
// A thunk for a command's function that only needs the pointer, like a
// button's ActivateComponent.  pvArgument is the event.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void InputActionCallPointer(void* pvObject, uint8* pucFunction, void* pvArgument, void* pvValue)
{
	void(M::*fFunction)(int x, int y);
	SComponentInputEvent*	psEvent;

	memcpy(&fFunction, pucFunction, sizeof(fFunction));
	psEvent = (SComponentInputEvent*)pvArgument;
	(((M*)pvObject)->*fFunction)(psEvent->x, psEvent->y);
}


//An input a component is listening for and the member function called with it (see SMemberCall).  The function's
//object isn't held by the action (it is usually the component the action is on) so it must outlive the action.
//
//A command's function can take the event, nothing, or the pointer's x and y relative to the component; the others'
//take the event.
class CInputAction : public CObject
{
CONSTRUCTABLE(CInputAction);
DESTRUCTABLE(CInputAction);
protected:
	EInputActionFilter		meFilter;
	CChars					mszName;
	EComponentInputCommand	meCommand;
	SMemberCall				msCall;
	bool					mbEnabled;  //A disabled action matches nothing, so its input goes on to the parent.

public:
	template<class M, class O>
	void					Init(EInputActionFilter eFilter, const char* szName, O* pcObject, void(M::*fFunction)(SComponentInputEvent*));
	template<class M, class O>
	void					Init(EComponentInputCommand eCommand, O* pcObject, void(M::*fFunction)(SComponentInputEvent*));
	template<class M, class O>
	void					Init(EComponentInputCommand eCommand, O* pcObject, void(M::*fFunction)(void));
	template<class M, class O>
	void					Init(EComponentInputCommand eCommand, O* pcObject, void(M::*fFunction)(int x, int y));
	void					Class(void) override;
	void					Free(void) override;
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	bool					Matches(SComponentInputEvent* psEvent);
	void					Call(SComponentInputEvent* psEvent);
	void					SetEnabled(bool bEnabled);
	bool					IsEnabled(void);

	EInputActionFilter		GetFilter(void);
	char*					GetActionName(void);
	EComponentInputCommand	GetCommand(void);

protected:
	void					Init(EInputActionFilter eFilter, const char* szName, EComponentInputCommand eCommand, SMemberCall* psCall);
};


//////////////////////////////////////////////////////////////////////////
//
// szName is the key or chord for IAF_KeyDown, IAF_KeyUp and IAF_Chord,
// otherwise NULL.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CInputAction::Init(EInputActionFilter eFilter, const char* szName, O* pcObject, void(M::*fFunction)(SComponentInputEvent*))
{
	SMemberCall	sCall;

	sCall.Init(pcObject, fFunction);
	Init(eFilter, szName, CIC_Ignored, &sCall);
}


//////////////////////////////////////////////////////////////////////////
//
// An IAF_Command action.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CInputAction::Init(EComponentInputCommand eCommand, O* pcObject, void(M::*fFunction)(SComponentInputEvent*))
{
	SMemberCall	sCall;

	sCall.Init(pcObject, fFunction);
	Init(IAF_Command, NULL, eCommand, &sCall);
}


template<class M, class O>
void CInputAction::Init(EComponentInputCommand eCommand, O* pcObject, void(M::*fFunction)(void))
{
	SMemberCall	sCall;

	sCall.Init(pcObject, fFunction);
	Init(IAF_Command, NULL, eCommand, &sCall);
}


template<class M, class O>
void CInputAction::Init(EComponentInputCommand eCommand, O* pcObject, void(M::*fFunction)(int x, int y))
{
	SMemberCall	sCall;

	sCall.Init<M>(pcObject, fFunction, &InputActionCallPointer<M>);
	Init(IAF_Command, NULL, eCommand, &sCall);
}


#endif // __INPUT_ACTION_H__
