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
#ifndef __MAP_STRING_STRING_H__
#define __MAP_STRING_STRING_H__
#include "MapStringTemplate.h"



template class CMapStringTemplate<CChars>;


class CMapStringString : public CMapStringTemplate<CChars>
{
public:
	CChars*	PrivateAllocateNode(char* szKey, char* szValue);
	void	PrivateFreeNode(CChars* psKey);

	void	Kill(void);
	void	Put(CChars* psKey, CChars* psValue);
	void	Put(char* psKey, char* psValue);
	CChars* GetWithKey(CChars* psKey);
	char*	GetWithKey(char* psKey);
	BOOL	GetAtIndex(int iIndex, CChars** ppsKey, CChars** ppsData);
	void	Remove(CChars* szKey);
	void	Remove(char* szKey);

	void	Dump(void);
};


#endif //__MAP_STRING_STRING_H__

