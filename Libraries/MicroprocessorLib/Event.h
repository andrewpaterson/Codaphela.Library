/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __EVENT_H__
#define __EVENT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"


//Something that happens at a time on a CTimeline, like Logicim's Event.  Subclasses do the work in Execute.
//
//An event is made with NewMalloc, given its time with Init and added to a timeline.  The timeline owns it from then
//on: it is killed and freed once it has executed or been removed.  An owner that keeps a pointer to its events, as an
//integrated circuit keeps its pending events, drops the pointer in RemoveFromOwner, which is called when the event
//executes and when it is removed.  If adding the event fails the caller still owns it and must kill and free it.
//
//Every event gets the next id when it is initialised, unless it is given one, as when it is loaded.
class CTimeline;
class CIntegratedCircuit;
class CEvent : public CConstructable
{
CONSTRUCTABLE(CEvent);
protected:
	int64	miTime;
	int64	miID;
	bool	mbRemoved;
	bool	mbExecuted;

public:
			void	Init(int64 iTime);
			void	Init(int64 iTime, int64 iID);
	virtual void	Kill(void);

	virtual void	Execute(CTimeline* pcTimeline);
	virtual void	RemoveFromOwner(void);

			bool	IsRemoved(void);
			bool	IsExecuted(void);
			int64	GetTime(void);
			int64	GetID(void);

	virtual void	PrintShort(CChars* psz);
	virtual CIntegratedCircuit*	GetIntegratedCircuit(void);

	static	void	ResetNextID(void);
};


typedef CArrayTemplatePtr<CEvent>	CArrayEventPtr;


#endif // __EVENT_H__

