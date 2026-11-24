/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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

** ------------------------------------------------------------------------ **/
#ifndef __HASH_SET_STRING_H__
#define __HASH_SET_STRING_H__
#include "HashSetTemplate.h"
#include "HashStringBase.h"


//A set of text by a hash of it (HASH: SHashOfText, or SHashOfTextIgnoreCase or SHashOfFileName), which keeps its own
//copy of each (see CHashStringBase): text interned, each an index (FindOrAddIndex) and its text again (GetKey).
template<class SIZE = SArrayNaturalSize, class HASH = SHashOfText>
class CHashSetString : public CHashStringBase<CHashSetTemplate<SHashText, HASH, SIZE>, SIZE>
{
};


#endif // __HASH_SET_STRING_H__
