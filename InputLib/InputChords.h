/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __INPUT_CHORDS_H__
#define __INPUT_CHORDS_H__
#include "StandardLib/SetType.h"
#include "StandardLib/ClassFunctionsCall.h"
#include "InputLib/InputListener.h"
#include "InputSourceEvaluator.h"
#include "InputChord.h"
#include "InputDeviceValueHistory.h"


#define MAX_INPUT_HISTORY_EVENTS	24


typedef CSetType<CInputSourceEvaluator>	CSetInputSourceEvaluators;
typedef CSetType<CInputChord>			CSetInputChords;


class CInputChordDesc;
struct SMatchingAction
{
	size	iIndex;
	size	iLength;
	bool	bPotentialMatch;
	bool	bTotalMatch;

};


struct SChordInputEvent
{
	CInputChord*		pcChord;
	CInputDeviceValue*	pcValue;
};


class CInputChords : public CInputListener
{
CONSTRUCTABLE(CInputChords); 
public:
	CSetInputSourceEvaluators	mscEvaluators;
	CSetInputChords				mlcChords;
	CArrayInputDeviceTimeValue	macHistory;
	uint32						muiEvictionAge;
	
	void 					Init(void);
	void 					Kill(void);
	void					Copy(CInputChords* pcSource, CInputDeviceCopyContext* pcContext);
	void					ToString(CChars* psz);
	void					DumpHistory(void);

	void 					InputEvent(CUnknown* pcSource, void* pvContext);
	CInputSourceEvaluator*	AddEvaluator(void);
	CInputChord*			AddChord(CAction* pcAction, CInputChordDesc* pcDesc);

	CInputChord*			StartChordsIteration(SSetIterator* psIter);
	CInputChord*			IterateChords(SSetIterator* psIter);
};


#endif // __INPUT_CHORDS_H__

