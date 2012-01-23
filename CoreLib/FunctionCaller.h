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
#ifndef __NEW_FUNCTION_CALLER_H__
#define __NEW_FUNCTION_CALLER_H__
#include "BaseLib/Define.h"


class CFunctionCaller
{
public:
};


class CFunctionCallerSuper1
{
public:
};


class CFunctionCallerSuper2
{
public:
};


class CFunctionCallerMultipleInheritance : public CFunctionCallerSuper1, public CFunctionCallerSuper2
{
public:
};


#include "FunctionCaller_ThisCall.inl"
#include "FunctionCaller_ThisMultiCall.inl"
#include "FunctionCaller_C_Call.inl"


struct SFunctionPointer
{
	union
	{
		ThisMultiCall_Void_Void		thisMultiCall;
		ThisCall_Void_Void			thisCall;
		CCall_Void_Void				cCall;
	};

	BOOL IsNull(void);
	void SetNull(void);
};


SFunctionPointer GetThisCallFunction(ThisCall_Void_Void function);
SFunctionPointer GetThisMultiCallFunction(ThisMultiCall_Void_Void function);
SFunctionPointer GetCCallFunction(CCall_Void_Void function);
SFunctionPointer GetNULLCallFunction(void);


#define GetThisCall(p) GetThisCallFunction((ThisCall_Void_Void)p)
#define GetThisMultiCall(p) GetThisMultiCallFunction((ThisMultiCall_Void_Void)p)
#define GetCCall(p) GetCCallFunction((CCall_Void_Void)p)
#define GetNULLCall GetNULLCallFunction()


#endif // __NEW_FUNCTION_CALLER_H__

