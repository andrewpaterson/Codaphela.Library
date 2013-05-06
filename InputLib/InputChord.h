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
#ifndef __INPUT_ACTION_H__
#define __INPUT_ACTION_H__
#include "BaseLib/MapPtrPtr.h"
#include "StandardLib/Action.h"
#include "InputCategoryGeneric.h"
#include "InputDeviceCopyContext.h"
#include "InputChordCriteria.h"


struct SMatchResult
{
	int		iIndex;
	int		iLength;
};


class CInputChords;
class CInputChordDesc;
class CInputChord : public CUnknown
{
BASE_FUNCTIONS(CInputChord);
protected:
	UInputChordCriteria				muActionCriteria;
	int								miTotalCriteria;
	int								miUpdateCriteria;
	CArrayActionInputChordCriteria	masStartDeviceValue;
	CArrayActionInputChordCriteria	masScratchPadDeviceValue;  //Shouldn't this live elsewhere?
	CAction*						mpcAction;
	CInputChords*					mpcInputChords;
	CInputChordDesc*				mpcDesc;  //Optional.

	int								miMatchedCriteria;  //How many criteria match the input stream.

public:
	void 							Init(CAction* pcAction, CInputChords* pcInputChords, CInputChordDesc* pcDesc);
	void 							Kill(void);
	void							Done(void);  //Remember to call this after adding all criteria.
	void							Copy(CInputChord* pcSource, CInputDeviceCopyContext* pcContext, CInputChords* pcInputChords);

	CActiveInputChordCriteria*		AsActive(CInputSourceEvaluator* pcAction);
	CInactiveInputChordCriteria*	AsInactive(CInputSourceEvaluator* pcAction);
	CGroupInputChordCriteria*		AsGroup(void);
	COrderedInputChordCriteria*		AsOrdered(void);

	void							CalculateTotalCriteria(void);
	void							CalculatePotentialStart(void);
	void							CalculateLongestAction(void);
	SMatchResult					Match(CArrayInputDeviceTimeValue* pcHistory);
	int								FindMatchingStart(CArrayInputDeviceTimeValue* pcHistory, int iStartPos);

	CInputSourceEvaluator*			AddEvaluatorSpecificSource(CInputVirtualDeviceSource* pcSource, EInputChordType eType);
	CInputSourceEvaluator*			AddEvaluatorSpecificSource(CInputDevice* pcDevice, CInputSourceDesc* pcSource, EInputChordType eType);
	CInputSourceEvaluator*			AddEvaluatorGenericSource(CInputCategoryGeneric* pcGeneric, EInputChordType eType);
	CInputSourceEvaluator*			AddEvaluatorAnyToggleSource(void);

	void							Call(void* pvContext);

	CInputChordDesc*				GetDesc(void);
	UInputChordCriteria*			GetActionCriteria(void);
	int								GetTotalCriteria(void);
	int								GetUpdateCriteria(void);
	int								GetMatchedCriteria(void);
	void							SetMatchedCriteria(int iMatchedCriteria);
	int								MatchActionEvent(CArrayInputDeviceTimeValue* pcHistory, int iStartPos);
	void							ToString(CChars* psz);
};


#endif // __INPUT_ACTION_H__

