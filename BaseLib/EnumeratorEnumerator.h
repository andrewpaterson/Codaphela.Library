/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __ENUMERATOR_ENUMERATOR_H__
#define __ENUMERATOR_ENUMERATOR_H__
#include "EnumeratorTemplate.h"
#include "EnumeratorVoid.h"


typedef CEnumeratorTemplate<CEnumeratorVoid> __CEnumeratorEnumerator;
class CEnumeratorEnumerator : public __CEnumeratorEnumerator
{
public:
	int					AddType(char* szType);
	int					AddType(char* szType, int iTypeLen);
	int					Add(int iType, char* szText);
	void				Add(char* szType, char* szText);
	char*				Get(int iType, int iID);
	void				Kill(void);
	CEnumeratorVoid*	GetEnumerator(int iType);
	CEnumeratorVoid*	GetEnumerator(char* szType);
	CEnumeratorVoid*	GetEnumerator(char* szType, int iTypeLen);
	char*				GetTypeName(int iType);
};



#endif //__ENUMERATOR_ENUMERATOR_H__

