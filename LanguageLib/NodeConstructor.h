/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __NODE_CONSTRUCTOR_H__
#define __NODE_CONSTRUCTOR_H__
#include <Windows.h>
#include "ElementTypes.h"


struct SNodeDetail
{
	EElement	eNodeType;
	EElement	eModifier;
	int			iNumElementsInNode;
	int			iMinimum;
	int			iMaximum;
	BOOL		bFinal;
	EElement	eNecessity;
};


#define MAX_NODE_NAME 256


class CNodeConstructor
{
public:
	SNodeDetail		msNode;
	char			mszName[MAX_NODE_NAME+1];

	void 		Init(void);
	void 		Kill(void);
	void 		SetName(char* szName);
	void 		SetNumElementsInNode(int iNumElements);
	BOOL		SetType(EElement eNodeType);
	void		SetOrderedIfPossible(void);
	void		SetNecessity(EElement eNecessity);
	void		SetFinal(BOOL bFinal);
	BOOL		ShouldBeAdded(void);
	BOOL		TestForCorrectness(void);
};


#endif //__NODE_CONSTRUCTOR_H__


