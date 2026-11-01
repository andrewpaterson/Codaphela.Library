/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __UNITS_H__
#define __UNITS_H__
#include "LongTime.h"


//Logicim's Units.  Frequencies are in hertz; NANOSECONDS_IN_SECOND is in LongTime.h.
#define FREQUENCY_GHZ					1000000000.0f
#define FREQUENCY_MHZ					1000000.0f
#define FREQUENCY_KHZ					1000.0f
#define FREQUENCY_HZ					1.0f

#define NANOSECONDS_IN_MILLISECOND		1000000LL


#endif // __UNITS_H__

