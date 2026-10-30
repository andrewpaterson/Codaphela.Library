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

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
//The names and types the array .inl files are written with, for arrays counted with lsize (past 64KB on
//16 bit targets).  Include ArraySizeUndef.inl after the .inl.
#define ARRAY_BLOCK					CArrayLargeBlock
#define ARRAY_TEMPLATE				CArrayLargeTemplate
#define ARRAY_TEMPLATE_PRIMITIVE	CArrayLargeTemplatePrimitive
#define ARRAY_CHAR					CArrayLargeChar
#define CHARS						CLargeChars
#define CHARS_NATURAL				CChars
#define ARRAY_HEADER				SArrayLargeTemplateHeader
#define ASIZE						lsize
#define ARRAY_SIZE_MAX				MAX_LSIZE
#define ARRAY_NOT_FOUND				ARRAY_LARGE_ELEMENT_NOT_FOUND
#define ARRAY_MEMCPY				memcpy
#define ARRAY_MEMMOVE				memmove
#define ARRAY_MEMSET				memset
#define ARRAY_MEMCMP				memcmp
#define ARRAY_MEMSWP				MemSwp
