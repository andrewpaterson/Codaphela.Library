/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "TypeNames.h"
#include "PointerRemapper.h"
#include "PointerFunctions.h"
#include "Logger.h"
#include "Int2.h"
#include "Int3.h"
#include "Int4.h"
#include "GeometricTypes.h"
#include "Double2.h"
#include "Double3.h"
#include "Double4.h"
#include "Double4x4.h"
#include "DataTypes.h"
#include "Date.h"
#include "DateTime.h"


CTypeNames	gcTypeNames;
bool		gbTypeNames = false;

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeNames::AddType(EPrimitiveType eType, size iSize, const char* szPrettyName, const char* szCppName, const char* szPrimitiveName)
{
	STypeName*	psTypeName;
	size		iPrettyNameLen;
	size		iCppNameLen;
	size		iPrimitiveNameLen;
	size		iLen;

	psTypeName = (STypeName*)masTypeNames.GrowToAtLeastNumElements(((size)eType) + 1, true, 0);

	psTypeName->eType = eType;
	if (iSize & SIZE_IN_BITS)
	{
		psTypeName->iBitSize = iSize & ~SIZE_IN_BITS;
		psTypeName->iByteSize = 0;
	}
	else
	{
		psTypeName->iByteSize = iSize;
		psTypeName->iBitSize = iSize * 8;
	}

	iPrettyNameLen = (size)strlen(szPrettyName) + 1;
	iCppNameLen = (size)strlen(szCppName) + 1;
	iPrimitiveNameLen = (size)strlen(szPrimitiveName) + 1;

	iLen = iPrettyNameLen + iCppNameLen + iPrimitiveNameLen;
	psTypeName->szPrettyName = (char*)malloc(iLen);
	psTypeName->szCppName = (char*)RemapSinglePointer(psTypeName->szPrettyName, iPrettyNameLen);
	psTypeName->szPrimitiveName = (char*)RemapSinglePointer(psTypeName->szCppName, iCppNameLen);

	strcpy(psTypeName->szPrettyName, szPrettyName);
	strcpy(psTypeName->szCppName, szCppName);
	strcpy(psTypeName->szPrimitiveName, szPrimitiveName);

	if (eType > PT_Undefined)
	{
		mmsziPrettyNames.Put(szPrettyName, eType);
	}
	if (!StrEmpty(szCppName))
	{
		mmsziCppNames.Put(szCppName, eType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeNames::Init(void)
{
	masTypeNames.Init();
	mmsziPrettyNames.Init();
	mmsziCppNames.Init();

	AddType(PT_Undefined,	0,					"Undefined",	"",					"PT_Undefined");
	AddType(PT_int8,		BYTE_BYTE_SIZE,		"Byte",			"int8",				"PT_int8");
	AddType(PT_int16,		SHORT_BYTE_SIZE,	"Short",		"int16",			"PT_int16");
	AddType(PT_int32,		INT_BYTE_SIZE,		"Int",			"int32",			"PT_int32");
	AddType(PT_int64,		LONG_BYTE_SIZE,		"Long",			"int64",			"PT_int64");
	AddType(PT_uint8,		BYTE_BYTE_SIZE,		"UByte",		"uint8",			"PT_uint8");
	AddType(PT_uint16,		SHORT_BYTE_SIZE,	"UShort",		"uint16",			"PT_uint16");
	AddType(PT_uint32,		INT_BYTE_SIZE,		"UInt",			"uint32",			"PT_uint32");
	AddType(PT_uint64,		LONG_BYTE_SIZE,		"ULong",		"uint64",			"PT_uint64");
	AddType(PT_float32,		FLOAT_BYTE_SIZE,	"Float",		"float",			"PT_float32");
	AddType(PT_float64,		DOUBLE_BYTE_SIZE,	"Double",		"double",			"PT_float64");
	AddType(PT_M2float32,	FLOAT2_BYTE_SIZE,	"Float2",		"SFloat2",			"PT_M2float32");
	AddType(PT_M3float32,	FLOAT3_BYTE_SIZE,	"Float3",		"SFloat3",			"PT_M3float32");
	AddType(PT_M4float32,	FLOAT4_BYTE_SIZE,	"Float4",		"SFloat4",			"PT_M4float32");
	AddType(PT_M2float64,	DOUBLE2_BYTE_SIZE,	"Double2",		"SDouble2",			"PT_M2float64");
	AddType(PT_M3float64,	DOUBLE3_BYTE_SIZE,	"Double3",		"SDouble3",			"PT_M3float64");
	AddType(PT_M4float64,	DOUBLE4_BYTE_SIZE,	"Double4",		"SDouble4",			"PT_M4float64");
	AddType(PT_M2int32,		INT2_BYTE_SIZE,		"Int2",			"SInt2",			"PT_M2int32");
	AddType(PT_M3int32,		INT3_BYTE_SIZE,		"Int3",			"SInt3",			"PT_M3int32");
	AddType(PT_M4int32,		INT4_BYTE_SIZE,		"Int4",			"SInt4",			"PT_M4int32");
	AddType(PT_char8,		CHAR_BYTE_SIZE,		"Char",			"char",				"PT_char8");
	AddType(PT_char16,		WIDE_CHAR_BYTE_SIZE,"Widechar",		"wchar_t",			"PT_char16");
	AddType(PT_bit,			BIT_SIZE,			"Bit",			"",					"PT_bit");
	AddType(PT_crumb,		CRUMB_SIZE,			"Crumb",		"",					"PT_crumb");
	AddType(PT_tribble,		TRIBBLE_SIZE,		"Tribble",		"",					"PT_tribble");
	AddType(PT_nybble,		NYBBLE_SIZE,		"Nybble",		"",					"PT_nybble");
	AddType(PT_nickle,		NICKLE_SIZE,		"Nickle",		"",					"PT_nickle");
	AddType(PT_sixbits,		SIXBITS_SIZE,		"Sixbits",		"",					"PT_sixbits");
	AddType(PT_bool,		BOOL_BYTE_SIZE,		"Bool",			"bool",				"PT_bool");
	AddType(PT_Pointer,		sizeof(void*),		"VoidPointer",	"void*",			"PT_Pointer");
	AddType(PT_void,		VOID_BYTE_SIZE,		"Void",			"void",				"PT_void");
	AddType(PT_Data,		BYTE_BYTE_SIZE,		"Data",			"",					"PT_Data");
	AddType(PT_M4x4float32, FLOAT4x4_BYTE_SIZE,	"Float4x4",		"",					"PT_M4x4float32");
	AddType(PT_M4x4float64, DOUBLE4x4_BYTE_SIZE,"Double4x4",	"",					"PT_M4x4float64");
	AddType(PT_String,		0,					"Chars",		"CChars",			"PT_String");
	AddType(PT_Number,		0,					"Chars",		"CChars",			"PT_String");
	AddType(PT_Date,		sizeof(CDate),		"Date",			"CDate",			"PT_Date");
	AddType(PT_DateTime,	sizeof(CDateTime),	"DateTime",		"CDateTime",		"PT_DateTime");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeNames::Kill(void)
{
	size			i;
	STypeName*	psTypeName;
	size			iNumElements;

	mmsziCppNames.Kill();
	mmsziPrettyNames.Kill();

	iNumElements = masTypeNames.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		if (i == 35)
		{
			i = 35;
		}
		psTypeName = masTypeNames.Get(i);
		SafeFree(psTypeName->szPrettyName);
	}
	masTypeNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CTypeNames::GetPrettyName(EPrimitiveType eType)
{
	STypeName*	psTypeName;

	psTypeName = masTypeNames.SafeGet((size)eType);
	if (psTypeName)
	{
		return psTypeName->szPrettyName;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CTypeNames::GetCPPName(EPrimitiveType eType)
{
	STypeName*	psTypeName;

	psTypeName = masTypeNames.SafeGet((size)eType);
	if (psTypeName)
	{
		return psTypeName->szCppName;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CTypeNames::GetPrimitiveName(EPrimitiveType eType)
{
	STypeName*	psTypeName;

	psTypeName = masTypeNames.SafeGet((size)eType);
	if (psTypeName)
	{
		return psTypeName->szPrimitiveName;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CTypeNames::GetTypeFromPrettyName(const char* szPrettyName)
{
	size*	piType;

	piType = (size*)mmsziPrettyNames.CMapStringTemplate::Get(szPrettyName);
	if (piType)
	{
		return (EPrimitiveType)(*piType);
	}
	return PT_Undefined;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CTypeNames::GetTypeFromCPPName(const char* szCppName)
{
	size*	piType;

	piType = (size*)mmsziCppNames.CMapStringTemplate::Get(szCppName);
	if (piType)
	{
		return (EPrimitiveType)(*piType);
	}
	return PT_Undefined;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTypeNames::GetByteSize(EPrimitiveType eType)
{
	STypeName*	psTypeName;

	psTypeName = masTypeNames.SafeGet((size)eType);
	if (psTypeName)
	{
		return psTypeName->iByteSize;
	}
	else
	{
		return 0;
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTypeNames::GetBitSize(EPrimitiveType eType)
{
	STypeName* psTypeName;

	psTypeName = masTypeNames.SafeGet((size)eType);
	if (psTypeName)
	{
		return psTypeName->iBitSize;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TypesInit(void)
{
	if (gbTypeNames)
	{
		gcLogger.Error("Types have already been initialised.");
	}

	gcTypeNames.Init();
	gbTypeNames = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TypesKill(void)
{
	if (!gbTypeNames)
	{
		gcLogger.Error("Types have already been Killed.");
	}

	gcTypeNames.Kill();
	gbTypeNames = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool TypesValidate(void)
{
	if (!gbTypeNames)
	{
		gcLogger.Error("Types has not been initialised.  Call TypesInit().");
		return false;
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintPrimitiveType(EPrimitiveType eType, CChars* psz)
{
	const char* pszTypeName;

	pszTypeName = gcTypeNames.GetPrimitiveName(eType);
	if (pszTypeName)
	{
		psz->Append(pszTypeName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* PrimitiveTypeToString(EPrimitiveType eType)
{
	CChars	sz;
	char*	psz;
	
	sz.Init();
	PrintPrimitiveType(eType, &sz);
	psz = StringToString(sz.Text());
	sz.Kill();

	return psz;
}

