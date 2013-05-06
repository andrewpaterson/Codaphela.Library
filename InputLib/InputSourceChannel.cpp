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
#include "CoreLib/Operators.h"
#include "InputSourceChannel.h"
#include "InputSourceValue.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSouceChannel::Init(CInputSourceValue* pcSourceValue, int iChannel, EInputSourceChannelComparator eCompare, char* pTest)
{
	mpcSourceValue = pcSourceValue;
	miChannel = iChannel;
	memcpy(mTest, pTest, 8);
	meCompare = eCompare;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSouceChannel::Kill(void)
{
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInputSouceChannel::Compare(void* pvData)
{
	CChannel*			pcChannel;
	BOOL				bResult;
	char				sValue[8];
	EPrimitiveTypes		eOperatorType;

	pcChannel = mpcSourceValue->GetDataFormat()->Get(miChannel);
	eOperatorType = pcChannel->eType;
	mpcSourceValue->GetDataFormat()->GetAsNative(sValue, miChannel, pvData);
	if ((eOperatorType >= PRIMTIVE_OPERATOR_END) && (eOperatorType < PRIMTIVE_TYPE_END))
	{
		eOperatorType = PT_uchar;
	}
	
	if (eOperatorType < PRIMTIVE_OPERATOR_END)
	{
		switch (meCompare)
		{
		case ISCC_Equals:
			gcOperators.mcEqual.Do(&bResult, eOperatorType, sValue, eOperatorType, mTest);
			return bResult;
		case ISCC_LessThan:
			gcOperators.mcLess.Do(&bResult, eOperatorType, sValue, eOperatorType, mTest);
			return bResult;
		case ISCC_LessThanEquals:
			gcOperators.mcLessOrEqual.Do(&bResult, eOperatorType, sValue, eOperatorType, mTest);
			return bResult;
		case ISCC_GreaterThan:
			gcOperators.mcGreater.Do(&bResult, eOperatorType, sValue, eOperatorType, mTest);
			return bResult;
		case ISCC_GreaterThanEquals:
			gcOperators.mcGreaterOrEqual.Do(&bResult, eOperatorType, sValue, eOperatorType, mTest);
			return bResult;
		case ISCC_NotEquals:
			gcOperators.mcNotEqual.Do(&bResult, eOperatorType, sValue, eOperatorType, mTest);
			return bResult;
		case ISCC_AlwaysValid:
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CInputSouceChannel::GetChannel(void) { return miChannel; }
EInputSourceChannelComparator CInputSouceChannel::GetCompare(void) { return meCompare; }
char* CInputSouceChannel::GetTest(void) { return mTest; }



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputSouceChannel::ToString(CChars* psz)
{
	psz->Append(" --- CInputSouceChannel ---\n");

	psz->Append("Channel: ");
	psz->Append(miChannel);
	psz->AppendNewLine();

	psz->Append("meCompare: ");
	switch (meCompare)
	{
	case ISCC_Equals:
		psz->Append("ISCC_Equals: ");
		break;
	case ISCC_LessThan:
		psz->Append("ISCC_LessThan: ");
		break;
	case ISCC_LessThanEquals:
		psz->Append("ISCC_LessThanEquals: ");
		break;
	case ISCC_GreaterThan:
		psz->Append("ISCC_GreaterThan: ");
		break;
	case ISCC_GreaterThanEquals:
		psz->Append("ISCC_GreaterThanEquals: ");
		break;
	case ISCC_NotEquals:
		psz->Append("ISCC_NotEquals: ");
		break;
	case ISCC_AlwaysValid:
		psz->Append("ISCC_AlwaysValid: ");
		break;
	}
	psz->AppendNewLine();

	psz->Append("Test: 0x");
	psz->AppendHexLoHi(mTest, 8);
}

