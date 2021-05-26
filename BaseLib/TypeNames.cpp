/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "DataTypes.h"


CTypeNames	gcTypeNames;
BOOL		gbTypeNames = FALSE;

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeNames::AddType(EPrimitiveType eType, int iSize, const char* szPrettyName, const char* szCppName, const char* szPrimitiveName)
{
	STypeName*	psTypeName;
	int			iPrettyNameLen;
	int			iCppNameLen;
	int			iPrimitiveNameLen;
	int			iLen;

	psTypeName = (STypeName*)masTypeNames.GrowToAtLeastNumElements(((int)eType)+1, TRUE, 0);

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

	iPrettyNameLen = (int)strlen(szPrettyName) + 1;
	iCppNameLen = (int)strlen(szCppName) + 1;
	iPrimitiveNameLen = (int)strlen(szPrimitiveName) + 1;

	iLen = iPrimitiveNameLen + iCppNameLen + iPrimitiveNameLen;
	psTypeName->szPrettyName = (char*)malloc(iLen);
	psTypeName->szCppName = (char*)RemapSinglePointer(psTypeName->szPrettyName, iPrettyNameLen);
	psTypeName->szPrimitiveName = (char*)RemapSinglePointer(psTypeName->szCppName, iCppNameLen);

	strcpy(psTypeName->szPrettyName, szPrettyName);
	strcpy(psTypeName->szCppName, szCppName);
	strcpy(psTypeName->szPrimitiveName, szPrimitiveName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeNames::Init(void)
{
	masTypeNames.Init();
	AddType(PT_Undefined,	0,					"Undefined",	"",					"PT_Undefined");
	AddType(PT_Pointer,		sizeof(void*),		"Pointer",		"void*",			"PT_Pointer");
	AddType(PT_int8,		BYTE_BYTE_SIZE,		"Byte",			"int8",				"PT_int8");
	AddType(PT_int16,		SHORT_BYTE_SIZE,	"Short",		"int16",			"PT_int16");
	AddType(PT_int32,		INT_BYTE_SIZE,		"Int",			"int32",			"PT_int32");
	AddType(PT_int64,		LONG_BYTE_SIZE,		"Long",			"long long int",	"PT_int64");
	AddType(PT_uint8,		CHAR_BYTE_SIZE,		"UByte",		"unsigned char",	"PT_uint8");
	AddType(PT_uint16,		SHORT_BYTE_SIZE,	"UShort",		"uint16",			"PT_uint16");
	AddType(PT_uint32,		INT_BYTE_SIZE,		"UInt",			"unsigned int",		"PT_uint32");
	AddType(PT_uint64,		LONG_BYTE_SIZE,		"ULong",		"unsigned long long int", "PT_uint64");
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
	AddType(PT_bool,		BOOL_BYTE_SIZE,		"Bool",			"BOOL",				"PT_bool");
	AddType(PT_void,		VOID_BYTE_SIZE,		"Void",			"void",				"PT_void");
	AddType(PT_char8,		CHAR_BYTE_SIZE,		"Char",			"char",				"PT_char8");
	AddType(PT_char16,		WIDE_CHAR_BYTE_SIZE,"Widechar",		"wchar_t",			"PT_char16");
	AddType(PT_bit,			BIT_SIZE,			"Bit",			"",					"PT_bit");
	AddType(PT_crumb,		CRUMB_SIZE,			"Crumb",		"",					"PT_crumb");
	AddType(PT_tribble,		TRIBBLE_SIZE,		"Tribble",		"",					"PT_tribble");
	AddType(PT_nybble,		NYBBLE_SIZE,		"Nybble",		"",					"PT_nybble");
	AddType(PT_nickle,		NICKLE_SIZE,		"Nickle",		"",					"PT_nickle");
	AddType(PT_sixbits,		SIXBITS_SIZE,		"Sixbits",		"",					"PT_sixbits");

	mmsziPrettyNames.Init();
	mmsziPrettyNames.Put("Int", PT_int32);
	mmsziPrettyNames.Put("Bool", PT_bool);
	mmsziPrettyNames.Put("Short", PT_int16);
	mmsziPrettyNames.Put("Void", PT_void);
	mmsziPrettyNames.Put("Pointer", PT_Pointer);
	mmsziPrettyNames.Put("Char", PT_char8);
	mmsziPrettyNames.Put("Widechar", PT_char16);
	mmsziPrettyNames.Put("Float", PT_float32);
	mmsziPrettyNames.Put("Double", PT_float64);
	mmsziPrettyNames.Put("UInt", PT_uint32);
	mmsziPrettyNames.Put("UShort", PT_uint16);
	mmsziPrettyNames.Put("UChar", PT_uint8);
	mmsziPrettyNames.Put("Long", PT_int64);
	mmsziPrettyNames.Put("ULong", PT_uint64);
	mmsziPrettyNames.Put("Float2", PT_M2float32);
	mmsziPrettyNames.Put("Float3", PT_M3float32);
	mmsziPrettyNames.Put("Float4", PT_M4float32);
	mmsziPrettyNames.Put("Double2", PT_M2float64);
	mmsziPrettyNames.Put("Double3", PT_M3float64);
	mmsziPrettyNames.Put("Double4", PT_M4float64);
	mmsziPrettyNames.Put("Int2", PT_M2int32);
	mmsziPrettyNames.Put("Int3", PT_M3int32);
	mmsziPrettyNames.Put("Int4", PT_M4int32);
	mmsziPrettyNames.Put("Bit", PT_bit);
	mmsziPrettyNames.Put("Crumb", PT_crumb);
	mmsziPrettyNames.Put("Tribble", PT_tribble);
	mmsziPrettyNames.Put("Nybble", PT_nybble);
	mmsziPrettyNames.Put("Nickle", PT_nickle);
	mmsziPrettyNames.Put("Sixbits", PT_sixbits);
	
	mmsziCppNames.Init();
	mmsziCppNames.Put("int", PT_int32);
	mmsziCppNames.Put("BOOL", PT_bool);
	mmsziCppNames.Put("int16", PT_int16);
	mmsziCppNames.Put("int8", PT_int8);
	mmsziCppNames.Put("void", PT_void);
	mmsziCppNames.Put("void*", PT_Pointer);
	mmsziCppNames.Put("char", PT_char8);
	mmsziCppNames.Put("wchar_t", PT_char16);
	mmsziCppNames.Put("float", PT_float32);
	mmsziCppNames.Put("double", PT_float64);
	mmsziCppNames.Put("unsigned int", PT_uint32);
	mmsziCppNames.Put("uint16", PT_uint16);
	mmsziCppNames.Put("unsigned char", PT_uint8);
	mmsziCppNames.Put("long long int", PT_int64);
	mmsziCppNames.Put("unsigned long long int", PT_uint64);
	mmsziCppNames.Put("SFloat2", PT_M2float32);
	mmsziCppNames.Put("SFloat3", PT_M3float32);
	mmsziCppNames.Put("SFloat4", PT_M4float32);
	mmsziCppNames.Put("SDouble2", PT_M2float64);
	mmsziCppNames.Put("SDouble3", PT_M3float64);
	mmsziCppNames.Put("SDouble4", PT_M4float64);
	mmsziCppNames.Put("SInt2", PT_M2int32);
	mmsziCppNames.Put("SInt3", PT_M3int32);
	mmsziCppNames.Put("SInt4", PT_M4int32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeNames::Kill(void)
{
	int			i;
	STypeName*	psTypeName;

	mmsziCppNames.Kill();
	mmsziPrettyNames.Kill();

	for (i = 0; i < masTypeNames.NumElements(); i++)
	{
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

	psTypeName = masTypeNames.SafeGet((int)eType);
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

	psTypeName = masTypeNames.SafeGet((int)eType);
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

	psTypeName = masTypeNames.SafeGet((int)eType);
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
	int*	piType;

	piType = mmsziPrettyNames.Get(szPrettyName);
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
	int*	piType;

	piType = mmsziCppNames.Get(szCppName);
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
int CTypeNames::GetByteSize(EPrimitiveType eType)
{
	STypeName*	psTypeName;

	psTypeName = masTypeNames.SafeGet((int)eType);
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
int CTypeNames::GetBitSize(EPrimitiveType eType)
{
	STypeName*	psTypeName;

	psTypeName = masTypeNames.SafeGet((int)eType);
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
	gbTypeNames = TRUE;
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
	gbTypeNames = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL TypesValidate(void)
{
	if (!gbTypeNames)
	{
		gcLogger.Error("Types has not been initialised.  Call TypesInit().");
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

