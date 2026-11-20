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
#ifndef __C_P_P_EXCEPTION_SPEC_H__
#define __C_P_P_EXCEPTION_SPEC_H__
#include "BaseLib/PrimitiveTypes.h"
#include "CPP65Type.h"


//What an exception specification allows.
enum ECPPExceptionSpecKind
{
	CPPES_Anything,	//None written, noexcept(false), or Microsoft's throw(...).
	CPPES_Nothing,	//noexcept, noexcept(true) or throw().
	CPPES_Types,	//throw(A, B): the types of mapsTypes, and classes derived from them.
};


//A function's exception specification ([except.spec]): what may leave it.  A function has one whether or not
//it is written; a pointer to a function has the one its declarator was written with.
class CCPPExceptionSpec
{
public:
	ECPPExceptionSpecKind	meKind;
	CArrayCPP65Types		mapsTypes;
	bool					mbDynamic;		//Written throw(A, B) or throw(): an exception it doesn't allow calls std::unexpected, not std::terminate.

	void	Init(void);
	void	Kill(void);

	void	SetAnything(void);
	void	SetNothing(void);
	void	AddType(SCPP65Type* psType);
	void	Set(CCPPExceptionSpec* pcSource);
	void	Add(CCPPExceptionSpec* pcOther);

	bool	IsNonThrowing(void) const;
	bool	IsAnything(void) const;
	bool	HasType(SCPP65Type* psType);
	bool	IsCompatible(CCPPExceptionSpec* pcOther);
};


#endif // __C_P_P_EXCEPTION_SPEC_H__
