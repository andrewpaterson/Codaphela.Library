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
#include "BaseLib/FloatHelper.h"
#include "InputSourceValue.h"
#include "InputSourceDesc.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceValue::PrivateInit(CInputSourceDesc* pcSourceDesc, CInputDataFormat* pcDataFormat)
{
	meValueType = ISVT_Unknown;
	mpcSourceDesc = pcSourceDesc;
	mpcDataFormat = pcDataFormat;
	mlcChannels.Init();
	miOrder = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceValue::Init(CInputSourceDesc* pcSourceDesc, CInputDataFormat* pcDataFormat, int iValueChannel)
{
	PrivateInit(pcSourceDesc, pcDataFormat);
	meValueType = ISVT_Channel;
	muValue.iChannel = iValueChannel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceValue::Init(CInputSourceDesc* pcSourceDesc, CInputDataFormat* pcDataFormat, float fValueNumber)
{
	PrivateInit(pcSourceDesc, pcDataFormat);
	meValueType = ISVT_Numeric;
	muValue.fNumber = fValueNumber;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceValue::Kill(void)
{
	mlcChannels.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceValue::SetStateDetail(float fMaxValue, float fMinValue)
{
	if (mpcSourceDesc->GetType() == ISET_State)
	{
		muDetail.sState.fMaxValue = fMaxValue;
		muDetail.sState.fMinValue = fMinValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceValue::SetDeltaDetail(float fSensitivity, float fOffset)
{
	if (mpcSourceDesc->GetType() == ISET_Delta)
	{	
		muDetail.sDelta.fSensitivity = fSensitivity;
		muDetail.sDelta.fOffset = fOffset;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputSourceValue::Process(void* pvData, float* pfValue)
{
	CInputSouceChannel*		pcSourceChannel;
	BOOL					bResult;
	float					f;
	SSetIterator			sIter;

	pcSourceChannel = mlcChannels.StartIteration(&sIter);
	while (pcSourceChannel)
	{
		bResult = pcSourceChannel->Compare(pvData);
		if (!bResult)
		{
			return FALSE;
		}
		pcSourceChannel = mlcChannels.Iterate(&sIter);
	}

	if (meValueType == ISVT_Numeric)
	{
		*pfValue = muValue.fNumber;
	}
	else if (meValueType == ISVT_Channel) 
	{
		if (mpcSourceDesc->GetType() == ISET_State)
		{
			f = mpcDataFormat->GetConvertToFloat(muValue.iChannel, pvData);
			*pfValue = NormaliseFloat(muDetail.sState.fMaxValue, muDetail.sState.fMinValue, f);
		}
		else if (mpcSourceDesc->GetType() == ISET_Delta)
		{
			f = mpcDataFormat->GetCastToFloat(muValue.iChannel, pvData);
			*pfValue = (f - muDetail.sDelta.fOffset) * muDetail.sDelta.fSensitivity;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSouceChannel* CInputSourceValue::AddChannel(int iChannel, EInputSourceChannelComparator eCompare, void* pvTest)
{
	CInputSouceChannel*		pcChannel;

	pcChannel = mlcChannels.Add();
	pcChannel->Init(this, iChannel, eCompare, (char*)pvTest);
	return pcChannel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceValue::CopyChannels(CInputSourceValue* pcInputSourceValue)
{
	CInputSouceChannel*		pcInputSourceChannel;
	CInputSouceChannel*		pcInputDestChannel;
	SSetIterator			sIter;

	pcInputSourceChannel = pcInputSourceValue->StartChannelsIteration(&sIter);
	while (pcInputSourceChannel)
	{
		pcInputDestChannel = mlcChannels.Add();
		pcInputDestChannel->Init(this, pcInputSourceChannel->GetChannel(), pcInputSourceChannel->GetCompare(), pcInputSourceChannel->GetTest());
		pcInputSourceChannel = pcInputSourceValue->IterateChannels(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSouceChannel* CInputSourceValue::StartChannelsIteration(SSetIterator* psIter) { return mlcChannels.StartIteration(psIter); }
CInputSouceChannel* CInputSourceValue::IterateChannels(SSetIterator* psIter) { return mlcChannels.Iterate(psIter); }
CInputDataFormat* CInputSourceValue::GetDataFormat(void) { return mpcDataFormat; }
int CInputSourceValue::GetOrder(void) { return miOrder; }
void CInputSourceValue::SetOrder(int iOrder) { miOrder = iOrder; }
EInputSourceValueType CInputSourceValue::GetValueType(void) { return meValueType; }
float CInputSourceValue::GetMaxValue(void) { return muDetail.sState.fMaxValue; }
float CInputSourceValue::GetMinValue(void) { return muDetail.sState.fMinValue; }
float CInputSourceValue::GetSensitivity(void) { return muDetail.sDelta.fSensitivity; }
float CInputSourceValue::GetOffset(void) { return muDetail.sDelta.fOffset; }
float CInputSourceValue::GetNumber(void) { return muValue.fNumber; }
int CInputSourceValue::GetChannel(void) { return muValue.iChannel; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSourceValue::ToString(CChars* psz, EInputSourceType eType)
{
	SSetIterator			sIter;
	CInputSouceChannel*		pcInputSouceChannel;

	psz->Append(" --- CInputSourceValue ---\n");
	psz->Append("Value Type: ");
	if (meValueType == ISVT_Numeric)
	{
		psz->Append("ISVT_Numeric");
		psz->AppendNewLine();
		psz->Append("Value: ");
		psz->Append(muValue.fNumber, 2);
		psz->AppendNewLine();
	}
	else if (meValueType == ISVT_Channel)
	{
		psz->Append("ISVT_Channel");
		psz->AppendNewLine();
		psz->Append("Value: ");
		psz->Append(muValue.iChannel);
		psz->AppendNewLine();
	}
	else if (meValueType == ISVT_Unknown)
	{
		psz->Append("ISVT_Unknown");
		psz->AppendNewLine();
	}
	psz->Append("Order: ");
	psz->Append(miOrder);
	psz->AppendNewLine();

	if (eType == ISET_State)
	{
		psz->Append("Max Value: ");
		psz->Append(muDetail.sState.fMaxValue);
		psz->AppendNewLine();
		psz->Append("Min Value: ");
		psz->Append(muDetail.sState.fMinValue);
		psz->AppendNewLine();
	}
	else if (eType == ISET_Delta)
	{
		psz->Append("Offset: ");
		psz->Append(muDetail.sDelta.fOffset);
		psz->AppendNewLine();
		psz->Append("Sensitivity: ");
		psz->Append(muDetail.sDelta.fSensitivity);
		psz->AppendNewLine();
	}

	psz->Append(" - CInputSouceChannel : mlcChannels -\n");

	pcInputSouceChannel = mlcChannels.StartIteration(&sIter);
	while (pcInputSouceChannel)
	{
		pcInputSouceChannel->ToString(psz);
		pcInputSouceChannel = mlcChannels.Iterate(&sIter);
	}
}

