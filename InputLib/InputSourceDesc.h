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
#include "BaseLib/ArrayChars.h"
#include "StandardLib/Unknown.h"
#include "StandardLib/SetType.h"
#include "InputSourceValue.h"
#include "InputCategoryGeneric.h"


class CInputDeviceDesc;
class CInputDeviceState;
class CInputDeviceValues;
class CInputSourceDesc : public CUnknown
{
CONSTRUCTABLE(CInputSourceDesc);
protected:
	EInputSourceType				meType;  //If a connection event is sent then fValue is 0.0f for disconection and 1.0f for connection.
	CChars							mszFriendlyName;
	CSetInputSourceValue			mlcValues;
	CInputDeviceDesc*				mpcDeviceDesc;
	float							mfRestValue;
	bool							mbEmitRestEvent;
	bool							mbHasRestValue;
	size								miStateIndex;
	CArrayInputCategoryGenericPtr	mapcGenerics;

public:
	void							Init(CInputDeviceDesc* pcDeviceDesc, EInputSourceType eType, char* szFriendlyName, size iStateIndex);
	void							Kill(void);
	void							Process(void* pvData, CInputDeviceState* pcState, CInputDeviceValues* pcEvents, size uiSequence);
	CInputSourceValue*				AddValue(size iValueChannel);
	CInputSourceValue*				AddValue(float fValueNumber);
	void							CopyValues(CInputSourceDesc* pcInputSourceDesc);
	void							CopyActions(CInputSourceDesc* pcInputSourceDesc);
	void							SetRest(float fRestValue, bool bEmitRestEvent, bool bHasRestValue);
	bool							Is(char* szFriendlyName);
	CInputSourceValue*				StartValuesIteration(SSetIterator* psIter);
	CInputSourceValue*				IterateValues(SSetIterator* psIter);
	float							GetRestValue(void);
	bool							HasRestValue(void);
	CInputDeviceDesc*				GetDeviceDesc(void);
	CArrayInputCategoryGenericPtr*	GetGenerics(void);
	EInputSourceType				GetType(void);
	size							GetStateIndex(void);
	char*							GetFriendlyName(void);
	void							SetFriendlyName(char* szFriendlyName);
	bool							GetEmitRestEvent(void);
	void							ToString(CChars* psz);
};


typedef CSetType<CInputSourceDesc> CSetInputSourceDesc;
typedef CArrayTemplate<CInputSourceDesc*> CArrayInputSourceDescPtr;


#endif // __INPUT_SOURCE_DESC_H__

