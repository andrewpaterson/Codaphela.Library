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
#ifndef __OPERATOR_ASSIGNMENT_H__
#define __OPERATOR_ASSIGNMENT_H__
#include "DataTypes.h"


//This is actually a *casting* operator for when you don't need to perform an operation on the data.
typedef void	(* OperatorAssignmentFunc)		(void* pvDest, void* pvLeft);


class COperatorAssignment
{
public:
	OperatorAssignmentFunc mapvAssignment[PRIMTIVE_OPERATOR_END][PRIMTIVE_OPERATOR_END];

	void Init(void);
	void Kill(void);
	void Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eLeft, void* pvLeft);
};


#endif // __OPERATOR_REMAINDER_H__

