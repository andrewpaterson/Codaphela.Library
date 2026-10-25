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
#ifndef __KICAD_SCHEM_LOGIC_FUNCTION_H__
#define __KICAD_SCHEM_LOGIC_FUNCTION_H__
#include "BaseLib/Chars.h"


//What one gate of a 74 series logic part does.
enum EKicadSchemLogicFunction
{
	KSLF_None,
	KSLF_And,
	KSLF_Nand,
	KSLF_Or,
	KSLF_Nor,
	KSLF_Xor,
	KSLF_Xnor,
	KSLF_Buffer,
	KSLF_Inverter,
	KSLF_DFlipFlop
};


//The function of a 74 series part from its part number, as "SN74LVC1G08DBVR", "74HC00" or "Nexperia:74LVC74APW,118":
//"74", then the family's letters, then "1G", "2G" or "3G" for a small part, then the function's digits.  The family is
//one Logicim has propagation times for (see the family voltage configurations) or LVC.  KSLF_None if it isn't a 74
//series gate or flip flop Logicim has.
EKicadSchemLogicFunction	ParseKicadSchemLogicFunction(const char* szPartNumber, CChars* pszFamily);
const char*					KicadSchemLogicFunctionName(EKicadSchemLogicFunction eFunction);
bool						IsKicadSchemLogicimFamily(const char* szFamily);


#endif // __KICAD_SCHEM_LOGIC_FUNCTION_H__

