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
#ifndef __CALC_EXPRESSION_H__
#define __CALC_EXPRESSION_H__
#include "BaseLib/Define.h"
#include "BaseLib/Number.h"
#include "CalcObject.h"


class CCalcExpression : public CCalcObject
{
public:
	virtual CNumber	Evaluate(void) = 0;
			BOOL 	IsExpression(void);
			BOOL 	IsOperator(void);	
};


#endif // __CALC_ROOT_H__

