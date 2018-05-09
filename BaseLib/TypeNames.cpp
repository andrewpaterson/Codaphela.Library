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
#include "DataTypes.h"


CTypeNames gcTypeNames;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeNames::AddType(EPrimitiveTypes eType, int iSize, const char* szPrettyName, const char* szCppName, const char* szPrimitiveName)
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
	AddType(PT_int,			INT_BYTE_SIZE,		"Int",			"int",				"PT_int");
	AddType(PT_short,		SHORT_BYTE_SIZE,	"Short",		"short",			"PT_short");
	AddType(PT_VoidPointer, sizeof(void*),		"Pointer",		"void*",			"PT_VoidPointer");
	AddType(PT_char,		CHAR_BYTE_SIZE,		"Char",			"char",				"PT_char");
	AddType(PT_float,		FLOAT_BYTE_SIZE,	 "Float",		"float",			"PT_float");
	AddType(PT_double,		DOUBLE_BYTE_SIZE,	"Double",		"double",			"PT_double");
	AddType(PT_uint,		INT_BYTE_SIZE,		"UInt",			"unsigned int",		"PT_uint");
	AddType(PT_ushort,		SHORT_BYTE_SIZE,	"UShort",		"unsigned short",	"PT_ushort");
	AddType(PT_uchar,		CHAR_BYTE_SIZE,		"UChar",		"unsigned char",	"PT_uchar");
	AddType(PT_long,		LONG_BYTE_SIZE,		"Long",			"long long int",	"PT_long");
	AddType(PT_ulong,		LONG_BYTE_SIZE,		"ULong",		"unsigned long long int", "PT_ulong");
	AddType(PT_float2,		FLOAT2_BYTE_SIZE,	"Float2",		"SFloat2",			"PT_float2");
	AddType(PT_float3,		FLOAT3_BYTE_SIZE,	"Float3",		"SFloat3",			"PT_float3");
	AddType(PT_float4,		FLOAT4_BYTE_SIZE,	"Float4",		"SFloat4",			"PT_float4");
	AddType(PT_int2,		INT2_BYTE_SIZE,		"Int2",			"SInt2",			"PT_int2");
	AddType(PT_int3,		INT3_BYTE_SIZE,		"Int3",			"SInt3",			"PT_int3");
	AddType(PT_int4,		INT4_BYTE_SIZE,		"Int4",			"SInt4",			"PT_int4");
	AddType(PT_bool,		BOOL_BYTE_SIZE,		"Bool",			"BOOL",				"PT_bool");
	AddType(PT_void,		VOID_BYTE_SIZE,		"Void",			"void",				"PT_void");
	AddType(PT_bit,			BIT_SIZE,			"Bit",			"",					"PT_bit");
	AddType(PT_crumb,		CRUMB_SIZE,			"Crumb",		"",					"PT_crumb");
	AddType(PT_nybble,		NYBBLE_SIZE,		"Nybble",		"",					"PT_nybble");
	AddType(PT_nickle,		NICKLE_SIZE,		"Nickle",		"",					"PT_nickle");
	AddType(PT_sixbits,		SIXBITS_SIZE,		"Sixbits",		"",					"PT_sixbits");

	mmsziPrettyNames.Init(30, TRUE);
	mmsziPrettyNames.Put("Int", PT_int);
	mmsziPrettyNames.Put("Bool", PT_bool);
	mmsziPrettyNames.Put("Short", PT_short);
	mmsziPrettyNames.Put("Void", PT_void);
	mmsziPrettyNames.Put("Pointer", PT_VoidPointer); //?
	mmsziPrettyNames.Put("Char", PT_char);
	mmsziPrettyNames.Put("Float", PT_float);
	mmsziPrettyNames.Put("Double", PT_double);
	mmsziPrettyNames.Put("UInt", PT_uint);
	mmsziPrettyNames.Put("UShort", PT_ushort);
	mmsziPrettyNames.Put("UChar", PT_uchar);
	mmsziPrettyNames.Put("Long", PT_long);
	mmsziPrettyNames.Put("ULong", PT_ulong);
	mmsziPrettyNames.Put("Float2", PT_float2);
	mmsziPrettyNames.Put("Float3", PT_float3);
	mmsziPrettyNames.Put("Float4", PT_float4);
	mmsziPrettyNames.Put("Int2", PT_int2);
	mmsziPrettyNames.Put("Int3", PT_int3);
	mmsziPrettyNames.Put("Int4", PT_int4);
	mmsziPrettyNames.Put("Bit", PT_bit);
	mmsziPrettyNames.Put("Crumb", PT_crumb);
	mmsziPrettyNames.Put("Nybble", PT_nybble);
	mmsziPrettyNames.Put("Nickle", PT_nickle);
	mmsziPrettyNames.Put("Sixbits", PT_sixbits);

	mmsziCppNames.Init(30, TRUE);
	mmsziCppNames.Put("int", PT_int);
	mmsziCppNames.Put("BOOL", PT_bool);
	mmsziCppNames.Put("short", PT_short);
	mmsziCppNames.Put("void", PT_void);
	mmsziCppNames.Put("void*", PT_VoidPointer); //?
	mmsziCppNames.Put("char", PT_char);
	mmsziCppNames.Put("float", PT_float);
	mmsziCppNames.Put("double", PT_double);
	mmsziCppNames.Put("unsigned int", PT_uint);
	mmsziCppNames.Put("unsigned short", PT_ushort);
	mmsziCppNames.Put("unsigned char", PT_uchar);
	mmsziCppNames.Put("long long int", PT_long);
	mmsziCppNames.Put("unsigned long long int", PT_ulong);
	mmsziCppNames.Put("SFloat2", PT_float2);
	mmsziCppNames.Put("SFloat3", PT_float3);
	mmsziCppNames.Put("SFloat4", PT_float4);
	mmsziCppNames.Put("SInt2", PT_int2);
	mmsziCppNames.Put("SInt3", PT_int3);
	mmsziCppNames.Put("SInt4", PT_int4);
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
const char* CTypeNames::GetPrettyName(EPrimitiveTypes eType)
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
const char* CTypeNames::GetCPPName(EPrimitiveTypes eType)
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
const char* CTypeNames::GetPrimitiveName(EPrimitiveTypes eType)
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
EPrimitiveTypes CTypeNames::GetTypeFromPrettyName(const char* szPrettyName)
{
	int*	piType;

	piType = mmsziPrettyNames.Get(szPrettyName);
	if (piType)
	{
		return (EPrimitiveTypes)(*piType);
	}
	return PT_Undefined;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveTypes CTypeNames::GetTypeFromCPPName(const char* szCppName)
{
	int*	piType;

	piType = mmsziCppNames.Get(szCppName);
	if (piType)
	{
		return (EPrimitiveTypes)(*piType);
	}
	return PT_Undefined;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTypeNames::GetByteSize(EPrimitiveTypes eType)
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
int CTypeNames::GetBitSize(EPrimitiveTypes eType)
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
	gcTypeNames.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TypesKill(void)
{
	gcTypeNames.Kill();
}

