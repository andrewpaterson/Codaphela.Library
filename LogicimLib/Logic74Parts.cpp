/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/StringHelper.h"
#include "Logic74Parts.h"


static SLogic74Part gasLogic74Parts[] =
{
	{ "00",		"2-Input NAND",			L74K_Gates,	GF_Nand,		4,	2,	"nand gate" },
	{ "02",		"2-Input NOR",			L74K_Gates,	GF_Nor,			4,	2,	"nor gate" },
	{ "04",		"Inverter",				L74K_Gates,	GF_Inverter,	6,	1,	"inverter not" },
	{ "08",		"2-Input AND",			L74K_Gates,	GF_And,			4,	2,	"and gate" },
	{ "10",		"3-Input NAND",			L74K_Gates,	GF_Nand,		3,	3,	"nand gate" },
	{ "11",		"3-Input AND",			L74K_Gates,	GF_And,			3,	3,	"and gate" },
	{ "14",		"Schmitt Inverter",		L74K_Gates,	GF_Inverter,	6,	1,	"inverter not schmitt trigger" },
	{ "17",		"Schmitt Buffer",		L74K_Gates,	GF_Buffer,		6,	1,	"buffer schmitt trigger" },
	{ "20",		"4-Input NAND",			L74K_Gates,	GF_Nand,		2,	4,	"nand gate" },
	{ "21",		"4-Input AND",			L74K_Gates,	GF_And,			2,	4,	"and gate" },
	{ "27",		"3-Input NOR",			L74K_Gates,	GF_Nor,			3,	3,	"nor gate" },
	{ "30",		"8-Input NAND",			L74K_Gates,	GF_Nand,		1,	8,	"nand gate" },
	{ "32",		"2-Input OR",			L74K_Gates,	GF_Or,			4,	2,	"or gate" },
	{ "34",		"Buffer",				L74K_Gates,	GF_Buffer,		6,	1,	"buffer driver" },
	{ "86",		"2-Input XOR",			L74K_Gates,	GF_Xor,			4,	2,	"xor exclusive or gate" },
	{ "132",	"2-Input Schmitt NAND",	L74K_Gates,	GF_Nand,		4,	2,	"nand gate schmitt trigger" },
	{ "266",	"2-Input XNOR",			L74K_Gates,	GF_Xnor,		4,	2,	"xnor exclusive nor gate" },
	{ "332",	"3-Input OR",			L74K_Gates,	GF_Or,			1,	3,	"or gate" },

	//					Function									Units		Keywords															Block					Bits Groups Enables Low Inverting ChannelEnables HighImpedance SynchronousLoad OneBased
	{ "74",		"D-Type Flip-Flop",							L74K_Block, GF_And, 2, 0,	"flip flop set reset",										LBK_FlipFlops,		{ 1, 2, 0, 0, false, false, false, false, false } },
	{ "125",	"Quad Three State Buffer",					L74K_Block, GF_And, 1, 0,	"buffer three state tri enable",							LBK_BusBuffer,		{ 1, 4, 1, 0, false, false, false, false, false } },
	{ "138",	"3-to-8 Line Decoder",						L74K_Block, GF_And, 1, 0,	"decoder demultiplexer",									LBK_Decoder,		{ 3, 1, 3, 2, false, false, false, false, false } },
	{ "139",	"Dual 2-to-4 Line Decoder",					L74K_Block, GF_And, 1, 0,	"decoder demultiplexer",									LBK_Decoder,		{ 2, 2, 1, 1, false, false, false, false, false } },
	{ "153",	"Dual 4-Input Multiplexer",					L74K_Block, GF_And, 1, 0,	"multiplexer selector data",								LBK_Multiplexer,	{ 2, 2, 1, 0, false, true, false, false, false } },
	{ "157",	"Quad 2-Input Multiplexer",					L74K_Block, GF_And, 1, 0,	"multiplexer selector data",								LBK_Multiplexer,	{ 1, 4, 1, 0, false, false, false, false, false } },
	{ "161",	"4-Bit Binary Counter",						L74K_Block, GF_And, 1, 0,	"counter binary synchronous",								LBK_Counter,		{ 4, 1, 0, 0, false, false, false, true, false } },
	{ "240",	"Octal Inverting Buffer",					L74K_Block, GF_And, 1, 0,	"buffer line driver inverting three state",					LBK_BusBuffer,		{ 4, 2, 1, 0, true, false, false, false, false } },
	{ "244",	"Octal Buffer",								L74K_Block, GF_And, 1, 0,	"buffer line driver three state",							LBK_BusBuffer,		{ 4, 2, 1, 0, false, false, false, false, false } },
	{ "245",	"Octal Bus Transceiver",					L74K_Block, GF_And, 1, 0,	"transceiver bus bidirectional three state",				LBK_BusTransceiver,	{ 8, 1, 0, 0, false, false, false, false, false } },
	{ "283",	"4-Bit Binary Full Adder",					L74K_Block, GF_And, 1, 0,	"adder sum carry arithmetic",								LBK_Adder,			{ 4, 1, 0, 0, false, false, false, false, true } },
	{ "373",	"Octal D-Type Latch",						L74K_Block, GF_And, 1, 0,	"latch transparent three state",							LBK_Latch,			{ 8, 1, 0, 0, false, false, false, false, false } },
	{ "374",	"Octal D-Type Register",					L74K_Block, GF_And, 1, 0,	"register flip flop edge three state",						LBK_Register,		{ 8, 1, 0, 0, false, false, false, false, false } },
	{ "521",	"8-Bit Identity Comparator",				L74K_Block, GF_And, 1, 0,	"comparator equality identity",								LBK_Comparator,		{ 8, 1, 0, 0, false, false, false, false, false } },
	{ "540",	"Octal Inverting Buffer",					L74K_Block, GF_And, 1, 0,	"buffer line driver inverting three state",					LBK_BusBuffer,		{ 8, 1, 2, 0, true, false, false, false, false } },
	{ "541",	"Octal Buffer",								L74K_Block, GF_And, 1, 0,	"buffer line driver three state",							LBK_BusBuffer,		{ 8, 1, 2, 0, false, false, false, false, false } },
	{ "573",	"Octal D-Type Latch",						L74K_Block, GF_And, 1, 0,	"latch transparent three state",							LBK_Latch,			{ 8, 1, 0, 0, false, false, false, false, false } },
	{ "574",	"Octal D-Type Register",					L74K_Block, GF_And, 1, 0,	"register flip flop edge three state",						LBK_Register,		{ 8, 1, 0, 0, false, false, false, false, false } },
	{ "688",	"8-Bit Identity Comparator",				L74K_Block, GF_And, 1, 0,	"comparator equality identity",								LBK_Comparator,		{ 8, 1, 0, 0, false, false, false, false, false } },
	{ "3253",	"Dual 1-of-4 Bus Switch",					L74K_Block, GF_And, 1, 0,	"multiplexer bus switch quickswitch",						LBK_Multiplexer,	{ 2, 2, 1, 0, false, true, true, false, false } },
	{ "16373",	"16-Bit D-Type Latch",						L74K_Block, GF_And, 1, 0,	"latch transparent three state",							LBK_Latch,			{ 8, 2, 0, 0, false, false, false, false, false } },
	{ "16374",	"16-Bit D-Type Register",					L74K_Block, GF_And, 1, 0,	"register flip flop edge three state",						LBK_Register,		{ 8, 2, 0, 0, false, false, false, false, false } },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size NumLogic74Parts(void)
{
	return sizeof(gasLogic74Parts) / sizeof(SLogic74Part);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLogic74Part* GetLogic74Part(size uiIndex)
{
	if (uiIndex >= NumLogic74Parts())
	{
		return NULL;
	}
	return &gasLogic74Parts[uiIndex];
}


//////////////////////////////////////////////////////////////////////////
//
// A small logic part ("2G04") is the part after the G with the count
// before it.  False for a part type there's no row for.
//
//////////////////////////////////////////////////////////////////////////
bool FindLogic74Part(SLogic74PartType* psType, const char* szPartType)
{
	size		i;
	const char*	szBase;
	int			iUnitCount;

	psType->psPart = NULL;
	psType->iUnitCount = 0;
	if (StrEmpty(szPartType))
	{
		return false;
	}

	szBase = szPartType;
	iUnitCount = 0;
	if ((szPartType[0] >= '1') && (szPartType[0] <= '9') && (szPartType[1] == 'G'))
	{
		iUnitCount = szPartType[0] - '0';
		szBase = &szPartType[2];
	}

	for (i = 0; i < NumLogic74Parts(); i++)
	{
		if (strcmp(gasLogic74Parts[i].szPartType, szBase) == 0)
		{
			psType->psPart = &gasLogic74Parts[i];
			psType->iUnitCount = (iUnitCount > 0) ? iUnitCount : gasLogic74Parts[i].iUnitCount;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// 74, the family and the type: "74LVC00".
//
//////////////////////////////////////////////////////////////////////////
void GetLogic74PartNumber(CChars* psz, const char* szFamily, const char* szPartType)
{
	psz->Append("74");
	psz->Append(szFamily);
	psz->Append(szPartType);
}


//////////////////////////////////////////////////////////////////////////
//
// Its count of units and what they are, "Quad 2-Input NAND", or for a
// part of one unit that isn't gates what it is, "Octal D-Type Latch".
//
//////////////////////////////////////////////////////////////////////////
void GetLogic74Description(CChars* psz, SLogic74PartType* psType)
{
	const char*		aszCounts[] = { "", "Single", "Dual", "Triple", "Quad", "", "Hex", "", "Octal" };

	if (((psType->psPart->eKind == L74K_Gates) || (psType->iUnitCount > 1)) && (psType->iUnitCount >= 1) && (psType->iUnitCount <= 8) && !StrEmpty(aszCounts[psType->iUnitCount]))
	{
		psz->Append(aszCounts[psType->iUnitCount]);
		psz->Append(' ');
	}
	psz->Append(psType->psPart->szFunction);
}

