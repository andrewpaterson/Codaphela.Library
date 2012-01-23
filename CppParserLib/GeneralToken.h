/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __GENERAL_TOKEN_H__
#define __GENERAL_TOKEN_H__
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ConstructorCall.h"


//The token memory should be moved out to each preprocessed file etc... but... later.


extern CMemoryStackExtended gcTokenMemory;
extern CMemoryStackExtended gcTokenStrings;


#define TOKEN_CONSTRUCT(p) 	static p* Construct(void* pvMem)\
{\
	CONSTRUCT(pvMem, p);\
	return (p*)pvMem;\
}


#define ADD_TOKEN(p, q, mem)	(p*)AddToken(p::Construct(mem), q)


void InitTokenMemory(void);
void KillTokenMemory(void);


#endif // __GENERAL_TOKEN_H__

