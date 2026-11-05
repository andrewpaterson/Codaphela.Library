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


///Holds a member function pointer whatever its class's inheritance; MSVC's are up to 24 bytes.
#define INPUT_ACTION_FUNCTION_SIZE	32


typedef void (*InputActionThunk)(void* pvObject, uint8* pucFunction, SComponentInputEvent* psEvent);


//////////////////////////////////////////////////////////////////////////
//
// Calls the member function of M kept in pucFunction on pvObject, with
// the event.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void InputActionCall(void* pvObject, uint8* pucFunction, SComponentInputEvent* psEvent)
{
	void(M::*fFunction)(SComponentInputEvent*);

	memcpy(&fFunction, pucFunction, sizeof(fFunction));
	(((M*)pvObject)->*fFunction)(psEvent);
}


//////////////////////////////////////////////////////////////////////////
//
// As InputActionCall, for a function that needs nothing from the event,
// like a text area's Left.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void InputActionCallPlain(void* pvObject, uint8* pucFunction, SComponentInputEvent* psEvent)
{
	void(M::*fFunction)(void);

	memcpy(&fFunction, pucFunction, sizeof(fFunction));
	(((M*)pvObject)->*fFunction)();
}


//////////////////////////////////////////////////////////////////////////
//
// As InputActionCall, for a function that only needs the pointer, like a
// button's ActivateComponent.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void InputActionCallPointer(void* pvObject, uint8* pucFunction, SComponentInputEvent* psEvent)
{
	void(M::*fFunction)(int x, int y);

	memcpy(&fFunction, pucFunction, sizeof(fFunction));
	(((M*)pvObject)->*fFunction)(psEvent->x, psEvent->y);
}


//An input a component is listening for and the member function called with it.  The function's object isn't held by
//the action (it is usually the component the action is on) so it must outlive the action.
//
//A command's function can take the event, nothing, or the pointer's x and y relative to the component; the others'
//take the event.  The object can be of a class derived from the function's, as when a component adds an action for a
//function it inherits.
//
//The member function pointer is kept as bytes with a function that knows its class; casting it to some other class's
//member pointer would lose the parts multiple inheritance adds.
class CInputAction : public CObject
{
CONSTRUCTABLE(CInputAction);
DESTRUCTABLE(CInputAction);
protected:
	EInputActionFilter		meFilter;
	CChars					mszName;
	EComponentInputCommand	meCommand;
	void*					mpvObject;
	InputActionThunk		mfThunk;
	uint8					maucFunction[INPUT_ACTION_FUNCTION_SIZE];
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
	template<class M, class F>
	void					Init(EInputActionFilter eFilter, const char* szName, EComponentInputCommand eCommand, M* pcObject, F fFunction, InputActionThunk fThunk);
	void					Init(EInputActionFilter eFilter, const char* szName, EComponentInputCommand eCommand, void* pvObject, InputActionThunk fThunk, void* pvFunction, size uiFunctionSize);
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
	Init<M>(eFilter, szName, CIC_Ignored, pcObject, fFunction, &InputActionCall<M>);
}


//////////////////////////////////////////////////////////////////////////
//
// An IAF_Command action.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CInputAction::Init(EComponentInputCommand eCommand, O* pcObject, void(M::*fFunction)(SComponentInputEvent*))
{
	Init<M>(IAF_Command, NULL, eCommand, pcObject, fFunction, &InputActionCall<M>);
}


template<class M, class O>
void CInputAction::Init(EComponentInputCommand eCommand, O* pcObject, void(M::*fFunction)(void))
{
	Init<M>(IAF_Command, NULL, eCommand, pcObject, fFunction, &InputActionCallPlain<M>);
}


template<class M, class O>
void CInputAction::Init(EComponentInputCommand eCommand, O* pcObject, void(M::*fFunction)(int x, int y))
{
	Init<M>(IAF_Command, NULL, eCommand, pcObject, fFunction, &InputActionCallPointer<M>);
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject is already the function's class, so the thunk can cast the
// kept void* straight back to it.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class F>
void CInputAction::Init(EInputActionFilter eFilter, const char* szName, EComponentInputCommand eCommand, M* pcObject, F fFunction, InputActionThunk fThunk)
{
	static_assert(sizeof(F) <= INPUT_ACTION_FUNCTION_SIZE, "INPUT_ACTION_FUNCTION_SIZE is too small for this member function pointer.");

	Init(eFilter, szName, eCommand, (void*)pcObject, fThunk, &fFunction, sizeof(F));
}


#endif // __INPUT_ACTION_H__
