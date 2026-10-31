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
#ifndef __LOGIC_74_PARTS_H__
#define __LOGIC_74_PARTS_H__
#include "BaseLib/Chars.h"
#include "MicroprocessorLib/GateArray.h"
#include "LogicBlockView.h"


//What a 74 series part is made of, and so which generic chip simulates it.
enum ELogic74Kind
{
	L74K_Gates,
	L74K_Block,
};


//A real 74 series part, by its type after the family: "00" is the 74LVC00 (and the 74HCT00 and so on).  Its units are
//what KiCad draws as units: a gate part's gates, a 74LVC74's flip flops.  szFunction names one unit ("2-Input NAND")
//and a part of gates or of more than one unit is described by its count of them ("Quad 2-Input NAND", "Dual D-Type
//Flip-Flop"); any other part by szFunction alone ("Octal D-Type Latch").  A part of gates is a gate array of
//eGateFunction gates of iInputCount inputs; any other a block (see CLogicBlockView) of eBlock sized by sParams.
//
//A small logic part ("1G08", "2G04") is the part of its type after the G ("08", "04") with the count before the G of
//its units, so it has no row of its own.
struct SLogic74Part
{
	const char*			szPartType;
	const char*			szFunction;
	ELogic74Kind		eKind;
	EGateFunction		eGateFunction;
	int					iUnitCount;
	int					iInputCount;
	const char*			szKeywords;
	ELogicBlockKind		eBlock;
	SLogicBlockParams	sParams;
};


//A part type resolved: its row and how many units it has.
struct SLogic74PartType
{
	SLogic74Part*	psPart;
	int				iUnitCount;
};


size			NumLogic74Parts(void);
SLogic74Part*	GetLogic74Part(size uiIndex);
bool			FindLogic74Part(SLogic74PartType* psType, const char* szPartType);
void			GetLogic74PartNumber(CChars* psz, const char* szFamily, const char* szPartType);
void			GetLogic74Description(CChars* psz, SLogic74PartType* psType);


#endif // __LOGIC_74_PARTS_H__

