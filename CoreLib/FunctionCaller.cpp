/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "FunctionCaller.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFunctionPointer GetThisCallFunction(ThisCall_Void_Void function)
{
	SFunctionPointer	sFunction;

	sFunction.thisCall = function;
	return sFunction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFunctionPointer GetThisMultiCallFunction(ThisMultiCall_Void_Void function)
{
	SFunctionPointer	sFunction;

	sFunction.thisMultiCall = function;
	return sFunction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFunctionPointer GetCCallFunction(CCall_Void_Void function)
{
	SFunctionPointer	sFunction;

	sFunction.cCall = function;
	return sFunction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFunctionPointer GetNULLCallFunction(void)
{
	SFunctionPointer	sFunction;

	sFunction.cCall = 0;
	return sFunction;
}


BOOL SFunctionPointer::IsNull(void)
{
	return cCall == NULL;
}


void SFunctionPointer::SetNull(void)
{
	cCall = NULL;
}

