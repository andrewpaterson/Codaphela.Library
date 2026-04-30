#ifndef __ASSERT_STATISTICS_H__
#define __ASSERT_STATISTICS_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela TestLib

Codaphela TestLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela TestLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela TestLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Define.h"
#include "BaseLib/PrimitiveTypes.h"


void PrivateBeginTests(char* szFile);
void PrivateTestStatistics();
int  TestTotalStatistics(void);
void InitTotalStatistics(void);

bool Pass(void);
bool Failed(size iLine, char* szFile);
bool Failed(void);


#define TestStatistics()					PrivateTestStatistics()
#define BeginTests()						PrivateBeginTests(__FILE__)
#define Fail()								Validate(Failed(__LINE__, __FILE__))


#endif // __ASSERT_STATISTICS_H__