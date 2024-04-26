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
#ifndef __NUMBERS_H__
#define __NUMBERS_H__

#include <math.h>
#include "Define.h"

#define SMALL_NUMBER		(1e-5f)
#define LARGE_NUMBER		(1e7f)
#define BINARY_PRECISION	(-16)

#define N_E        2.7182818284590452f
#define N_LOG2E    1.4426950408889634f
#define N_LOG10E   0.4342944819032518f
#define N_LN2      0.6931471805599453f
#define N_LN10     2.3025850929940456f
#define N_PI       3.1415926535897932f
#define N_2_SQRTPI 1.1283791670955125f
#define N_SQRT2    1.4142135623730950f

#define Deg2Rad(x)		((N_PI * x) / 180.0f)

#define KB				*(1024LL)
#define MB				*(1024LL*1024LL)
#define GB				*(1024LL*1024LL*1024LL)

#define CORRUPT_MEMORY	((void*)(16 KB))


#endif // __NUMBERS_H__

