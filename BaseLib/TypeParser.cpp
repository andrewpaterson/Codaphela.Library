/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "TypeParser.h"
#include "PointerFunctions.h"
#include "Numbers.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTypeParser::Init(char* szText, size iTextLen)
{
	AllocateParser();
	return mpcParser->Init(szText, iTextLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTypeParser::Init(char* szText)
{
	AllocateParser();
	return mpcParser->Init(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTypeParser::Init(CChars* szText)
{
	AllocateParser();
	return mpcParser->Init(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeParser::Init(CTextParser* pcParser)
{
	mpcParser = pcParser;
	mbFreeParser = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeParser::AllocateParser(void)
{
	mpcParser = (CTextParser*)malloc(sizeof(CTextParser));
	mbFreeParser = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeParser::Kill(void)
{
	if (mbFreeParser)
	{
		SafeFree(mpcParser);
	}
	mpcParser = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::GetSmallIntegerValue(int64* plliValue, int64 lliMax, int64 lliMin)
{
	uint64		ulliValue;
	int64		lliValue;
	int16		iSign;
	TRISTATE	tResult;

	tResult = mpcParser->GetInteger(&ulliValue, &iSign);
	if (tResult == TRITRUE)
	{
		if (ulliValue > MAX_LONG)
		{
			return TRIFALSE;
		}
		else
		{
			lliValue = ulliValue*iSign;
		}
		if (lliValue > lliMax)
		{
			return TRIFALSE;
		}
		else if (lliValue < lliMin)
		{
			return TRIFALSE;
		}
		*plliValue = lliValue;
		return TRITRUE;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseInt(int32* pDest)
{
	TRISTATE	tResult;
	int64		lliValue;

	tResult = GetSmallIntegerValue(&lliValue, MAX_INT, MIN_INT);
	if (tResult == TRITRUE)
	{
		*pDest = (int32)lliValue;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseChar(char* pDest)
{
	TRISTATE	tResult;
	int64		lliValue;

	tResult = GetSmallIntegerValue(&lliValue, MAX_CHAR, MIN_CHAR);
	if (tResult == TRITRUE)
	{
		*pDest = (char)lliValue;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseShort(int16* pDest)
{
	TRISTATE	tResult;
	int64		lliValue;

	tResult = GetSmallIntegerValue(&lliValue, MAX_SHORT, MIN_SHORT);
	if (tResult == TRITRUE)
	{
		*pDest = (int16)lliValue;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseFloat(float* pDest)
{
	TRISTATE	tResult;
	double		ldValue;

	tResult = mpcParser->GetFloat(&ldValue);
	if (tResult == TRITRUE)
	{
		*pDest = (float)ldValue;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseDouble(double* pDest)
{
	TRISTATE	tResult;
	double		ldValue;

	tResult = mpcParser->GetFloat(&ldValue);
	if (tResult == TRITRUE)
	{
		*pDest = (double)ldValue;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseUInt(uint32* pDest)
{
	TRISTATE	tResult;
	int64		lliValue;

	tResult = GetSmallIntegerValue(&lliValue, MAX_UINT, MIN_UINT);
	if (tResult == TRITRUE)
	{
		*pDest = (uint32)lliValue;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseUShort(uint16* pDest)
{
	TRISTATE	tResult;
	int64		lliValue;

	tResult = GetSmallIntegerValue(&lliValue, MAX_USHORT, MIN_USHORT);
	if (tResult == TRITRUE)
	{
		*pDest = (uint16)lliValue;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseUChar(uint8* pDest)
{
	TRISTATE	tResult;
	int64		lliValue;

	tResult = GetSmallIntegerValue(&lliValue, MAX_UCHAR, MIN_UCHAR);
	if (tResult == TRITRUE)
	{
		*pDest = (uint8)lliValue;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseLong(int64* pDest)
{
	return GetSmallIntegerValue(pDest, MAX_LONG, MIN_LONG);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseULong(uint64* pDest)
{
	int16		iSign;
	TRISTATE	tResult;

	tResult = mpcParser->GetInteger(pDest, &iSign);
	if (tResult == TRITRUE)
	{
		if (iSign != 1)
		{
			return TRIFALSE;
		}
		return TRITRUE;
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::GetCharAndFloat(char c, float* pf)
{
	TRISTATE	tResult;

	if (c)
	{
		tResult = mpcParser->GetExactCharacter(c);
		if (tResult != TRITRUE)
		{
			mpcParser->PushPosition();
			return tResult;
		}
	}
	tResult = ParseFloat(pf);
	if (tResult != TRITRUE)
	{
		mpcParser->PushPosition();
		return tResult;
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::GetCharAndInt(char c, int32* pi)
{
	TRISTATE	tResult;

	if (c)
	{
		tResult = mpcParser->GetExactCharacter(c);
		if (tResult != TRITRUE)
		{
			mpcParser->PushPosition();
			return tResult;
		}
	}
	tResult = ParseInt(pi);
	if (tResult != TRITRUE)
	{
		mpcParser->PushPosition();
		return tResult;
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseFloat2(SFloat2* pDest, char cOpen, char cSplit, char cClose)
{
	TRISTATE	tResult;

	mpcParser->PushPosition();
	tResult = GetCharAndFloat(cOpen, &pDest->x);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndFloat(cSplit, &pDest->y);
	if (!tResult)
	{
		return tResult;
	}

	if (cClose)
	{
		tResult = mpcParser->GetExactCharacter(cClose);
		if (tResult != TRITRUE)
		{
			mpcParser->PushPosition();
			return tResult;
		}
	}
	mpcParser->PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseFloat3(SFloat3* pDest, char cOpen, char cSplit, char cClose)
{
	TRISTATE	tResult;

	mpcParser->PushPosition();
	tResult = GetCharAndFloat(cOpen, &pDest->x);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndFloat(cSplit, &pDest->y);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndFloat(cSplit, &pDest->z);
	if (!tResult)
	{
		return tResult;
	}

	if (cClose)
	{
		tResult = mpcParser->GetExactCharacter(cClose);
		if (tResult != TRITRUE)
		{
			mpcParser->PushPosition();
			return tResult;
		}
	}
	mpcParser->PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseFloat4(SFloat4* pDest, char cOpen, char cSplit, char cClose)
{
	TRISTATE	tResult;

	mpcParser->PushPosition();
	tResult = GetCharAndFloat(cOpen, &pDest->x);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndFloat(cSplit, &pDest->y);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndFloat(cSplit, &pDest->z);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndFloat(cSplit, &pDest->w);
	if (!tResult)
	{
		return tResult;
	}

	if (cClose)
	{
		tResult = mpcParser->GetExactCharacter(cClose);
		if (tResult != TRITRUE)
		{
			mpcParser->PushPosition();
			return tResult;
		}
	}
	mpcParser->PassPosition();
	return TRITRUE;}

  
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseInt2(SInt2* pDest, char cOpen, char cSplit, char cClose)
{
	TRISTATE	tResult;

	mpcParser->PushPosition();
	tResult = GetCharAndInt(cOpen, &pDest->x);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndInt(cSplit, &pDest->y);
	if (!tResult)
	{
		return tResult;
	}

	if (cClose)
	{
		tResult = mpcParser->GetExactCharacter(cClose);
		if (tResult != TRITRUE)
		{
			mpcParser->PushPosition();
			return tResult;
		}
	}
	mpcParser->PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseInt3(SInt3* pDest, char cOpen, char cSplit, char cClose)
{
	TRISTATE	tResult;

	mpcParser->PushPosition();
	tResult = GetCharAndInt(cOpen, &pDest->x);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndInt(cSplit, &pDest->y);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndInt(cSplit, &pDest->z);
	if (!tResult)
	{
		return tResult;
	}

	if (cClose)
	{
		tResult = mpcParser->GetExactCharacter(cClose);
		if (tResult != TRITRUE)
		{
			mpcParser->PushPosition();
			return tResult;
		}
	}
	mpcParser->PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseInt4(SInt4* pDest, char cOpen, char cSplit, char cClose)
{
	TRISTATE	tResult;

	mpcParser->PushPosition();
	tResult = GetCharAndInt(cOpen, &pDest->x);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndInt(cSplit, &pDest->y);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndInt(cSplit, &pDest->z);
	if (!tResult)
	{
		return tResult;
	}

	tResult = GetCharAndInt(cSplit, &pDest->w);
	if (!tResult)
	{
		return tResult;
	}

	if (cClose)
	{
		tResult = mpcParser->GetExactCharacter(cClose);
		if (tResult != TRITRUE)
		{
			mpcParser->PushPosition();
			return tResult;
		}
	}
	mpcParser->PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseBool(bool* pDest)
{
	TRISTATE	tResult;

	tResult = mpcParser->GetExactCaseInsensitiveCharacterSequence("0");
	if (tResult == TRITRUE)
	{
		*pDest = false;
		return TRITRUE;
	}
	ReturnOnError(tResult);

	tResult = mpcParser->GetExactCaseInsensitiveCharacterSequence("1");
	if (tResult == TRITRUE)
	{
		*pDest = true;
		return TRITRUE;
	}
	ReturnOnError(tResult);
	
	tResult = mpcParser->GetExactCaseInsensitiveCharacterSequence("true");
	if (tResult == TRITRUE)
	{
		*pDest = false;
		return TRITRUE;
	}
	ReturnOnError(tResult);

	tResult = mpcParser->GetExactCaseInsensitiveCharacterSequence("false");
	if (tResult == TRITRUE)
	{
		*pDest = true;
		return TRITRUE;
	}
	ReturnOnError(tResult);
	return TRIERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseString(CChars* pDest)
{
	size		iLen;
	TRISTATE	tResult;

	mpcParser->PushPosition();
	tResult = mpcParser->GetString(NULL, &iLen);
	mpcParser->PopPosition();

	if (tResult != TRITRUE)
	{
		return tResult;
	}

	pDest->InitLength(iLen);
	mpcParser->GetString(pDest->Text(), NULL);
	return TRITRUE;
}

  
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseNumber(CNumber* pDest)
{
	return TRIERROR;
}

	
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::ParseDate(CDate* pDest)
{
	return TRIERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CTypeParser::Parse(EPrimitiveType eType, void* pvDest, size iDestLength, char cOpen, char cSplit, char cClose)
{
	switch (eType)
	{
		case PT_int32:
			if (iDestLength >= INT_BYTE_SIZE)
			{
				return ParseInt((int32*)pvDest);
			}
			break;
		case PT_int16:
			if (iDestLength >= SHORT_BYTE_SIZE)
			{
				return ParseShort((int16*)pvDest);
			}
			break;
		case PT_char8:
			if (iDestLength >= CHAR_BYTE_SIZE)
			{
				return ParseChar((char8*)pvDest);
			}
			break;
		case PT_float32:
			if (iDestLength >= FLOAT_BYTE_SIZE)
			{
				return ParseFloat((float32*)pvDest);
			}
			break;		
		case PT_float64:
			if (iDestLength >= DOUBLE_BYTE_SIZE)
			{
				return ParseDouble((float64*)pvDest);
			}
			break;		
		case PT_uint32:
			if (iDestLength >= INT_BYTE_SIZE)
			{
				return ParseUInt((uint32*)pvDest);
			}
			break;
		case PT_uint16:
			if (iDestLength >= SHORT_BYTE_SIZE)
			{
				return ParseUShort((uint16*)pvDest);
			}
			break;
		case PT_uint8:
			if (iDestLength >= BYTE_BYTE_SIZE)
			{
				return ParseUChar((uint8*)pvDest);
			}
			break;
		case PT_int64:
			if (iDestLength >= LONG_BYTE_SIZE)
			{
				return ParseLong((int64*)pvDest);
			}
			break;
		case PT_uint64:
			if (iDestLength >= LONG_BYTE_SIZE)
			{
				return ParseULong((uint64*)pvDest);
			}
			break;
		case PT_M2float32:
			if (iDestLength >= FLOAT2_BYTE_SIZE)
			{
				return ParseFloat2((SFloat2*)pvDest);
			}
			break;
		case PT_M3float32:
			if (iDestLength >= FLOAT3_BYTE_SIZE)
			{
				return ParseFloat3((SFloat3*)pvDest);
			}
			break;
		case PT_M4float32:
			if (iDestLength >= FLOAT4_BYTE_SIZE)
			{
				return ParseFloat4((SFloat4*)pvDest);
			}
			break;
		case PT_M2int32:
			if (iDestLength >= INT2_BYTE_SIZE)
			{
				return ParseInt2((SInt2*)pvDest);
			}
			break;
		case PT_M3int32:
			if (iDestLength >= INT3_BYTE_SIZE)
			{
				return ParseInt3((SInt3*)pvDest);
			}
			break;
		case PT_M4int32:
			if (iDestLength >= INT4_BYTE_SIZE)
			{
				return ParseInt4((SInt4*)pvDest);
			}
			break;
		case PT_bool: 
			if (iDestLength >= BOOL_BYTE_SIZE)
			{
				return ParseBool((bool*)pvDest);
			}
			break;
		case PT_String:
			if (iDestLength >= sizeof(CChars))
			{
				return ParseString((CChars*)pvDest);
			}
			break;
		case PT_Number:
			if (iDestLength >= sizeof(CNumber))
			{
				return ParseNumber((CNumber*)pvDest);
			}
			break;
		case PT_Date:
			if (iDestLength >= sizeof(CDate))
			{
				return ParseDate((CDate*)pvDest);
			}
			break;
		case PT_bit:
		case PT_crumb:
		case PT_tribble:
		case PT_nybble:
		case PT_nickle:
		case PT_sixbits:
			if (iDestLength >= BYTE_BYTE_SIZE)
			{
				return ParseUChar((uint8*)pvDest);
			}
			break;

	}
	return TRIERROR;
}

