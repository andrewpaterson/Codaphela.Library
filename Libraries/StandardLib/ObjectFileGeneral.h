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


#define OBJECT_FILE_EXTENSION	"DRG"

//Magic file values.
#define BASIC_OBJECT_FILE		'\0SAB'
#define CHUNKED_OBJECT_FILE		'\0KHC'

#define OBJECT_DATA				'\0TAD'
#define OBJECT_FROM_HEAP		'\0AEH'

#define OBJECT_POINTER_NULL		'\0LUN'
#define OBJECT_POINTER_ID		'\0XDI'
#define OBJECT_POINTER_NAMED	'\0MAN'

#define OBJECT_UNNAMED_FILE		"Unnamed"


#endif // __OBJECT_FILE_GENERAL__

