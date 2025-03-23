/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "Operators.h"


COperators gcOperators;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperators::Init(void)
{
	mcAdd.Init();
	mcSub.Init();
	mcIncrement.Init();
	mcDecrement.Init();
	mcMultiply.Init();
	mcDivide.Init();
	mcRemainder.Init();
	mcPower.Init();

	mcEqual.Init();
	mcNotEqual.Init();
	mcLess.Init();
	mcLessOrEqual.Init();
	mcGreater.Init();
	mcGreaterOrEqual.Init();

	mcBwAnd.Init();
	mcLeftShift.Init();
	mcBwNot.Init();
	mcBwOr.Init();
	mcRightShift.Init();
	mcBwXOr.Init();

	mcLgAnd.Init();
	mcLgNot.Init();
	mcLgOr.Init();

	mcAssignment.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperators::Kill(void)
{
	mcAdd.Kill();
	mcSub.Kill();
	mcIncrement.Kill();
	mcDecrement.Kill();
	mcMultiply.Kill();
	mcDivide.Kill();
	mcRemainder.Kill();
	mcPower.Kill();

	mcEqual.Kill();
	mcNotEqual.Kill();
	mcLess.Kill();
	mcLessOrEqual.Kill();
	mcGreater.Kill();
	mcGreaterOrEqual.Kill();

	mcBwAnd.Kill();
	mcLeftShift.Kill();
	mcBwNot.Kill();
	mcBwOr.Kill();
	mcRightShift.Kill();
	mcBwXOr.Kill();

	mcLgAnd.Kill();
	mcLgNot.Kill();
	mcLgOr.Kill();

	mcAssignment.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void OperatorsInit(void)
{
	gcOperators.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void OperatorsKill(void)
{
	float f = 0;
	
	gcOperators.Kill();
}
