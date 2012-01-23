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
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeNames::AddType(EPrimitiveTypes eType, char* szPrettyName, char* szCppName, char* szPrimitiveName)
{
	STypeName*	psTypeName;
	int			iPrettyNameLen;
	int			iCppNameLen;
	int			iPrimitiveNameLen;
	int			iLen;

	psTypeName = masTypeNames.GrowToAtLeastNumElements(((int)eType)+1, TRUE, 0);

	psTypeName->eType = eType;
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
	masTypeNames.Init(30);
	AddType(PT_Undefined, "Undefined", "", "PT_Undefined");
	AddType(PT_int,"Int", "int", "PT_int");
	AddType(PT_short, "Short", "short", "PT_short");
	AddType(PT_VoidPointer, "Pointer", "void*",	"PT_VoidPointer");
	AddType(PT_char, "Char",	"char",	"PT_char");
	AddType(PT_float, "Float", "float", "PT_float");
	AddType(PT_double, "Double", "double", "PT_double");
	AddType(PT_uint, "UInt", "unsigned int", "PT_uint");
	AddType(PT_ushort,"UShort", "unsigned short", "PT_ushort");
	AddType(PT_uchar, "UChar", "unsigned char", "PT_uchar");
	AddType(PT_long, "Long", "long long int", "PT_long");
	AddType(PT_ulong, "ULong", "unsigned long long int", "PT_ulong");
	AddType(PT_float2, "Float2", "SFloat2", "PT_float2");
	AddType(PT_float3, "Float3", "SFloat3", "PT_float3");
	AddType(PT_float4, "Float4", "SFloat4", "PT_float4");
	AddType(PT_int2, "Int2", "SInt2", "PT_int2");
	AddType(PT_int3, "Int3", "SInt3", "PT_int3");
	AddType(PT_int4, "Int4", "SInt4", "PT_int4");
	AddType(PT_bool, "Bool", "BOOL", "PT_bool");
	AddType(PT_void, "Void", "void", "PT_void");
	AddType(PT_String, "String", "CChars", "PT_String");
	AddType(PT_Number, "Number", "CNumber", "PT_Number");
	AddType(PT_Date, "Date", "CDate", "PT_Date");
	AddType(PT_bit, "Bit", "", "PT_bit");
	AddType(PT_crumb, "Crumb", "", "PT_crumb");
	AddType(PT_nybble, "Nybble", "", "PT_nybble");
	AddType(PT_nickle, "Nickle", "", "PT_nickle");
	AddType(PT_sixbits, "Sixbits", "", "PT_sixbits");

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
	mmsziPrettyNames.Put("String", PT_String);
	mmsziPrettyNames.Put("Number", PT_Number);
	mmsziPrettyNames.Put("Date", PT_Date);
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
	mmsziCppNames.Put("CChars", PT_String);
	mmsziCppNames.Put("CNumber", PT_Number);
	mmsziCppNames.Put("CDate", PT_Date);
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
char* CTypeNames::GetPrettyName(EPrimitiveTypes eType)
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
char* CTypeNames::GetCPPName(EPrimitiveTypes eType)
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
char* CTypeNames::GetPrimitiveName(EPrimitiveTypes eType)
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
EPrimitiveTypes CTypeNames::GetTypeFromPrettyName(char* szPrettyName)
{
	int*	piType;

	piType = mmsziPrettyNames.GetWithKey(szPrettyName);
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
EPrimitiveTypes CTypeNames::GetTypeFromCPPName(char* szCppName)
{
	int*	piType;

	piType = mmsziCppNames.GetWithKey(szCppName);
	if (piType)
	{
		return (EPrimitiveTypes)(*piType);
	}
	return PT_Undefined;
}

