/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::InitPlatformSpecific(void)
{
	CChars	sz;

	AddSpecialDefine("__FUNCSIG__");
	AddSpecialDefine("__COUNTER__");
	AddSpecialDefine("__FUNCDNAME__");

	AddDefine("_M_IX86");
	AddDefine("__inline", "inline");

#ifdef _ATL_VER
	AddDefine("_ATL_VER", _ATL_VER);
#endif

#ifdef _CHAR_UNSIGNED
	AddDefine("_CHAR_UNSIGNED");
#endif

#ifdef __CLR_VER
	AddDefine("__CLR_VER", __CLR_VER);
#endif

#ifdef _DEBUG
	AddDefine("_DEBUG");
#endif

#ifdef _DLL
	AddDefine("_DLL");
#endif

#ifdef _INTEGRAL_MAX_BITS
	sz.Init();
	sz.Append(_INTEGRAL_MAX_BITS);
	AddDefine("_INTEGRAL_MAX_BITS", sz.Text());
	sz.Kill();
#endif


#ifdef _MANAGED
	AddDefine("_MANAGED");
#endif

#ifdef _MFC_VER
	AddDefine("_MFC_VER", _MFC_VER);
#endif

#ifdef _MSC_EXTENSIONS
	sz.Init();
	sz.Append(_MSC_EXTENSIONS);
	AddDefine("_MSC_EXTENSIONS", sz.Text());
	sz.Kill();
#endif

#ifdef _MSC_VER
	sz.Init();
	sz.Append(_MSC_VER);
	AddDefine("_MSC_VER", sz.Text());
	sz.Kill();
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
	sz.Init();
	sz.Append(_MSC_FULL_VER);
	AddDefine("_MSC_FULL_VER", sz.Text());
	sz.Kill();
#endif

#ifdef _WIN32_WINNT
	sz.Init();
	sz.Append(_WIN32_WINNT);
	AddDefine("_WIN32_WINNT", sz.Text());
	sz.Kill();
#else
	AddDefine("_WIN32_WINNT", "0");
#endif


#ifdef _WIN32_FUSION
	sz.Init();
	sz.Append(_WIN32_FUSION);
	AddDefine("_WIN32_FUSION", sz.Text());
	sz.Kill();
#else
	AddDefine("_WIN32_FUSION", "0");
#endif

#ifdef _WIN32_WINDOWS
	sz.Init();
	sz.Append(_WIN32_WINDOWS);
	AddDefine("_WIN32_WINDOWS", sz.Text());
	sz.Kill();
#else
	AddDefine("_WIN32_WINDOWS", "0");
#endif
}


#endif // _WIN32

