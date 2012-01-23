/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __INPUT_SOURCE_DESC_H__
#define __INPUT_SOURCE_DESC_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayString.h"
#include "StandardLib/Unknown.h"
#include "StandardLib/SetType.h"
#include "InputSourceValue.h"
#include "InputCategoryGeneric.h"


class CInputDeviceDesc;
class CInputDeviceState;
class CInputDeviceValues;
class CInputSourceDesc : public CUnknown
{
BASE_FUNCTIONS(CInputSourceDesc);
protected:
	EInputSourceType				meType;  //If a connection event is sent then fValue is 0.0f for disconection and 1.0f for connection.
	CChars							mszFriendlyName;
	CSetInputSourceValue			mlcValues;
	CInputDeviceDesc*				mpcDeviceDesc;
	float							mfRestValue;
	BOOL							mbEmitRestEvent;
	BOOL							mbHasRestValue;
	int								miStateIndex;
	CArrayInputCategoryGenericPtr	mapcGenerics;

public:
	void							Init(CInputDeviceDesc* pcDeviceDesc, EInputSourceType eType, char* szFriendlyName, int iStateIndex);
	void							Kill(void);
	void							Process(void* pvData, CInputDeviceState* pcState, CInputDeviceValues* pcEvents, unsigned int uiSequence);
	CInputSourceValue*				AddValue(int iValueChannel);
	CInputSourceValue*				AddValue(float fValueNumber);
	void							CopyValues(CInputSourceDesc* pcInputSourceDesc);
	void							CopyActions(CInputSourceDesc* pcInputSourceDesc);
	void							SetRest(float fRestValue, BOOL bEmitRestEvent, BOOL bHasRestValue);
	BOOL							Is(char* szFriendlyName);
	CInputSourceValue*				StartValuesIteration(SSetIterator* psIter);
	CInputSourceValue*				IterateValues(SSetIterator* psIter);
	float							GetRestValue(void);
	BOOL							HasRestValue(void);
	CInputDeviceDesc*				GetDeviceDesc(void);
	CArrayInputCategoryGenericPtr*	GetGenerics(void);
	EInputSourceType				GetType(void);
	int								GetStateIndex(void);
	char*							GetFriendlyName(void);
	void							SetFriendlyName(char* szFriendlyName);
	BOOL							GetEmitRestEvent(void);
	void							ToString(CChars* psz);
};


typedef CSetType<CInputSourceDesc> CSetInputSourceDesc;
typedef CArrayTemplate<CInputSourceDesc*> CArrayInputSourceDescPtr;


#endif // __INPUT_SOURCE_DESC_H__

