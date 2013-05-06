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
#ifndef __PROGRAM_DEVICE_DETAIL_H__
#define __PROGRAM_DEVICE_DETAIL_H__
#include "BaseLib/Chars.h"
#include "NativeDetail.h"


class CProgramDeviceSourceValue
{
public:
	CChars	mszName;
	int		miName;

	void Init(char* szName, int iName);
	void Kill(void);
};


struct SProgramDeviceValue
{
	int		iName;
	float	fValue;
};


typedef CArrayTemplate<CProgramDeviceSourceValue>	CArrayProgramDeviceSourceValue;
typedef CArrayTemplate<SProgramDeviceValue>			CArrayProgramDeviceValue;


class CInputCategory;
class CProgramDeviceDetail : public CNativeDetail
{
public:
	CChars							mszID;
	CChars							mszFriendlyName;
	CArrayProgramDeviceSourceValue	macSources;
	CArrayProgramDeviceValue		macValues;

	void 							Init(char* szID, char* szFriendlyName);
	void 							Kill(void);
	void 							AddSource(char* szName);
	void 							AddEvent(char* szName, float fValue);
	int  							GetSourceID(char* szName);
	void 							ClearEvents(void);
	char*							GetFriendlyName(void);
	char*							GetID(void);
};


typedef CArrayTemplate<CProgramDeviceDetail>	CArrayProgramDeviceDetail;


#endif // __PROGRAM_DEVICE_DETAIL_H__

