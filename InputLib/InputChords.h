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
	CInputDeviceValue*	pcValue;  //NULL for a chord called after its last input: a pending one, or one left after an eviction.
};


//What the chords let through, in order: an input that isn't (or is no longer) part of a chord, or a chord that matched
//in place of its inputs.
struct SInputChordsEvent
{
	CInputChord*			pcChord;  //NULL for an input.
	SInputDeviceTimeValue	sValue;  //The input, or the last input of the chord.
};


typedef CArrayTemplate<SInputChordsEvent>	CArrayInputChordsEvent;


//Matches the inputs of a virtual device against its chords.  An input that could be part of a chord is held in the
//history until a chord matches, using it up, or no chord can match, letting it through.  A chord must match within the
//eviction age of its first input or its held inputs are let through.  A chord that matches while a longer chord with
//the same start could still match, like ctrl+shift then a before ctrl+shift then a then b, is pending: it is called
//when the longer chord stops matching or is evicted, and not at all if the longer one matches.  Movement (a delta)
//that isn't part of a chord is let through straight away and doesn't stop a chord matching.  What is let through is
//in the events until the next update.
class CInputChords : public CInputListener
{
CONSTRUCTABLE(CInputChords);
public:
	CSetInputSourceEvaluators	mscEvaluators;
	CSetInputChords				mlcChords;
	CArrayInputDeviceTimeValue	macHistory;
	uint32						muiEvictionAge;  //Milliseconds.
	uint32						muiTime;  //Milliseconds, of the update the inputs are from.
	CArrayInputChordsEvent		masEvents;
	CInputChord*				mpcPending;  //A chord that matched, waiting to see if a longer one does.  NULL if none.
	size						muiPendingStart;  //The pending chord's first input in the history.
	size						muiPendingLength;  //How many inputs it uses up.

	void 					Init(void);
	void 					Kill(void);
	void					Copy(CInputChords* pcSource, CInputDeviceCopyContext* pcContext);
	void					ToString(CChars* psz);
	void					DumpHistory(void);

	void 					InputEvent(CUnknown* pcSource, void* pvContext);
	CInputSourceEvaluator*	AddEvaluator(void);
	CInputChord*			AddChord(CAction* pcAction, CInputChordDesc* pcDesc);
	CInputChord*			FindMatchingChord(CInputChord* pcChord);
	void					RemoveChord(CInputChord* pcChord);

	bool					StartChordsIteration(SSetIterator* psIter, CInputChord** ppChord);
	bool					IterateChords(SSetIterator* psIter, CInputChord** ppChord);

	void					BeginUpdate(uint32 uiTime);
	void					EndUpdate(void);
	CArrayInputChordsEvent*	GetEvents(void);
	bool					IsChordSource(SInputDeviceValue* psValue);

protected:
	void					Match(CInputDeviceValue* psValue);
	void					CallPending(void);
	void					Evict(void);
	void					Release(size uiNumHistory);
	void					AddEvent(CInputChord* pcChord, SInputDeviceTimeValue* psValue);
};


#endif // __INPUT_CHORDS_H__

