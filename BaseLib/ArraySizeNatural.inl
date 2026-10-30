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
//The names and types the array .inl files are written with, for arrays counted with size (the natural
//width).  Include ArraySizeUndef.inl after the .inl.
#define ARRAY_BLOCK					CArrayBlock
#define ARRAY_TEMPLATE				CArrayTemplate
#define ARRAY_TEMPLATE_PRIMITIVE	CArrayTemplatePrimitive
#define ARRAY_CHAR					CArrayChar
#define CHARS						CChars
#define ARRAY_HEADER				SArrayTemplateHeader
#define ASIZE						size
#define ARRAY_SIZE_MAX				MAX_SIZE
#define ARRAY_NOT_FOUND				ARRAY_ELEMENT_NOT_FOUND

//At most MAX_SIZE bytes are moved, so on 16 bit targets the moves take 16 bit counts (Libc65's string816_16.asm).
#if defined(__W65C816__)
#define ARRAY_MEMCPY				memcpy16
#define ARRAY_MEMMOVE				memmove16
#define ARRAY_MEMSET				memset16
#define ARRAY_MEMCMP				memcmp16
#define ARRAY_MEMSWP				MemSwp16
#else
#define ARRAY_MEMCPY				memcpy
#define ARRAY_MEMMOVE				memmove
#define ARRAY_MEMSET				memset
#define ARRAY_MEMCMP				memcmp
#define ARRAY_MEMSWP				MemSwp
#endif // __W65C816__

//With a 16 bit size a natural array holds at most MAX_SIZE bytes.  Past that it fails as if out of memory.
#if defined(SIZE_16)
#define ARRAY_CHECK_SIZE
#endif // SIZE_16
