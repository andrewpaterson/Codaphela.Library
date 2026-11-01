/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
template<class M>
bool CompareInputValues(void* pvValue, void* pvTest, EInputSourceChannelComparator eCompare)
{
	M	tValue;
	M	tTest;

	memcpy(&tValue, pvValue, sizeof(M));
	memcpy(&tTest, pvTest, sizeof(M));

	switch (eCompare)
	{
	case ISCC_Equals:
		return tValue == tTest;
	case ISCC_LessThan:
		return tValue < tTest;
	case ISCC_LessThanEquals:
		return tValue <= tTest;
	case ISCC_GreaterThan:
		return tValue > tTest;
	case ISCC_GreaterThanEquals:
		return tValue >= tTest;
	case ISCC_NotEquals:
		return tValue != tTest;
	case ISCC_AlwaysValid:
		return true;
	case ISCC_Unknown:
		break;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputSouceChannel::Compare(void* pvData)
{
	CChannel*			pcChannel;
	char				sValue[8];
	EPrimitiveType		eType;

	pcChannel = mpcSourceValue->GetDataFormat()->Get(miChannel);
	eType = pcChannel->eType;
	mpcSourceValue->GetDataFormat()->GetAsNative(sValue, miChannel, pvData);
	if ((eType >= PRIMTIVE_OPERATOR_END) && (eType < NUM_PRIMITIVE_TYPES))
	{
		eType = PT_uint8;
	}

	switch (eType)
	{
	case PT_int8:		return CompareInputValues<int8>(sValue, mTest, meCompare);
	case PT_int16:		return CompareInputValues<int16>(sValue, mTest, meCompare);
	case PT_int32:		return CompareInputValues<int32>(sValue, mTest, meCompare);
	case PT_int64:		return CompareInputValues<int64>(sValue, mTest, meCompare);
	case PT_uint8:		return CompareInputValues<uint8>(sValue, mTest, meCompare);
	case PT_uint16:		return CompareInputValues<uint16>(sValue, mTest, meCompare);
	case PT_uint32:		return CompareInputValues<uint32>(sValue, mTest, meCompare);
	case PT_uint64:		return CompareInputValues<uint64>(sValue, mTest, meCompare);
	case PT_char8:		return CompareInputValues<char>(sValue, mTest, meCompare);
	case PT_float32:	return CompareInputValues<float32>(sValue, mTest, meCompare);
	case PT_float64:	return CompareInputValues<float64>(sValue, mTest, meCompare);
	case PT_bool:		return CompareInputValues<bool>(sValue, mTest, meCompare);
	default:			return false;
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
	case ISCC_Unknown:
		psz->Append("ISCC_Unknown: ");
		break;
	}
	psz->AppendNewLine();

	psz->Append("Test: 0x");
	psz->AppendHexLoHi(mTest, 8);
}

