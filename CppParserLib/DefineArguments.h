/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __DEFINE_ARGUMENTS_H__
#define __DEFINE_ARGUMENTS_H__
#include "PPTokenList.h"


struct SDefineArgument 
{
	int64				mlliDefineID;
	CArrayPPTokenLists	macTokenArguments;
};


typedef CArrayTemplate<SDefineArgument> CArrayDefineArgument;


class CDefineArguments
{
protected:
	CArrayDefineArgument	mcDefineToArguments;

public:
	void 					Init(void);
	void 					Kill(void);
	SDefineArgument*		Get(int64 lliDefineID);
	SDefineArgument*		Add(int64 lliDefineID);
	void					Remove(int64 lliDefineID);
	int						GetDefineArgumentsIndex(SDefineArgument* psArguments);
};


#endif // !__DEFINE_ARGUMENTS_H__

