/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "GuidClass.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGuidClass::Init(uint32 uiData1, uint16 uiData2, uint16 uiData3, int8* paubData4)
{
	muiData1 = uiData1;
	muiData2 = uiData2;
	muiData3 = uiData3;
	memcpy(maubData4, paubData4, 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGuidClass::Init(char8* sz)
{
	CTextParser	cParser;
	TRISTATE	tOpenCurly;
	TRISTATE	tOpenRound;
	TRISTATE	tResult;
	uint64		uiValue;		
	int32		iLength;
	BOOL		bResult;

	cParser.Init(sz);
	tOpenCurly = cParser.GetExactCharacter('{');
	tOpenRound = TRIERROR;
	if (tOpenCurly == TRIERROR)
	{
		cParser.Kill();
		Clear();
		return;
	}
	else if (tOpenCurly == TRIFALSE)
	{
		tOpenRound = cParser.GetExactCharacter('(');
	}

	tResult = cParser.GetHexadecimal(&uiValue, &iLength);
	if (tResult == TRITRUE)
	{
		bResult = Get0xFormat(&cParser, (uint32)uiValue);
		if (!bResult)
		{
			Clear();
			cParser.Kill();
			return;
		}
	}
	else
	{
		tResult = cParser.GetHexadecimalPart(&uiValue, &iLength);
		if (tResult == TRITRUE)
		{
			if (iLength == 16)
			{
				bResult = Get32Format(&cParser, uiValue);
			}
			else if (iLength == 8)
			{
				bResult = Get8_4_4_4_12Format(&cParser, (uint32)uiValue);
				if (!bResult)
				{
					Clear();
					cParser.Kill();
					return;
				}
			}
			else
			{
				Clear();
				cParser.Kill();
				return;
			}
		}
	}

	if (tOpenCurly == TRITRUE)
	{
		tResult = cParser.GetExactCharacter('}');
		if (tResult != TRITRUE)
		{
			cParser.Kill();
			Clear();
			return;
		}
	}
	else if (tOpenRound == TRITRUE)
	{
		tResult = cParser.GetExactCharacter(')');
		if (tResult != TRITRUE)
		{
			cParser.Kill();
			Clear();
			return;
		}
	}

	cParser.Kill();
	return;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGuidClass::Clear(void)
{
	memset(this, 0, sizeof(CGuidClass));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGuidClass::Get0xFormat(CTextParser* pcParser, uint32 uiValue1)
{
	TRISTATE	tResult;
	uint64		uiValue2;
	uint64		uiValue3;
	uint64		uiSmallRight;
	int			iLength;
	int			i;
	TRISTATE	tOpenCurly;
	TRISTATE	tOpenRound;

	tResult = pcParser->GetExactCharacter(',');
	if (tResult != TRITRUE)
	{
		return FALSE;
	}
	tResult = pcParser->GetHexadecimal(&uiValue2, &iLength);
	if (tResult != TRITRUE)
	{
		return FALSE;
	}
	tResult = pcParser->GetExactCharacter(',');
	if (tResult != TRITRUE)
	{
		return FALSE;
	}
	tResult = pcParser->GetHexadecimal(&uiValue3, &iLength);
	if (tResult != TRITRUE)
	{
		return FALSE;
	}
	tResult = pcParser->GetExactCharacter(',');
	if (tResult != TRITRUE)
	{
		return FALSE;
	}

	muiData1 = uiValue1;
	muiData2 = (uint16)uiValue2;
	muiData3 = (uint16)uiValue3;

	tOpenCurly = pcParser->GetExactCharacter('{');
	if (tOpenCurly == TRIERROR)
	{
		return FALSE;
	}
	else if (tOpenCurly == TRIFALSE)
	{
		tOpenRound = pcParser->GetExactCharacter('(');
	}

	for (i = 0; i < 8; i++)
	{
		tResult = pcParser->GetHexadecimal(&uiSmallRight, &iLength);
		if (tResult != TRITRUE)
		{
			return FALSE;
		}
		if (i != 7)
		{
			tResult = pcParser->GetExactCharacter(',');
			if (tResult != TRITRUE)
			{
				return FALSE;
			}
		}
		maubData4[i] = (uint8)uiSmallRight;
	}

	if (tOpenCurly == TRITRUE)
	{
		tResult = pcParser->GetExactCharacter('}');
		if (tResult != TRITRUE)
		{
			return FALSE;
		}
	}
	else if (tOpenRound == TRITRUE)
	{
		tResult = pcParser->GetExactCharacter(')');
		if (tResult != TRITRUE)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGuidClass::Get32Format(CTextParser* pcParser, uint64 uiLeft)
{
	uint64		uiRight;
	TRISTATE	tResult;
	int32		iLength;

	tResult = pcParser->GetHexadecimalPart(&uiRight, &iLength);
	if (tResult == TRITRUE)
	{
		if (iLength == 16)
		{
			muiData1 = (uint32)(uiLeft >> 32);
			muiData2 = (uint16)((uiLeft & 0x00000000ffff0000LL) >> 16);
			muiData3 = (uint16)(uiLeft & 0x000000000000ffffLL);
			maubData4[0] = ((uint8*)&uiRight)[7];
			maubData4[1] = ((uint8*)&uiRight)[6];
			maubData4[2] = ((uint8*)&uiRight)[5];
			maubData4[3] = ((uint8*)&uiRight)[4];
			maubData4[4] = ((uint8*)&uiRight)[3];
			maubData4[5] = ((uint8*)&uiRight)[2];
			maubData4[6] = ((uint8*)&uiRight)[1];
			maubData4[7] = ((uint8*)&uiRight)[0];
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CGuidClass::Get8_4_4_4_12Format(CTextParser* pcParser, uint32 uiValue1)
{
	TRISTATE	tResult;
	uint64		uiValue2;
	uint64		uiValue3;
	uint64		uiRight1;
	uint64		uiRight2;
	int			iLength;

	tResult = pcParser->GetExactCharacter('-');
	if (tResult != TRITRUE)
	{
		return FALSE;
	}
	tResult = pcParser->GetHexadecimalPart(&uiValue2, &iLength, 4);
	if ((tResult != TRITRUE) || (iLength != 4))
	{
		return FALSE;
	}
	tResult = pcParser->GetExactCharacter('-');
	if (tResult != TRITRUE)
	{
		return FALSE;
	}
	tResult = pcParser->GetHexadecimalPart(&uiValue3, &iLength, 4);
	if ((tResult != TRITRUE) || (iLength != 4))
	{
		return FALSE;
	}
	tResult = pcParser->GetExactCharacter('-');
	if (tResult != TRITRUE)
	{
		return FALSE;
	}
	tResult = pcParser->GetHexadecimalPart(&uiRight1, &iLength, 4);
	if ((tResult != TRITRUE) || (iLength != 4))
	{
		return FALSE;
	}
	tResult = pcParser->GetExactCharacter('-');
	if (tResult != TRITRUE)
	{
		return FALSE;
	}
	tResult = pcParser->GetHexadecimalPart(&uiRight2, &iLength, 12);
	if ((tResult != TRITRUE) || (iLength != 12))
	{
		return FALSE;
	}

	muiData1 = uiValue1;
	muiData2 = (uint16)uiValue2;
	muiData3 = (uint16)uiValue3;
	maubData4[0] = ((uint8*)&uiRight1)[1];
	maubData4[1] = ((uint8*)&uiRight1)[0];
	maubData4[2] = ((uint8*)&uiRight2)[5];
	maubData4[3] = ((uint8*)&uiRight2)[4];
	maubData4[4] = ((uint8*)&uiRight2)[3];
	maubData4[5] = ((uint8*)&uiRight2)[2];
	maubData4[6] = ((uint8*)&uiRight2)[1];
	maubData4[7] = ((uint8*)&uiRight2)[0];
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGuidClass::Append(CChars* psz)
{
	psz->Append('{');
	psz->AppendHexHiLo(&muiData1, 4);
	psz->Append('-');
	psz->AppendHexHiLo(&muiData2, 2);
	psz->Append('-');
	psz->AppendHexHiLo(&muiData3, 2);
	psz->Append('-');
	psz->AppendHexHiLo(maubData4, 8);
	psz->Append('}');
}

