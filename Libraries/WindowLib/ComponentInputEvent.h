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
#ifndef __COMPONENT_INPUT_EVENT_H__
#define __COMPONENT_INPUT_EVENT_H__
#include "BaseLib/PrimitiveTypes.h"
#include "ComponentInputCommand.h"


enum EComponentInputEventType
{
	CIET_KeyDown,	//A key or button went down.  szName is its source.
	CIET_KeyUp,
	CIET_Chord,		//A chord matched.  szName is the chord.
	CIET_Printable,	//A key typed cPrintable.
	CIET_Command,	//A key or chord means eCommand.
	CIET_Movement,	//The pointer moved by iRight and iDown.
	CIET_Wheel,		//The wheel turned by iRight and iDown notches.
};


//An input as a component's input actions are given it, translated by the window's focus from the queued
//SWindowInputEvent.  Nothing in it is InputLib's but the raw event.
struct SWindowInputEvent;
struct SComponentInputEvent
{
	EComponentInputEventType	eType;
	char*						szName;		//The key's or chord's name, NULL for the other types.
	float						fValue;		//The key's value, 1 down and 0 up for a key or button.
	char						cPrintable;
	EComponentInputCommand		eCommand;
	bool						bInsert;	//Typing inserts rather than overwrites.
	int							x;			//The pointer, relative to the top left of the component the action is on.
	int							y;
	int							iRight;		//Of a movement or the wheel.
	int							iDown;
	bool						bShift;		//Shift is held; shift and caps lock are only used for printables.
	bool						bCapsLock;
	bool						bNumLock;
	uint32						uiTime;		//Milliseconds, from the input source.
	SWindowInputEvent*			psWindowEvent;	//What was queued, NULL if nothing was.
	bool						bPass;		//Set by an action that couldn't use the event, so it goes on to the parent.
};


#endif // __COMPONENT_INPUT_EVENT_H__
