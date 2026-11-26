/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __C_P_P_BUILTINS_H__
#define __C_P_P_BUILTINS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "CPPSyntaxes.h"


class CCPPUnit;
struct SCPP65Type;


//GCC's builtin functions the compiler does (CCPPBuiltins::FindBuiltin), for the type checker and the IR front end.
enum ECPPBuiltin
{
	CPPB_Unreachable,		//unreachable and trap.
	CPPB_Pause,			//ia32_pause: nothing.
	CPPB_Expect,
	CPPB_Memcpy,
	CPPB_Alloca,			//An error: frames are fixed in size.
	CPPB_BCDAdd,			//The W65C816's decimal mode.
	CPPB_BCDSubtract,
	CPPB_Ctz,
	CPPB_Clz,
	CPPB_Popcount,
};


//What ctz, clz and popcount count the bits of: unsigned int, unsigned long (l) or unsigned long long (ll).
enum ECPPBuiltinOperand
{
	CPPBO_None,
	CPPBO_UnsignedInt,
	CPPBO_UnsignedLong,
	CPPBO_UnsignedLongLong,
};


struct SCPPBuiltin
{
	const char*				szName;			//Without __builtin_.
	ECPPBuiltin				eBuiltin;
	int						iArguments;
	ECPPBuiltinOperand		eOperand;
	const char*				szDecimalModeOnly;	//The error on a target with no decimal mode (CCPPTarget::HasDecimalMode), NULL if the builtin is every target's.
};



//The compiler's builtins (CCPPUnit::GetBuiltins): GCC's __builtin_ functions it does (gasCPP65Builtins), the floating
//point constants math.h makes of them (FloatBuiltinValue), memcpy copied in place, and the host compiler's
//intrinsics that no header declares.
class CCPPBuiltins
{
protected:
	CCPPUnit*				mpcUnit;

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	bool					IsIntrinsic(int64 lliName);
	bool					IsBuiltin(int64 lliName);
	const SCPPBuiltin*		FindBuiltin(int64 lliName);
	SCPP65Type*				BuiltinOperandType(const SCPPBuiltin* psBuiltin);
	char*					BuiltinName(int64 lliName, int* piLength);
	bool					IsInlineMemcpy(int64 lliName, CArraySyntax* papcArguments);
	bool					FloatBuiltinValue(int64 lliName, size uiArguments, double* pdValue, bool* pbFloat);
};


#endif // __C_P_P_BUILTINS_H__

