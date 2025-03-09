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
#ifndef __INPUT_ACTION_CRITERIA_H__
#define __INPUT_ACTION_CRITERIA_H__
#include "InputSourceEvaluator.h"
#include "InputDeviceValueHistory.h"


//Seriously, if you've got more than 8 combinations in a group you need your head checked.
#define MAX_GROUP_SIZE	8


enum EBasicActionActive
{
	BAA_Unknown = 0,
	BAA_Active,
	BAA_Inactive,
	BAA_Group,
	BAA_Ordered,
};


class CInputChordCriteria
{
public:
	EBasicActionActive	eAction;
};


class CActionInputChordCriteria : public CInputChordCriteria
{
public:
	CInputSourceEvaluator*	mpcEvaluator;

	void SetEvaluator(CInputSourceEvaluator* pcEvaluator);
};


typedef CArrayTemplateMinimal<CActionInputChordCriteria> CArrayActionInputChordCriteria;


class CActiveInputChordCriteria : public CActionInputChordCriteria
{
public:
	void Init(CInputSourceEvaluator* pcEvaluator);
	void Kill(void);
	void ToArray(CArrayActionInputChordCriteria* pcDest);
	size Size(void);
	void ToString(CChars* psz);
};


class CInactiveInputChordCriteria : public CActionInputChordCriteria
{
public:
	void Init(CInputSourceEvaluator* pcEvaluator);
	void Kill(void);
	void ToArray(CArrayActionInputChordCriteria* pcDest);
	size Size(void);
	void ToString(CChars* psz);
};


class CGroupInputChordCriteria : public CInputChordCriteria
{
public:
	CArrayBlock		mausBasicActionCriteria;  //Array of UInputChordCriteria

	void Init(void);
	void Kill(void);
	void ToArray(CArrayActionInputChordCriteria* pcDest);
	size Size(void);
	void ToString(CChars* psz);

	CActiveInputChordCriteria*		AddActiveAction(CInputSourceEvaluator* pcAction);
	CInactiveInputChordCriteria*	AddInactiveAction(CInputSourceEvaluator* pcAction);
};


class COrderedInputChordCriteria : public CInputChordCriteria
{
public:
	CArrayBlock		mausBasicActionCriteria;  //Array of UInputChordCriteria

	void Init(void);
	void Kill(void);
	size Size(void);
	size Size(size iLastIndexInclusive);
	void ToString(CChars* psz);

	CActiveInputChordCriteria*		AddActiveAction(CInputSourceEvaluator* pcAction);
	CInactiveInputChordCriteria*	AddInactiveAction(CInputSourceEvaluator* pcAction);
	CGroupInputChordCriteria*		AddGroupAction(void);
	void							RemoveLastAction(void);
};


union UInputChordCriteria
{
	CInputChordCriteria				eType;
	CActiveInputChordCriteria		cActive;
	CInactiveInputChordCriteria		cInactive;
	CGroupInputChordCriteria		cGroup;
	COrderedInputChordCriteria		cOrdered;
};


void KillActionCriteria(UInputChordCriteria* puCriteria);
void ToStringActionCriteria(UInputChordCriteria* puCriteria, CChars* psz);


#endif // !__INPUT_ACTION_CRITERIA_H__

