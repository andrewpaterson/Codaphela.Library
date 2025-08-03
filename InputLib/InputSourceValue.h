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
#ifndef __INPUT_SOURCE_VALUE_H_
#define __INPUT_SOURCE_VALUE_H_
#include "StandardLib/SetUnknown.h"
#include "InputSourceChannel.h"
#include "InputDataFormat.h"


enum EInputSourceType
{
	ISET_State,
	ISET_Delta,
	ISET_Connection,
};


enum EInputSourceValueType
{
	ISVT_Unknown,
	ISVT_Numeric,
	ISVT_Channel,
};


union UInputSourceValue
{
	float	fNumber;
	size	iChannel;
};


union UInputSourceTypeDetail
{
	//Only valid for state events (buttons and axes etc...)
	struct  
	{
		float	fMaxValue;  
		float	fMinValue;
	} sState;

	//Only valid for delta events (motion)
	struct 
	{
		float	fSensitivity;
		float	fOffset;
	} sDelta;
};


class CInputSourceDesc;
class CInputSourceValue : public CUnknown
{
CONSTRUCTABLE(CInputSourceValue);
protected:
	CInputDataFormat*			mpcDataFormat;
	CInputSourceDesc*			mpcSourceDesc;
	CSetInputSouceChannel		mlcChannels;
	EInputSourceValueType		meValueType;
	UInputSourceValue			muValue;
	UInputSourceTypeDetail		muDetail;
	size						miOrder;

public:
	void 					Init(CInputSourceDesc* pcSourceDesc, CInputDataFormat* pcDataFormat, size iValueChannel);
	void 					Init(CInputSourceDesc* pcSourceDesc, CInputDataFormat* pcDataFormat, float fValueNumber);
	void 					PrivateInit(CInputSourceDesc* pcSourceDesc, CInputDataFormat* pcDataFormat);
	void 					Kill(void);
	void 					SetStateDetail(float fMaxValue, float fMinValue);
	void 					SetDeltaDetail(float fSensitivity, float fOffset);
	bool 					Process(void* pvData, float* pfValue);
	CInputSouceChannel*		AddChannel(size iChannel, EInputSourceChannelComparator eCompare, void* pvTest);
	void 					CopyChannels(CInputSourceValue* pcInputSourceValue);
	CInputSouceChannel*		StartChannelsIteration(SSetIterator* psIter);
	CInputSouceChannel*		IterateChannels(SSetIterator* psIter);
	CInputDataFormat*		GetDataFormat(void);
	size					GetOrder(void);
	void					SetOrder(size iOrder);
	EInputSourceValueType	GetValueType(void);
	float					GetMaxValue(void);
	float					GetMinValue(void);
	float					GetSensitivity(void);
	float					GetOffset(void);
	float					GetNumber(void);
	size					GetChannel(void);
	void 					ToString(CChars* psz, EInputSourceType eType);
};


typedef CSetType<CInputSourceValue>	CSetInputSourceValue;


#endif // __INPUT_SOURCE_VALUE_H_

