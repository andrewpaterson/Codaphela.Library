/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CPPVtable.h"
#include "CPP65DebugValues.h"


//////////////////////////////////////////////////////////////////////////
//
// A value of uiType at uiAddress in eSpace.
//
//////////////////////////////////////////////////////////////////////////
void SCPP65DebugValue::Init(CAsm65DebugRecords* pcRecords, uint32 uiType, EAsm65StackSpace eSpace, uint32 uiAddress)
{
	this->pcRecords = pcRecords;
	this->uiType = uiType;
	this->uiAddress = uiAddress;
	uiArrayCount = 0;
	uiSpace = (uint8)eSpace;
	bInMemory = true;
	uiBitOffset = 0;
	uiBits = 0;
	uiFormat = CPP65DF_Default;
	uiRegister = CPP65_DEBUG_NO_REGISTER;
	memset(auiBytes, 0, sizeof(auiBytes));
}


//////////////////////////////////////////////////////////////////////////
//
// A value of uiType worked out, not in memory: uiValue's low bytes.
//
//////////////////////////////////////////////////////////////////////////
void SCPP65DebugValue::Init(CAsm65DebugRecords* pcRecords, uint32 uiType, uint64 uiValue)
{
	size	ui;

	Init(pcRecords, uiType, ASS_Kernel, 0);
	bInMemory = false;
	for (ui = 0; ui < sizeof(auiBytes); ui++)
	{
		auiBytes[ui] = (uint8)(uiValue >> (ui * 8));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a type is a char's (shown as a character, and a pointer to it or
// an array of it as a string).
//
//////////////////////////////////////////////////////////////////////////
static bool IsCharacter(SAsm65DebugType* psType)
{
	return psType && (psType->uiKind == ADT_Integer) && (psType->uiFlags & ADTF_Character) && (psType->uiBytes == 1);
}


//////////////////////////////////////////////////////////////////////////
//
// A character as C++ writes it between cQuote's.
//
//////////////////////////////////////////////////////////////////////////
static void AppendCharacter(CChars* pszDest, uint8 uiCharacter, char cQuote)
{
	char	sz[8];

	switch (uiCharacter)
	{
	case '\n':
		pszDest->Append("\\n");
		return;
	case '\t':
		pszDest->Append("\\t");
		return;
	case '\r':
		pszDest->Append("\\r");
		return;
	case '\0':
		pszDest->Append("\\0");
		return;
	case '\\':
		pszDest->Append("\\\\");
		return;
	}
	if (uiCharacter == (uint8)cQuote)
	{
		pszDest->Append('\\')->Append(cQuote);
	}
	else if ((uiCharacter >= 0x20) && (uiCharacter < 0x7F))
	{
		pszDest->Append((char)uiCharacter);
	}
	else
	{
		snprintf(sz, sizeof(sz), "\\x%02x", uiCharacter);
		pszDest->Append(sz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The bits of an address on pcRecords' processor: 24 on the W65C816 (a
// pointer's top byte ignored), 32 on the 65C64.
//
//////////////////////////////////////////////////////////////////////////
uint32 CPP65DebugAddressMask(CAsm65DebugRecords* pcRecords)
{
	return (pcRecords->GetProcessor() == ADP_65C64) ? 0xFFFFFFFF : 0xFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
// The hexadecimal digits an address is shown in on pcRecords' processor.
//
//////////////////////////////////////////////////////////////////////////
int CPP65DebugAddressDigits(CAsm65DebugRecords* pcRecords)
{
	return (pcRecords->GetProcessor() == ADP_65C64) ? 8 : 6;
}


//////////////////////////////////////////////////////////////////////////
//
// An int's bytes on pcRecords' processor (CCPPTarget::GetIntBytes): 2 on
// the W65C816, 4 on the 65C64.
//
//////////////////////////////////////////////////////////////////////////
uint32 CPP65DebugIntBytes(CAsm65DebugRecords* pcRecords)
{
	return (pcRecords->GetProcessor() == ADP_65C64) ? 4 : 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugValues::Init(CAsm65StackMemory* pcMemory)
{
	mpcMemory = pcMemory;
	mbHexadecimal = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugValues::Kill(void)
{
	mpcMemory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Integers in hexadecimal (Visual Studio's Hexadecimal Display).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugValues::SetHexadecimal(bool bHexadecimal)
{
	mbHexadecimal = bHexadecimal;
}


//////////////////////////////////////////////////////////////////////////
//
// psVariable, one of psFrame's function's, where it is while psFrame
// runs (CAsm65StackWalker::Locate): in memory, or in a register, whose
// value it is (uiRegister says which).  False if it isn't kept anywhere
// then.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::Variable(CAsm65StackWalker* pcWalker, SAsm65StackFrame* psFrame, SAsm65DebugVariable* psVariable, SCPP65DebugValue* psValue)
{
	SAsm65StackPlace	sPlace;

	if (!pcWalker->Locate(psFrame, psVariable, &sPlace))
	{
		return false;
	}
	if (sPlace.bAddress)
	{
		psValue->Init(psFrame->pcRecords, psVariable->uiType, (EAsm65StackSpace)psFrame->uiSpace, sPlace.uiAddress);
	}
	else
	{
		psValue->Init(psFrame->pcRecords, psVariable->uiType, sPlace.uiValue);
		psValue->uiRegister = sPlace.uiRegister;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A global or static of pcRecords' program, which runs in eSpace.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugValues::Global(CAsm65DebugRecords* pcRecords, SAsm65DebugGlobal* psGlobal, EAsm65StackSpace eSpace, SCPP65DebugValue* psValue)
{
	psValue->Init(pcRecords, psGlobal->uiType, eSpace, psGlobal->sPlace.uiOffset);
}


//////////////////////////////////////////////////////////////////////////
//
// What psFunction (of pcRecords' program) returned, just after it
// returned, from its processor's registers then (psRegisters), as its back
// end returns a result: the W65C816's of up to 32 bits in A (C, the low
// half) and X (the high), the 65C64's of up to 64 bits in A.  False for
// void, an object or an array, a result too big for those (returned
// through a hidden pointer), or registers not known.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::Result(CAsm65DebugRecords* pcRecords, SAsm65DebugFunction* psFunction, SAsm65StackRegisters* psRegisters, SCPP65DebugValue* psValue)
{
	SAsm65DebugType*	psType;
	uint64				uiValue;

	psType = pcRecords->GetType(psFunction->uiResult);
	if (!psType || (psType->uiKind == ADT_Class) || (psType->uiKind == ADT_Array) || (psType->uiBytes == 0))
	{
		return false;
	}
	switch (pcRecords->GetProcessor())
	{
	case ADP_W65C816:
		if ((psType->uiBytes > 4) || !psRegisters->IsKnown(AW65DR_C) || !psRegisters->IsKnown(AW65DR_X))
		{
			return false;
		}
		uiValue = (psRegisters->Get(AW65DR_C) & 0xFFFF) | ((psRegisters->Get(AW65DR_X) & 0xFFFF) << 16);
		break;
	case ADP_65C64:
		if ((psType->uiBytes > 8) || !psRegisters->IsKnown(A65DR_A))
		{
			return false;
		}
		uiValue = psRegisters->Get(A65DR_A);
		break;
	default:
		return false;
	}
	psValue->Init(pcRecords, psFunction->uiResult, uiValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The Call Stack's text for the frame, as Visual Studio's: its function's
// name (an inlined call's row its callee's) and its parameters, each's
// type, name and value, "Factorial(int i = 3, char* sz = 0x012345
// "Hello")".  A parameter that isn't kept anywhere there is "(optimised
// away)", one that can't be read "???".  False if some of it couldn't be
// read.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::AppendCall(CAsm65StackWalker* pcWalker, SAsm65StackFrame* psFrame, CChars* pszDest)
{
	SAsm65DebugVariable*	psVariable;
	SCPP65DebugValue		sValue;
	size					ui;
	bool					bFirst;
	bool					bResult;

	pszDest->Append(psFrame->GetName());
	pszDest->Append('(');
	bFirst = true;
	bResult = true;
	for (ui = 0; ui < psFrame->psFunction->uiNumVariables; ui++)
	{
		psVariable = psFrame->pcRecords->GetVariable(psFrame->psFunction, ui);
		if (!psVariable->bParameter || (psVariable->iInline != psFrame->iInline) || psVariable->szName.Equals("this"))
		{
			continue;
		}
		if (!bFirst)
		{
			pszDest->Append(", ");
		}
		bFirst = false;
		sValue.Init(psFrame->pcRecords, psVariable->uiType, (EAsm65StackSpace)psFrame->uiSpace, 0);
		AppendType(&sValue, pszDest);
		pszDest->Append(' ')->Append(psVariable->szName)->Append(" = ");
		if (Variable(pcWalker, psFrame, psVariable, &sValue))
		{
			bResult &= Format(&sValue, pszDest);
		}
		else
		{
			pszDest->Append(((psVariable->uiWhere == ADW_None) || (psVariable->uiWhere == ADW_Ranges)) ? CPP65_DEBUG_OPTIMISED_AWAY : "???");
		}
	}
	pszDest->Append(')');
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The Value column's text.  False if some of it couldn't be read (shown
// as ???).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::Format(SCPP65DebugValue* psValue, CChars* pszDest)
{
	SAsm65DebugType*	psType;
	SCPP65DebugValue	sTarget;
	uint64				uiValue;
	float				f;
	double				d;
	uint32				uiAddress;
	char				sz[40];

	psType = GetType(psValue);
	if (!psType)
	{
		pszDest->Append("?");
		return false;
	}
	if (psValue->uiArrayCount != 0)
	{
		if (IsCharacter(psType))
		{
			return AppendString(psValue->pcRecords, (EAsm65StackSpace)psValue->uiSpace, psValue->uiAddress, psValue->uiArrayCount, pszDest);
		}
		pszDest->Append("{...}");
		return true;
	}
	switch (psType->uiKind)
	{
	case ADT_Reference:
		if (Dereference(psValue, &sTarget))
		{
			return Format(&sTarget, pszDest);
		}
		break;
	case ADT_Array:
		if (IsCharacter(psValue->pcRecords->GetType(psType->uiTarget)) && psValue->bInMemory)
		{
			return AppendString(psValue->pcRecords, (EAsm65StackSpace)psValue->uiSpace, psValue->uiAddress, psType->uiCount, pszDest);
		}
		pszDest->Append("{...}");
		return true;
	case ADT_Class:
		pszDest->Append("{...}");
		return true;
	case ADT_Void:
	case ADT_Function:
		return true;
	}

	if (!Read(psValue, &uiValue))
	{
		pszDest->Append("???");
		return false;
	}
	switch (psType->uiKind)
	{
	case ADT_Bool:
		pszDest->Append((uiValue == 0) ? "false" : "true");
		if (uiValue > 1)
		{
			pszDest->Append(" (")->Append((uint32)uiValue)->Append(")");
		}
		return true;
	case ADT_Integer:
		AppendInteger(psValue, psType, uiValue, pszDest);
		return true;
	case ADT_Enum:
		AppendEnum(psValue, psType, uiValue, pszDest);
		return true;
	case ADT_Float:
		if (psType->uiBytes == sizeof(float))
		{
			memcpy(&f, &uiValue, sizeof(f));
			snprintf(sz, sizeof(sz), "%.9g", f);
		}
		else
		{
			memcpy(&d, &uiValue, sizeof(d));
			snprintf(sz, sizeof(sz), "%.17g", d);
		}
		pszDest->Append(sz);
		return true;
	default:
		//A pointer (or a reference to nothing): its address, and a char*'s string (,s: its string alone).
		uiAddress = (uint32)uiValue & CPP65DebugAddressMask(psValue->pcRecords);
		if ((uiAddress != 0) && IsCharacter(psValue->pcRecords->GetType(psType->uiTarget)))
		{
			if (psValue->uiFormat != CPP65DF_String)
			{
				snprintf(sz, sizeof(sz), "0x%0*x ", CPP65DebugAddressDigits(psValue->pcRecords), uiAddress);
				pszDest->Append(sz);
			}
			return AppendString(psValue->pcRecords, (EAsm65StackSpace)psValue->uiSpace, uiAddress, CPP65_DEBUG_STRING_MOST, pszDest);
		}
		snprintf(sz, sizeof(sz), "0x%0*x", CPP65DebugAddressDigits(psValue->pcRecords), uiAddress);
		pszDest->Append(sz);
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The Type column's text: the type's name, and for an object with a
// vtable pointer (or a pointer or reference to one) the class of the
// object it is in when that is another, "CShape* {CSquare}".
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::AppendType(SCPP65DebugValue* psValue, CChars* pszDest)
{
	SAsm65DebugType*	psType;
	SCPP65DebugValue	sClass;
	CChars				szDynamic;

	psType = GetType(psValue);
	if (!psType)
	{
		pszDest->Append("?");
		return false;
	}
	pszDest->Append(&psType->szName);
	if (psValue->uiArrayCount != 0)
	{
		pszDest->Append('[')->Append(psValue->uiArrayCount)->Append(']');
		return true;
	}
	if (ClassOf(psValue, &sClass))
	{
		szDynamic.Init();
		if (DynamicClass(&sClass, &szDynamic) && !szDynamic.Equals(&GetType(&sClass)->szName))
		{
			pszDest->Append(" {")->Append(&szDynamic)->Append("}");
		}
		szDynamic.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A class's bases and fields, an array's elements, what a pointer points
// at (a class's members, or one child), a reference's value's.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65DebugValues::NumChildren(SCPP65DebugValue* psValue)
{
	SAsm65DebugType*	psType;
	SAsm65DebugMember*	psMember;
	SCPP65DebugValue	sTarget;
	size				ui;
	size				uiCount;

	psType = GetType(psValue);
	if (!psType)
	{
		return 0;
	}
	if (psValue->uiArrayCount != 0)
	{
		return psValue->uiArrayCount;
	}
	switch (psType->uiKind)
	{
	case ADT_Class:
		uiCount = 0;
		for (ui = 0; ui < psType->uiNumMembers; ui++)
		{
			psMember = psValue->pcRecords->GetMember(psType, ui);
			uiCount += ((psMember->uiKind == ADM_Base) || (psMember->uiKind == ADM_Field)) ? 1 : 0;
		}
		return uiCount;
	case ADT_Array:
		return psType->uiCount;
	case ADT_Pointer:
		if (!Dereference(psValue, &sTarget))
		{
			return 0;
		}
		return (GetType(&sTarget)->uiKind == ADT_Class) ? NumChildren(&sTarget) : 1;
	case ADT_Reference:
		return Dereference(psValue, &sTarget) ? NumChildren(&sTarget) : 0;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The uiIndex'th child (NumChildren) and its name: a base's class, a
// field's, [n], or * for what a pointer points at.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::GetChild(SCPP65DebugValue* psValue, size uiIndex, CChars* pszName, SCPP65DebugValue* psChild)
{
	SAsm65DebugType*	psType;
	SAsm65DebugMember*	psMember;
	SCPP65DebugValue	sTarget;
	size				ui;
	bool				bChild;

	psType = GetType(psValue);
	if (!psType)
	{
		return false;
	}
	if ((psValue->uiArrayCount != 0) || (psType->uiKind == ADT_Array))
	{
		bChild = Element(psValue, uiIndex, psChild);
		if (bChild)
		{
			pszName->Append('[')->Append((uint32)uiIndex)->Append(']');
		}
		return bChild;
	}
	bChild = false;
	switch (psType->uiKind)
	{
	case ADT_Class:
		for (ui = 0; psValue->bInMemory && (ui < psType->uiNumMembers); ui++)
		{
			psMember = psValue->pcRecords->GetMember(psType, ui);
			if (((psMember->uiKind != ADM_Base) && (psMember->uiKind != ADM_Field)) || (uiIndex-- > 0))
			{
				continue;
			}
			MemberValue(psValue, psMember, psChild);
			pszName->Append((psMember->uiKind == ADM_Base) ? &psValue->pcRecords->GetType(psMember->uiType)->szName : &psMember->szName);
			bChild = true;
			break;
		}
		break;
	case ADT_Pointer:
	case ADT_Reference:
		if (!Dereference(psValue, &sTarget))
		{
			return false;
		}
		sTarget.uiFormat = psValue->uiFormat;
		if ((psType->uiKind == ADT_Reference) || (GetType(&sTarget)->uiKind == ADT_Class))
		{
			return GetChild(&sTarget, uiIndex, pszName, psChild);
		}
		*psChild = sTarget;
		pszName->Append('*');
		bChild = uiIndex == 0;
		break;
	}
	psChild->uiFormat = psValue->uiFormat;
	return bChild;
}


//////////////////////////////////////////////////////////////////////////
//
// The field or base named szName of the class psValue is: its own fields
// first, then its bases and what is in them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::Member(SCPP65DebugValue* psValue, const char* szName, SCPP65DebugValue* psMember)
{
	SAsm65DebugType*	psType;
	SAsm65DebugMember*	psRecord;
	SCPP65DebugValue	sFound;
	size				ui;
	int					iPass;

	psType = GetType(psValue);
	if (!psType || (psType->uiKind != ADT_Class) || !psValue->bInMemory)
	{
		return false;
	}
	for (iPass = 0; iPass < 2; iPass++)
	{
		for (ui = 0; ui < psType->uiNumMembers; ui++)
		{
			psRecord = psValue->pcRecords->GetMember(psType, ui);
			if (psRecord->uiKind != ((iPass == 0) ? ADM_Field : ADM_Base))
			{
				continue;
			}
			MemberValue(psValue, psRecord, &sFound);
			if ((iPass == 0) ? psRecord->szName.Equals(szName) : psValue->pcRecords->GetType(psRecord->uiType)->szName.Equals(szName))
			{
				*psMember = sFound;
				return true;
			}
			if ((iPass == 1) && Member(&sFound, szName, psMember))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A base or field (psRecord) of the class psValue is.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugValues::MemberValue(SCPP65DebugValue* psValue, SAsm65DebugMember* psRecord, SCPP65DebugValue* psMember)
{
	uint8	uiFormat;

	uiFormat = psValue->uiFormat;
	psMember->Init(psValue->pcRecords, psRecord->uiType, (EAsm65StackSpace)psValue->uiSpace, psValue->uiAddress + (uint32)psRecord->iValue);
	psMember->uiBitOffset = psRecord->uiBitOffset;
	psMember->uiBits = psRecord->uiBits;
	psMember->uiFormat = uiFormat;
}


//////////////////////////////////////////////////////////////////////////
//
// What typing szText into psValue's Value writes (C2): a number, a
// character ('c'), true or false, an enumerator, a float, or an address
// (nullptr).  A reference's value is what is written.  False if psValue
// can't be written (an array, a class, worked out) or szText isn't one of
// its type's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::Edit(SCPP65DebugValue* psValue, const char* szText, SCPP65DebugWrite* psWrite)
{
	SAsm65DebugType*	psType;
	SCPP65DebugValue	sTarget;
	uint64				uiValue;
	uint64				uiOld;
	uint64				uiMask;
	float				f;
	double				d;
	char*				szEnd;
	size				ui;

	psType = GetType(psValue);
	if (!psType || (!psValue->bInMemory && (psValue->uiRegister == CPP65_DEBUG_NO_REGISTER)))
	{
		return false;
	}
	if (psType->uiKind == ADT_Reference)
	{
		return Dereference(psValue, &sTarget) && Edit(&sTarget, szText, psWrite);
	}

	while (*szText == ' ')
	{
		szText++;
	}
	switch (psType->uiKind)
	{
	case ADT_Bool:
		if ((strcmp(szText, "true") == 0) || (strcmp(szText, "false") == 0))
		{
			uiValue = (szText[0] == 't') ? 1 : 0;
		}
		else if (!ParseInteger(psValue, psType, szText, &uiValue))
		{
			return false;
		}
		break;
	case ADT_Integer:
	case ADT_Enum:
		if (!ParseInteger(psValue, psType, szText, &uiValue))
		{
			return false;
		}
		break;
	case ADT_Float:
		d = strtod(szText, &szEnd);
		if ((szEnd == szText) || (*szEnd != '\0'))
		{
			return false;
		}
		uiValue = 0;
		if (psType->uiBytes == sizeof(float))
		{
			f = (float)d;
			memcpy(&uiValue, &f, sizeof(f));
		}
		else
		{
			memcpy(&uiValue, &d, sizeof(d));
		}
		break;
	case ADT_Pointer:
		if ((strcmp(szText, "nullptr") == 0) || (strcmp(szText, "NULL") == 0))
		{
			uiValue = 0;
			break;
		}
		uiValue = strtoull(szText, &szEnd, 0);
		if ((szEnd == szText) || (*szEnd != '\0') || (szText[0] == '-') || (uiValue > CPP65DebugAddressMask(psValue->pcRecords)))
		{
			return false;
		}
		break;
	default:
		return false;
	}

	//A value in a register: the register's other bytes as they were.
	psWrite->uiRegister = psValue->uiRegister;
	if (!psValue->bInMemory)
	{
		psWrite->uiAddress = 0;
		psWrite->uiSpace = psValue->uiSpace;
		psWrite->uiSize = sizeof(psWrite->auiBytes);
		for (ui = 0; ui < sizeof(psWrite->auiBytes); ui++)
		{
			psWrite->auiBytes[ui] = (ui < psType->uiBytes) ? (uint8)(uiValue >> (ui * 8)) : psValue->auiBytes[ui];
		}
		return true;
	}

	//A bit field's bits go into the bytes that hold them.
	psWrite->uiAddress = psValue->uiAddress;
	psWrite->uiSpace = psValue->uiSpace;
	psWrite->uiSize = (uint8)((psValue->uiBits != 0) ? (psValue->uiBitOffset + psValue->uiBits + 7) / 8 : psType->uiBytes);
	if (psWrite->uiSize > sizeof(psWrite->auiBytes))
	{
		return false;
	}
	if (psValue->uiBits != 0)
	{
		uiOld = 0;
		if (!ReadBytes(psValue, 0, &uiOld, psWrite->uiSize))
		{
			return false;
		}
		uiMask = (((uint64)1 << psValue->uiBits) - 1) << psValue->uiBitOffset;
		uiValue = (uiOld & ~uiMask) | ((uiValue << psValue->uiBitOffset) & uiMask);
	}
	for (ui = 0; ui < sizeof(psWrite->auiBytes); ui++)
	{
		psWrite->auiBytes[ui] = (uint8)(uiValue >> (ui * 8));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A value of a type that fits in 64 bits (an integer, bool, enum, float or
// pointer; a bit field's bits), a signed one sign extended.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::Read(SCPP65DebugValue* psValue, uint64* puiValue)
{
	SAsm65DebugType*	psType;
	SAsm65DebugType*	psInteger;
	size				uiBytes;
	size				uiBits;
	uint64				uiValue;

	psType = GetType(psValue);
	if (!psType)
	{
		return false;
	}
	uiBits = (psValue->uiBits != 0) ? psValue->uiBits : psType->uiBytes * 8;
	uiBytes = (psValue->uiBitOffset + uiBits + 7) / 8;
	uiValue = 0;
	if ((uiBits == 0) || (uiBytes > sizeof(uint64)) || !ReadBytes(psValue, 0, &uiValue, uiBytes))
	{
		return false;
	}
	uiValue >>= psValue->uiBitOffset;
	if (uiBits < 64)
	{
		uiValue &= ((uint64)1 << uiBits) - 1;
		psInteger = (psType->uiKind == ADT_Enum) ? psValue->pcRecords->GetType(psType->uiTarget) : psType;
		if (psInteger && (psInteger->uiKind == ADT_Integer) && (psInteger->uiFlags & ADTF_Signed) && (uiValue & ((uint64)1 << (uiBits - 1))))
		{
			uiValue |= ~(((uint64)1 << uiBits) - 1);
		}
	}
	*puiValue = uiValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What a pointer or reference points at.  False for a null pointer, or a
// pointer to void or a function.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::Dereference(SCPP65DebugValue* psValue, SCPP65DebugValue* psTarget)
{
	SAsm65DebugType*	psType;
	SAsm65DebugType*	psPointee;
	uint64				uiValue;

	psType = GetType(psValue);
	if (!psType || ((psType->uiKind != ADT_Pointer) && (psType->uiKind != ADT_Reference)) || (psType->uiTarget == ASM65_DEBUG_NONE))
	{
		return false;
	}
	psPointee = psValue->pcRecords->GetType(psType->uiTarget);
	if (!psPointee || (psPointee->uiKind == ADT_Void) || (psPointee->uiKind == ADT_Function) || !Read(psValue, &uiValue) || ((uiValue & CPP65DebugAddressMask(psValue->pcRecords)) == 0))
	{
		return false;
	}
	psTarget->Init(psValue->pcRecords, psType->uiTarget, (EAsm65StackSpace)psValue->uiSpace, (uint32)uiValue & CPP65DebugAddressMask(psValue->pcRecords));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The class of the object psValue (a class's) is in, from its vtable
// pointer (its own, or its first base's with one): the name in the type
// info its vtable's header points at.  False for a class with no vtable
// pointer, or one not yet set.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::DynamicClass(SCPP65DebugValue* psValue, CChars* pszDest)
{
	SAsm65DebugType*	psType;
	SAsm65DebugMember*	psMember;
	CAsm65DebugRecords*	pcRecords;
	EAsm65StackSpace	eSpace;
	uint32				uiOffset;
	uint32				uiVtable;
	uint32				uiTypeInfo;
	uint32				uiName;
	size				ui;

	psType = GetType(psValue);
	if (!psType || (psType->uiKind != ADT_Class) || !psValue->bInMemory)
	{
		return false;
	}
	uiOffset = 0;
	psMember = NULL;
	for (ui = 0; ui < psType->uiNumMembers; ui++)
	{
		psMember = psValue->pcRecords->GetMember(psType, ui);
		if (psMember->uiKind == ADM_VTable)
		{
			break;
		}
		if (psMember->uiKind == ADM_Base)
		{
			//Into the base: its members from the first.
			uiOffset += (uint32)psMember->iValue;
			psType = psValue->pcRecords->GetType(psMember->uiType);
			ui = (size)-1;
		}
	}
	if (ui >= psType->uiNumMembers)
	{
		return false;
	}

	eSpace = (EAsm65StackSpace)psValue->uiSpace;
	pcRecords = psValue->pcRecords;
	if (!ReadLong(pcRecords, eSpace, psValue->uiAddress + uiOffset + (uint32)psMember->iValue, &uiVtable) || (uiVtable == 0) ||
		!ReadLong(pcRecords, eSpace, uiVtable + CPP65_VTABLE_TYPE_INFO_OFFSET, &uiTypeInfo) || (uiTypeInfo == 0) ||
		!ReadLong(pcRecords, eSpace, uiTypeInfo + CPP65_TYPE_INFO_NAME_OFFSET, &uiName) || (uiName == 0))
	{
		return false;
	}
	return AppendString(pcRecords, eSpace, uiName, CPP65_DEBUG_STRING_MOST, pszDest, '\0');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65DebugType* CCPP65DebugValues::GetType(SCPP65DebugValue* psValue)
{
	return (psValue->uiType == ASM65_DEBUG_NONE) ? NULL : psValue->pcRecords->GetType(psValue->uiType);
}


//////////////////////////////////////////////////////////////////////////
//
// uiBytes of psValue's from uiOffset: memory's, or the bytes it holds.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::ReadBytes(SCPP65DebugValue* psValue, uint32 uiOffset, void* pvDest, size uiBytes)
{
	if (psValue->bInMemory)
	{
		return mpcMemory->Read((EAsm65StackSpace)psValue->uiSpace, psValue->uiAddress + uiOffset, pvDest, uiBytes);
	}
	if (uiOffset + uiBytes > sizeof(psValue->auiBytes))
	{
		return false;
	}
	memcpy(pvDest, &psValue->auiBytes[uiOffset], uiBytes);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer's address, the four bytes at uiAddress (the bits an address
// has on pcRecords' processor).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::ReadLong(CAsm65DebugRecords* pcRecords, EAsm65StackSpace eSpace, uint32 uiAddress, uint32* puiLong)
{
	uint8	auiLong[4];
	uint32	uiMask;

	uiMask = CPP65DebugAddressMask(pcRecords);
	if (!mpcMemory->Read(eSpace, uiAddress & uiMask, auiLong, 4))
	{
		return false;
	}
	*puiLong = (auiLong[0] | (auiLong[1] << 8) | (auiLong[2] << 16) | ((uint32)auiLong[3] << 24)) & uiMask;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An integer of psType: decimal, or hexadecimal as wide as it is; a char
// as its character too, 99 'c'.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugValues::AppendInteger(SCPP65DebugValue* psValue, SAsm65DebugType* psType, uint64 uiValue, CChars* pszDest)
{
	uint64	uiMask;
	char	sz[32];

	uiMask = (psType->uiBytes >= 8) ? ~(uint64)0 : ((uint64)1 << (psType->uiBytes * 8)) - 1;
	if ((psValue->uiFormat == CPP65DF_Hexadecimal) || (mbHexadecimal && (psValue->uiFormat != CPP65DF_Decimal)))
	{
		snprintf(sz, sizeof(sz), "0x%0*llx", (int)(psType->uiBytes * 2), (unsigned long long)(uiValue & uiMask));
	}
	else if (psType->uiFlags & ADTF_Signed)
	{
		snprintf(sz, sizeof(sz), "%lld", (long long)uiValue);
	}
	else
	{
		snprintf(sz, sizeof(sz), "%llu", (unsigned long long)(uiValue & uiMask));
	}
	pszDest->Append(sz);
	if (IsCharacter(psType))
	{
		pszDest->Append(" '");
		AppendCharacter(pszDest, (uint8)uiValue, '\'');
		pszDest->Append('\'');
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An enum by its enumerator, or as enumerators or'ed together (flags,
// each used once) when they make it, or its number.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugValues::AppendEnum(SCPP65DebugValue* psValue, SAsm65DebugType* psType, uint64 uiValue, CChars* pszDest)
{
	SAsm65DebugMember*	psMember;
	SAsm65DebugType*	psInteger;
	CChars				szFlags;
	uint64				uiLeft;
	size				ui;

	for (ui = 0; ui < psType->uiNumMembers; ui++)
	{
		psMember = psValue->pcRecords->GetMember(psType, ui);
		if ((uint64)psMember->iValue == uiValue)
		{
			pszDest->Append(&psMember->szName);
			return;
		}
	}

	szFlags.Init();
	uiLeft = ((int64)uiValue > 0) ? uiValue : 0;
	for (ui = 0; (ui < psType->uiNumMembers) && (uiLeft != 0); ui++)
	{
		psMember = psValue->pcRecords->GetMember(psType, ui);
		if ((psMember->iValue > 0) && (((uint64)psMember->iValue & uiLeft) == (uint64)psMember->iValue))
		{
			szFlags.Append(szFlags.Empty() ? "" : " | ")->Append(&psMember->szName);
			uiLeft &= ~(uint64)psMember->iValue;
		}
	}
	if ((uiLeft == 0) && ((int64)uiValue > 0))
	{
		pszDest->Append(&szFlags);
	}
	else
	{
		psInteger = psValue->pcRecords->GetType(psType->uiTarget);
		AppendInteger(psValue, psInteger ? psInteger : psType, uiValue, pszDest);
	}
	szFlags.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The characters at uiAddress to a '\0' (at most uiMost), between cQuote's
// as C++ writes them, or as they are when cQuote is '\0'.  False if memory
// ran out first.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::AppendString(CAsm65DebugRecords* pcRecords, EAsm65StackSpace eSpace, uint32 uiAddress, size uiMost, CChars* pszDest, char cQuote)
{
	uint8	uiCharacter;
	size	ui;
	bool	bRead;

	if (cQuote != '\0')
	{
		pszDest->Append(cQuote);
	}
	bRead = true;
	for (ui = 0; ui < uiMost; ui++)
	{
		bRead = mpcMemory->Read(eSpace, (uiAddress + (uint32)ui) & CPP65DebugAddressMask(pcRecords), &uiCharacter, 1);
		if (!bRead || (uiCharacter == '\0'))
		{
			break;
		}
		if (cQuote != '\0')
		{
			AppendCharacter(pszDest, uiCharacter, cQuote);
		}
		else
		{
			pszDest->Append((char)uiCharacter);
		}
	}
	if (cQuote != '\0')
	{
		pszDest->Append(cQuote);
	}
	return bRead;
}


//////////////////////////////////////////////////////////////////////////
//
// An array's element (or one of the uiArrayCount psValue shows).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::Element(SCPP65DebugValue* psValue, size uiIndex, SCPP65DebugValue* psElement)
{
	SAsm65DebugType*	psType;
	SAsm65DebugType*	psElementType;
	uint32				uiElement;
	size				uiCount;

	psType = GetType(psValue);
	uiElement = (psValue->uiArrayCount != 0) ? psValue->uiType : psType->uiTarget;
	uiCount = (psValue->uiArrayCount != 0) ? psValue->uiArrayCount : psType->uiCount;
	psElementType = psValue->pcRecords->GetType(uiElement);
	if (!psValue->bInMemory || !psElementType || (uiIndex >= uiCount))
	{
		return false;
	}
	psElement->Init(psValue->pcRecords, uiElement, (EAsm65StackSpace)psValue->uiSpace, psValue->uiAddress + (uint32)(uiIndex * psElementType->uiBytes));
	psElement->uiFormat = psValue->uiFormat;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The class psValue is, or points or refers to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::ClassOf(SCPP65DebugValue* psValue, SCPP65DebugValue* psClass)
{
	SAsm65DebugType*	psType;

	psType = GetType(psValue);
	if (psType && (psType->uiKind == ADT_Class))
	{
		*psClass = *psValue;
		return true;
	}
	return Dereference(psValue, psClass) && (GetType(psClass)->uiKind == ADT_Class);
}


//////////////////////////////////////////////////////////////////////////
//
// A number (decimal, 0x hexadecimal, 0 octal), a character ('c', '\n'), or
// for an enum one of its enumerators, that fits psValue (its bits, signed
// or not).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugValues::ParseInteger(SCPP65DebugValue* psValue, SAsm65DebugType* psType, const char* szText, uint64* puiValue)
{
	static const char	szEscapes[] = "n\nt\tr\r0\0\\\\''\"\"";
	SAsm65DebugType*	psInteger;
	SAsm65DebugMember*	psMember;
	const char*			szEnd;
	char*				szNumberEnd;
	size				uiBits;
	size				ui;
	bool				bSigned;
	int64				iValue;

	for (ui = 0; (psType->uiKind == ADT_Enum) && (ui < psType->uiNumMembers); ui++)
	{
		psMember = psValue->pcRecords->GetMember(psType, ui);
		if (psMember->szName.Equals(szText))
		{
			*puiValue = (uint64)psMember->iValue;
			return true;
		}
	}

	psInteger = (psType->uiKind == ADT_Enum) ? psValue->pcRecords->GetType(psType->uiTarget) : psType;
	bSigned = psInteger && (psInteger->uiKind == ADT_Integer) && (psInteger->uiFlags & ADTF_Signed);
	uiBits = (psValue->uiBits != 0) ? psValue->uiBits : psType->uiBytes * 8;
	if (szText[0] == '\'')
	{
		//A character, or one escaped.
		iValue = (uint8)szText[1];
		szEnd = &szText[2];
		if (szText[1] == '\\')
		{
			for (ui = 0; (ui < sizeof(szEscapes) - 1) && (szEscapes[ui] != szText[2]); ui += 2)
			{
			}
			if ((ui >= sizeof(szEscapes) - 1) || (szText[2] == '\0'))
			{
				return false;
			}
			iValue = (uint8)szEscapes[ui + 1];
			szEnd = &szText[3];
		}
		if ((szText[1] == '\0') || (szEnd[0] != '\'') || (szEnd[1] != '\0'))
		{
			return false;
		}
	}
	else
	{
		iValue = bSigned ? strtoll(szText, &szNumberEnd, 0) : (int64)strtoull(szText, &szNumberEnd, 0);
		if ((szNumberEnd == szText) || (*szNumberEnd != '\0') || (!bSigned && (szText[0] == '-')))
		{
			return false;
		}
	}

	//It must fit: -2^(bits-1) to 2^(bits-1)-1 signed, 0 to 2^bits-1 not.
	if ((uiBits < 64) && (bSigned ? ((iValue < -((int64)1 << (uiBits - 1))) || (iValue >= ((int64)1 << (uiBits - 1)))) : ((uint64)iValue >= ((uint64)1 << uiBits))))
	{
		return false;
	}
	*puiValue = (uint64)iValue;
	return true;
}

