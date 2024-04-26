/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#if !defined( __AARDVARK_H__ )
#define __AARDVARK_H__

//The Aardvark files exist to test how correct the compilers preprocessor is.
//It begins with a double A to ensure there is a good chance it is compiled first.
#include "Aardvark.h"

#define D3(X) #X
#define D7(P,Q) D3(P) D3(Q)
#define D4() " "
#define D6(P,Q) D3(P)##D4()##D3(Q)
#define D8(P,Q) D3(P) " " D3(Q)
#define D5(X) X
#define D9(P,Q) D5(P) D4() D5(Q)
#define D2(P,Q) D9(#P, #Q)
#define D1(P,Q) #P#Q

#define VERTICAL_INT\
 \
i\
n\
t

#define OLIGAARDVARK\
	AwesomeRussian(\
		int x,\
		int y)


void DoStuff();

#define F1(X) X##__##X


#endif // __AARDVARK_H__

