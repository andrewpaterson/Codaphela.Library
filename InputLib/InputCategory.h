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
#ifndef __INPUT_CATEGORY_H__
#define __INPUT_CATEGORY_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "StandardLib/Unknown.h"
#include "StandardLib/SetType.h"
#include "InputDevicePointerArray.h"
#include "InputCategoryGeneric.h"
#include "InputCategory.h"


class CInputDevices;
class CInputCategory : public CUnknown
{
CONSTRUCTABLE(CInputCategory);
protected:
	CChars						mszCategory;
	CSetInputCategoryGeneric	mlcGenerics;
	CInputDevices*				mpcInputDevices;

public:
	void 					Init(char* szCategory, CInputDevices* pcInputDevices);
	void 					Kill(void);
	CInputCategoryGeneric*	AddGeneric(char* szGeneric);
	BOOL 					ContainsGeneric(CInputCategoryGeneric* pcGeneric);
	BOOL					ContainsGeneric(char* szGeneric);
	CInputDevice*			GetFirstDevice(BOOL bPhysical = TRUE);
	void					GetDevices(CArrayInputDevicePtr* pcDevices, BOOL bPhysical = TRUE);
	CInputCategoryGeneric*	GetGeneric(char* szGeneric);
	char*					GetCategoryName(void);
	CInputDevices*			GetInputDevices(void);
	BOOL					Is(char* szName);
};


typedef CSetType<CInputCategory> CLinkListCategory;


#endif // !__INPUT_CATEGORY_H__

