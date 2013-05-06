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
#ifndef __INPUT_SOURCE_CHANNEL_H__
#define __INPUT_SOURCE_CHANNEL_H__
#include "StandardLib/SetType.h"

enum EInputSourceChannelComparator
{
	ISCC_Unknown,
	ISCC_Equals,
	ISCC_LessThan,
	ISCC_LessThanEquals,
	ISCC_GreaterThan,
	ISCC_GreaterThanEquals,
	ISCC_NotEquals,
	ISCC_AlwaysValid,
};


class CInputSourceValue;
class CInputSouceChannel : public CUnknown
{
BASE_FUNCTIONS(CInputSouceChannel);
protected:
	int								miChannel;  //Index into InputDataFormat channels.
	EInputSourceChannelComparator	meCompare;
	char							mTest[8];	//The value in here must be the same type as that in the channel.
												//mTest is the value that is tested by the comparator.  ie:  Match = 'Channel' 'Compare' 'Test'
	CInputSourceValue*				mpcSourceValue;

public:
	void 							Init(CInputSourceValue* pcSourceValue, int iChannel, EInputSourceChannelComparator eCompare, char* pTest);
	void 							Kill(void);
	BOOL 							Compare(void* pvData);
	void 							ToString(CChars* psz);
	int								GetChannel(void);
	EInputSourceChannelComparator	GetCompare(void);
	char*							GetTest(void);

};


typedef CSetType<CInputSouceChannel> CSetInputSouceChannel;


#endif // __INPUT_SOURCE_CHANNEL_H__

