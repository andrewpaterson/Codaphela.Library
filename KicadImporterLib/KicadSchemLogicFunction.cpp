/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/StringHelper.h"
#include "KicadSchemLogicFunction.h"


//The families in Logicim's propagation times.
static const char* gaszKicadSchemLogicimFamilies[] = { "ABT", "AHCT", "ALS", "ALVC", "F", "FCT", "HCT", "LCX", "LS", "LVC", "LVT", "LVX", "VCX", "VHCT" };


struct SKicadSchemLogicPartFunction
{
	const char*					szNumber;
	EKicadSchemLogicFunction	eFunction;
};


static SKicadSchemLogicPartFunction gasKicadSchemLogicPartFunctions[] =
{
	{ "00", KSLF_Nand },
	{ "02", KSLF_Nor },
	{ "04", KSLF_Inverter },
	{ "05", KSLF_Inverter },
	{ "06", KSLF_Inverter },
	{ "07", KSLF_Buffer },
	{ "08", KSLF_And },
	{ "09", KSLF_And },
	{ "10", KSLF_Nand },
	{ "11", KSLF_And },
	{ "14", KSLF_Inverter },
	{ "17", KSLF_Buffer },
	{ "20", KSLF_Nand },
	{ "21", KSLF_And },
	{ "27", KSLF_Nor },
	{ "30", KSLF_Nand },
	{ "32", KSLF_Or },
	{ "34", KSLF_Buffer },
	{ "74", KSLF_DFlipFlop },
	{ "86", KSLF_Xor },
	{ "132", KSLF_Nand },
	{ "266", KSLF_Xnor },
	{ "7266", KSLF_Xnor },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsKicadSchemDigit(char c)
{
	return (c >= '0') && (c <= '9');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsKicadSchemUpper(char c)
{
	return (c >= 'A') && (c <= 'Z');
}


//////////////////////////////////////////////////////////////////////////
//
// Tries each "74" in the part number that doesn't follow a digit.
//
//////////////////////////////////////////////////////////////////////////
EKicadSchemLogicFunction ParseKicadSchemLogicFunction(const char* szPartNumber, CChars* pszFamily)
{
	size		i;
	size		j;
	size		uiFamilyStart;
	size		uiFamilyEnd;
	size		uiNumberStart;
	size		k;
	CChars		szNumber;
	CChars		szFamily;
	EKicadSchemLogicFunction	eFunction;

	if (szPartNumber == NULL)
	{
		return KSLF_None;
	}

	for (i = 0; szPartNumber[i] != '\0'; i++)
	{
		if ((szPartNumber[i] != '7') || (szPartNumber[i + 1] != '4') || ((i > 0) && IsKicadSchemDigit(szPartNumber[i - 1])))
		{
			continue;
		}

		j = i + 2;
		uiFamilyStart = j;
		while (IsKicadSchemUpper(szPartNumber[j]))
		{
			j++;
		}
		uiFamilyEnd = j;
		if (IsKicadSchemDigit(szPartNumber[j]) && (szPartNumber[j + 1] == 'G'))
		{
			j += 2;
		}
		uiNumberStart = j;
		while (IsKicadSchemDigit(szPartNumber[j]))
		{
			j++;
		}
		if (j == uiNumberStart)
		{
			continue;
		}

		szNumber.Init(szPartNumber, uiNumberStart, j);
		eFunction = KSLF_None;
		for (k = 0; k < sizeof(gasKicadSchemLogicPartFunctions) / sizeof(SKicadSchemLogicPartFunction); k++)
		{
			if (szNumber.Equals(gasKicadSchemLogicPartFunctions[k].szNumber))
			{
				eFunction = gasKicadSchemLogicPartFunctions[k].eFunction;
				break;
			}
		}
		szNumber.Kill();
		if (eFunction == KSLF_None)
		{
			continue;
		}

		if (pszFamily != NULL)
		{
			szFamily.Init(szPartNumber, uiFamilyStart, uiFamilyEnd);
			pszFamily->Set(IsKicadSchemLogicimFamily(szFamily.Text()) ? szFamily.Text() : "LVC");
			szFamily.Kill();
		}
		return eFunction;
	}
	return KSLF_None;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsKicadSchemLogicimFamily(const char* szFamily)
{
	size	i;

	for (i = 0; i < sizeof(gaszKicadSchemLogicimFamilies) / sizeof(const char*); i++)
	{
		if (StringCompare(gaszKicadSchemLogicimFamilies[i], szFamily) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* KicadSchemLogicFunctionName(EKicadSchemLogicFunction eFunction)
{
	switch (eFunction)
	{
	case KSLF_And:			return "And";
	case KSLF_Nand:			return "Nand";
	case KSLF_Or:			return "Or";
	case KSLF_Nor:			return "Nor";
	case KSLF_Xor:			return "Xor";
	case KSLF_Xnor:			return "Xnor";
	case KSLF_Buffer:		return "Buffer";
	case KSLF_Inverter:		return "Inverter";
	case KSLF_DFlipFlop:	return "D Flip Flop";
	default:				return "None";
	}
}

