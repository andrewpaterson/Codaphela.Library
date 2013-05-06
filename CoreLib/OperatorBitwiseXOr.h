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
#ifndef __OPERATOR_BITWISE_XOR_H__
#define __OPERATOR_BITWISE_XOR_H__
#include "BaseLib/DataTypes.h"


typedef void	(* OperatorBitwiseXOrFunc)		(void* pvDest, void* pvLeft, void* pvRight);


class COperatorBitwiseXOr
{
public:
	OperatorBitwiseXOrFunc mapvBitwiseXOr[PRIMTIVE_OPERATOR_END][PRIMTIVE_OPERATOR_END][PRIMTIVE_OPERATOR_END];

	void Init(void);
	void Kill(void);
	void Do(EPrimitiveTypes eDest, void* pvDest, EPrimitiveTypes eLeft, void* pvLeft, EPrimitiveTypes eRight, void* pvRight);
};


#endif // __OPERATOR_BITWISE_XOR_H__
