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
#ifndef __PROGRAM_INPUT_H__
#define __PROGRAM_INPUT_H__
#include "ProgramDeviceDetail.h"
#include "InputDevices.h"


class CProgramInput
{
public:
	CArrayProgramDeviceDetail	macPDeviceDetails;
	CInputDataFormat*			mpcFormat;
	CInputCategory*				mpcCategory;

	void 					Init(void);
	void 					Kill(void);
	void					AddFormat(CInputDevices* pcDevices);
	void					AddCategory(CInputDevices* pcDevices);
	CProgramDeviceDetail*	AddDetail(char* szID, char* szFriendlyName);
	void 					AddDevicesTo(CInputDevices* pcDevices);
	void 					Update(CInputDevices* pcDevices, uint32 uiSequence);
	CInputDeviceDesc*		CreateStandardDesc(CInputDevices* pcDevices, CProgramDeviceDetail* pcDetail);
	char*					GetCategoryName(void);
};


#endif // __PROGRAM_INPUT_H__

