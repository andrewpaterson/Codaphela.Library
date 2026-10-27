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


//Holds a member function pointer whatever its class's inheritance; MSVC's are up to 24 bytes.
#define INPUT_ACTION_FUNCTION_SIZE	32


typedef void (*InputActionThunk)(void* pvObject, uint8* pucFunction, SComponentInputEvent* psEvent);


//////////////////////////////////////////////////////////////////////////
//
// Calls the member function of M kept in pucFunction on pvObject.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void InputActionCall(void* pvObject, uint8* pucFunction, SComponentInputEvent* psEvent)
{
	void(M::*fFunction)(SComponentInputEvent*);

	memcpy(&fFunction, pucFunction, sizeof(fFunction));
	(((M*)pvObject)->*fFunction)(psEvent);
}


//An input a component is listening for and the member function called with it.  The function's object isn't held by
//the action (it is usually the component the action is on) so it must outlive the action.
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

public:
	template<class M>
	void					Init(EInputActionFilter eFilter, const char* szName, M* pcObject, void(M::*fFunction)(SComponentInputEvent*));
	template<class M>
	void					Init(EComponentInputCommand eCommand, M* pcObject, void(M::*fFunction)(SComponentInputEvent*));
	void					Class(void) override;
	void					Free(void) override;
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	bool					Matches(SComponentInputEvent* psEvent);
	void					Call(SComponentInputEvent* psEvent);

	EInputActionFilter		GetFilter(void);
	char*					GetName(void);
	EComponentInputCommand	GetCommand(void);

protected:
	void					Init(EInputActionFilter eFilter, const char* szName, EComponentInputCommand eCommand, void* pvObject, InputActionThunk fThunk, void* pvFunction, size uiFunctionSize);
};


//////////////////////////////////////////////////////////////////////////
//
// szName is the key or chord for IAF_KeyDown, IAF_KeyUp and IAF_Chord,
// otherwise NULL.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CInputAction::Init(EInputActionFilter eFilter, const char* szName, M* pcObject, void(M::*fFunction)(SComponentInputEvent*))
{
	static_assert(sizeof(fFunction) <= INPUT_ACTION_FUNCTION_SIZE, "INPUT_ACTION_FUNCTION_SIZE is too small for this member function pointer.");

	Init(eFilter, szName, CIC_Ignored, pcObject, &InputActionCall<M>, &fFunction, sizeof(fFunction));
}


//////////////////////////////////////////////////////////////////////////
//
// An IAF_Command action.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CInputAction::Init(EComponentInputCommand eCommand, M* pcObject, void(M::*fFunction)(SComponentInputEvent*))
{
	static_assert(sizeof(fFunction) <= INPUT_ACTION_FUNCTION_SIZE, "INPUT_ACTION_FUNCTION_SIZE is too small for this member function pointer.");

	Init(IAF_Command, NULL, eCommand, pcObject, &InputActionCall<M>, &fFunction, sizeof(fFunction));
}


#endif // __INPUT_ACTION_H__
