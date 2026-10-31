/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifdef _WIN32
#include "Preprocessor.h"


//////////////////////////////////////////////////////////////////////////
//
// The Microsoft compiler's own macros, as the compiler building this
// defines them (__COUNTER__ is every platform's: Init adds it).  The
// Windows version macros are 0 when the build doesn't set them.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::InitPlatformSpecific(void)
{
	AddSpecialDefine("__FUNCSIG__");
	AddSpecialDefine("__FUNCDNAME__");

	AddDefine("__inline", "inline", &mcDefineReplacementMemory);

#ifdef _M_IX86
	AddNumberDefine("_M_IX86", _M_IX86);
#endif

#ifdef _M_X64
	AddNumberDefine("_M_X64", _M_X64);
	AddNumberDefine("_M_AMD64", _M_X64);
#endif

#ifdef _ATL_VER
	AddNumberDefine("_ATL_VER", _ATL_VER);
#endif

#ifdef _CHAR_UNSIGNED
	AddDefine("_CHAR_UNSIGNED");
#endif

#ifdef __CLR_VER
	AddNumberDefine("__CLR_VER", __CLR_VER);
#endif

#ifdef _DEBUG
	AddDefine("_DEBUG");
#endif

#ifdef _DLL
	AddDefine("_DLL");
#endif

#ifdef _INTEGRAL_MAX_BITS
	AddNumberDefine("_INTEGRAL_MAX_BITS", _INTEGRAL_MAX_BITS);
#endif

#ifdef _MANAGED
	AddDefine("_MANAGED");
#endif

#ifdef _MFC_VER
	AddNumberDefine("_MFC_VER", _MFC_VER);
#endif

#ifdef _MSC_EXTENSIONS
	AddNumberDefine("_MSC_EXTENSIONS", _MSC_EXTENSIONS);
#endif

#ifdef _MSC_VER
	AddNumberDefine("_MSC_VER", _MSC_VER);
#endif

#ifdef _MT
	AddDefine("_MT");
#endif

#ifdef _NATIVE_WCHAR_T_DEFINED
	AddDefine("_NATIVE_WCHAR_T_DEFINED");
#endif

#ifdef _WCHAR_T_DEFINED
	AddDefine("_WCHAR_T_DEFINED");
#endif

#ifdef _WIN32
	AddDefine("_WIN32");
#endif

#ifdef _WIN64
	AddDefine("_WIN64");
#endif

#ifdef _MSC_FULL_VER
	AddNumberDefine("_MSC_FULL_VER", _MSC_FULL_VER);
#endif

#ifdef _WIN32_WINNT
	AddNumberDefine("_WIN32_WINNT", _WIN32_WINNT);
#else
	AddNumberDefine("_WIN32_WINNT", 0);
#endif

#ifdef _WIN32_FUSION
	AddNumberDefine("_WIN32_FUSION", _WIN32_FUSION);
#else
	AddNumberDefine("_WIN32_FUSION", 0);
#endif

#ifdef _WIN32_WINDOWS
	AddNumberDefine("_WIN32_WINDOWS", _WIN32_WINDOWS);
#else
	AddNumberDefine("_WIN32_WINDOWS", 0);
#endif
}


#endif // _WIN32

