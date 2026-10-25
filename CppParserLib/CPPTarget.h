/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __C_P_P_TARGET_H__
#define __C_P_P_TARGET_H__
#include "BaseLib/PrimitiveTypes.h"


enum ECPPTarget
{
	CPPT_65C64,
	CPPT_W65C816,
	CPPT_NumTargets
};


//What the C++ compiler needs to know about the processor it compiles for (W65C816.txt step 5).  The generator, the IR
//front end and CCPP65Builder take it from here instead of assuming the 65C64.
struct SCPPTarget
{
	ECPPTarget		eTarget;
	const char*		szName;				//The project configuration's name.
	const char*		szDefine;			//The predefined macro: __65C64__ or __W65C816__.
	int				iIntBytes;			//int and unsigned int: 4 on the 65C64, 2 on the W65C816.  long is 4 on both.
	int				iPointerBytes;		//A pointer's size in memory (a W65C816 pointer is 24 bits and a zero byte).
	bool			bWidenResults;		//Results narrower than 32 bits are returned as i32 (the 65C64's 32 bit A).
	bool			bDirectGenerator;	//Functions the IR front end can't write are written by the generator itself.
	uint32			uiBankSize;			//The linker keeps sections inside banks of this size (0 for none).
	uint32			uiStackTop;			//S as the start up sets it.
};


SCPPTarget*	GetCPPTarget(ECPPTarget eTarget);


#endif // __C_P_P_TARGET_H__

