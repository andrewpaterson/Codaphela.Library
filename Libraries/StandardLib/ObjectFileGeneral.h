/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __OBJECT_FILE_GENERAL__
#define __OBJECT_FILE_GENERAL__
#include "BaseLib/PrimitiveTypes.h"


#define OBJECT_FILE_EXTENSION	"DRG"

//Magic file values: three characters and a zero in a file, as a little endian int32 of them.  A multi-character constant's
//value is up to the compiler.  The shifts are of an int32 as an int can be 16 bits.
#define OBJECT_FILE_MAGIC(a, b, c)	((int32)(a) | ((int32)(b) << 8) | ((int32)(c) << 16))

#define BASIC_OBJECT_FILE		OBJECT_FILE_MAGIC('B', 'A', 'S')
#define CHUNKED_OBJECT_FILE		OBJECT_FILE_MAGIC('C', 'H', 'K')

#define OBJECT_DATA				OBJECT_FILE_MAGIC('D', 'A', 'T')
#define OBJECT_FROM_HEAP		OBJECT_FILE_MAGIC('H', 'E', 'A')

#define OBJECT_POINTER_NULL		OBJECT_FILE_MAGIC('N', 'U', 'L')
#define OBJECT_POINTER_ID		OBJECT_FILE_MAGIC('I', 'D', 'X')
#define OBJECT_POINTER_NAMED	OBJECT_FILE_MAGIC('N', 'A', 'M')

#define OBJECT_UNNAMED_FILE		"Unnamed"


#endif // __OBJECT_FILE_GENERAL__

