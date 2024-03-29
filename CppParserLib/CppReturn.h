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
#ifndef __C_P_P_RETURN_H__
#define __C_P_P_RETURN_H__
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"


enum ECPPReturn
{
	RFR_True,
	RFR_FalseNotFound,
};


class CCPPReturn
{
public:
	ECPPReturn	eResult;

	static CCPPReturn True(void)
	{
		CCPPReturn	cReturn;

		cReturn.eResult = RFR_True;
		return cReturn;
	}

	static CCPPReturn NotFound(void)
	{
		CCPPReturn	cReturn;

		cReturn.eResult = RFR_FalseNotFound;
		return cReturn;
	}

	static CCPPReturn InternalError(void)
	{
		gcLogger.Error("Internal Parser Error!\n");
		exit(1);
	}


	bool IsTrue(void);
	bool IsNotFound(void);
};


#endif // !__C_P_P_RETURN_H__

