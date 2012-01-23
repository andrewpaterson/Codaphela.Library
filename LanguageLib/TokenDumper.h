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
#ifndef __TOKEN_DUMPER_H__
#define __TOKEN_DUMPER_H__
#include "BaseLib/EnumeratorVoid.h"


void DumpTokens(int* piTokenArray, int iLength, CEnumeratorVoid* pcKeywords, CEnumeratorVoid* pcIdentifiers);
void DumpTokenSingle(void* pi);
void DumpTokenIdentifier(void* pi, CEnumeratorVoid* pcIdentifiers);
void DumpTokenKeywordText(void* pi, CEnumeratorVoid* pcKeywords);
void DumpTokenKeywordChar(void* pi);
void DumpTokenInteger(void* pi);
void DumpTokenReal(void* pi);
void DumpTokenUnknownText(void* pi);
void DumpTokenUnknownChar(void* pi);

#endif //__TOKEN_DUMPER_H__

