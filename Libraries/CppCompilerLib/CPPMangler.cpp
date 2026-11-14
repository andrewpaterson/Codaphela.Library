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
#include "CTReservedWord.h"
#include "CPPMangler.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::Init(CCPPTokeniser* pcTokeniser, CArrayCPPClasses* papcClasses, CMapCPPFunctionsOfNames* pcFunctionsOfNames)
{
	mpcTokeniser = pcTokeniser;
	mpapcClasses = papcClasses;
	mpcFunctionsOfNames = pcFunctionsOfNames;
	mapcUnnamedNamespaces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::Kill(void)
{
	mapcUnnamedNamespaces.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The first function with the name, of those the unit has declared.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPMangler::FirstOfName(int64 lliName)
{
	SCPPFunctionsOfName*	psOfName;

	psOfName = mpcFunctionsOfNames->Get(lliName);
	return psOfName ? psOfName->pcFirst : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Functions are _name, operators _opName.  Members are _Class__name, constructors
// _Class__0 and destructors _Class__1; the vtable is _Class__2.  A template
// instance's class is Class__T and its arguments, mangled.  An overloaded member
// has its parameters on the end, as a function that is not a member always does,
// so adding an overload never moves another's label.  C++ names cannot contain __
// and cannot start with a digit, so none can clash.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendFunctionLabel(CChars* psz, CCPPFunction* pcFunction)
{
	CChars	szName;
	size	i;
	bool	bPlainName;

	szName.Init();
	if (pcFunction->meKind == CPPFK_Constructor)
	{
		szName.Append('0');
	}
	else if (pcFunction->meKind == CPPFK_Destructor)
	{
		szName.Append('1');
	}
	else
	{
		AppendFunctionName(&szName, pcFunction->mlliName);
	}

	//main, and the global operator new and delete the generator calls by name, are just their names.
	bPlainName = !pcFunction->mpcClass && !pcFunction->mpcNamespace && (szName.Equals("main") ||
			((pcFunction->mapsParameters.NumElements() == 1) && (szName.Equals("opNew") || szName.Equals("opNewArray") || szName.Equals("opDelete") || szName.Equals("opDeleteArray"))));
	if (pcFunction->mpcClass)
	{
		AppendClassLabel(psz, pcFunction->mpcClass, szName.Text());
	}
	else
	{
		psz->Append('_');
		if (!pcFunction->mbCLinkage)
		{
			AppendScopeNames(psz, pcFunction->mpcNamespace);
		}
		psz->Append(&szName);
	}
	szName.Kill();

	if (pcFunction->mszArguments.Length() > 0)
	{
		psz->Append("__T");
		psz->Append(&pcFunction->mszArguments);
	}
	if (((!pcFunction->mpcClass && !pcFunction->mpcTemplate) || (pcFunction->mszArguments.Length() > 0) || IsOverloaded(pcFunction)) && !pcFunction->mbCLinkage && !bPlainName)
	{
		//A function that is not a member has its parameters in its label, as units can declare different sets of
		//overloads.  Without parameters (only one such overload can exist) or with C linkage it is just its name.
		//So does an instance of a function template, as Add<M>(void) and Add<M>(size) are both Add__T_..., and an
		//overloaded member (hand-written assembly calls it by a label that doesn't depend on the order of its
		//overloads); a const one ends in K.
		if (pcFunction->mbConstThis)
		{
			psz->Append("__K");
		}
		if ((pcFunction->mapsParameters.NumElements() > 0) || pcFunction->mbVariadic)
		{
			psz->Append("__F");
			for (i = 0; i < pcFunction->mapsParameters.NumElements(); i++)
			{
				AppendType(psz, pcFunction->mapsParameters.GetPtr(i));
			}
			if (pcFunction->mbVariadic)
			{
				//f(int) and f(int, ...) are different overloads.
				psz->Append('z');
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// _name, or _Class__name for a static data member.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendGlobalLabel(CChars* psz, CCPPVariable* pcVariable)
{
	CChars	szName;
	char*	szIdentifier;
	int		iLength;
	int64	lliName;

	lliName = pcVariable->mpcSymbol->mlliName;
	szIdentifier = mpcTokeniser->GetIdentifier(lliName, &iLength);
	if (pcVariable->mpcClass)
	{
		szName.Init(szIdentifier, 0, iLength);
		AppendClassLabel(psz, pcVariable->mpcClass, szName.Text());
		szName.Kill();
		return;
	}
	psz->Append('_');
	if (!pcVariable->mbCLinkage)
	{
		AppendScopeNames(psz, pcVariable->mpcSymbol->mpcScope->GetNamespace());
	}
	psz->Append(szIdentifier, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Another function (not a template) of the same class (or none), kind and
// name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPMangler::IsOverloaded(CCPPFunction* pcFunction)
{
	CCPPFunction*	pcOther;

	for (pcOther = FirstOfName(pcFunction->mlliName); pcOther; pcOther = pcOther->mpcNextOfName)
	{
		if ((pcOther != pcFunction) && (pcOther->mpcClass == pcFunction->mpcClass) && (pcOther->meKind == pcFunction->meKind) && !pcOther->mpcTemplate)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__part.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendClassLabel(CChars* psz, CCPPClass* pcClass, const char* szPart)
{
	psz->Append('_');
	AppendClassName(psz, pcClass);
	psz->Append("__");
	psz->Append(szPart);
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__2, the class's vtable (CPP65_VTABLE_LABEL).
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendVtableLabel(CChars* psz, CCPPClass* pcClass)
{
	AppendClassLabel(psz, pcClass, CPP65_VTABLE_LABEL);
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__3, the class's type info (CPP65_TYPE_INFO_LABEL).
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendTypeInfoLabel(CChars* psz, CCPPClass* pcClass)
{
	AppendClassLabel(psz, pcClass, CPP65_TYPE_INFO_LABEL);
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable entry's label: R_Pure for a pure virtual function, else the
// function's, or its thunk's (__t and the adjustment's negation) when this
// is adjusted by iAdjust first.  AppendVtableEntry writes the thunk.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendVtableEntryLabel(CChars* psz, CCPPFunction* pcFunction, int iAdjust)
{
	if (pcFunction->mbPure)
	{
		psz->Append("R_Pure");
		return;
	}
	AppendFunctionLabel(psz, pcFunction);
	if (iAdjust != 0)
	{
		psz->Append("__t");
		psz->Append(-iAdjust);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Class, or Class__T and the template arguments, after the classes and
// namespaces it is in (Outer__Class).  A class without a name is the unit,
// Unnamed and its number.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendClassName(CChars* psz, CCPPClass* pcClass)
{
	char*	szName;
	int		iLength;

	if (pcClass->IsUnnamed())
	{
		//A class without a name is the unit's own (its labels are local to the object).
		psz->Append("Unnamed");
		psz->Append((int)mpapcClasses->Find(pcClass));
		return;
	}
	if (pcClass->mpcSymbol)
	{
		AppendScopeNames(psz, pcClass->mpcSymbol->mpcScope);
	}
	szName = mpcTokeniser->GetIdentifier(pcClass->mlliName, &iLength);
	psz->Append(szName, iLength);
	if (pcClass->mszArguments.Length() > 0)
	{
		psz->Append("__T");
		psz->Append(&pcClass->mszArguments);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The named classes and namespaces pcScope is in, outermost first, each
// followed by __, so N::Outer::Inner's class is N__Outer__Inner and two
// Inners (of different classes) have different labels.  A namespace without
// a name is Unnamed and its number, so what it declares doesn't clash with
// the same name outside it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendScopeNames(CChars* psz, CCPPScope* pcScope)
{
	char*		szName;
	int			iLength;
	CCPPSymbol*	pcOwner;
	size		uiUnnamed;

	if (!pcScope || ((pcScope->meKind != CPPSC_Class) && (pcScope->meKind != CPPSC_Namespace)))
	{
		return;
	}
	pcOwner = pcScope->mpcOwner;
	AppendScopeNames(psz, pcScope->mpcParent);
	if (pcOwner && (pcOwner->mlliName >= 0))
	{
		szName = mpcTokeniser->GetIdentifier(pcOwner->mlliName, &iLength);
		psz->Append(szName, iLength);
		psz->Append("__");
	}
	else if (pcScope->IsUnnamedNamespace())
	{
		uiUnnamed = mapcUnnamedNamespaces.Find(pcScope);
		if (uiUnnamed == ARRAY_ELEMENT_NOT_FOUND)
		{
			uiUnnamed = mapcUnnamedNamespaces.NumElements();
			mapcUnnamedNamespaces.Add(pcScope);
		}
		psz->Append("Unnamed");
		psz->Append((int)uiUnnamed);
		psz->Append("__");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A function's name: its identifier, or op and the operator's name.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendFunctionName(CChars* psz, int64 lliName)
{
	char*	szName;
	int		iLength;

	if (lliName == CPP_CONVERSION_NAME)
	{
		psz->Append("opConvert");
		return;
	}
	if (lliName < 0)
	{
		psz->Append("op");
		psz->Append(GetOperatorLabel(lliName));
		return;
	}
	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	psz->Append(szName, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Types in labels: v void, b bool, c char, h unsigned char, s short, t unsigned
// short, i int, j unsigned, x long long, y unsigned long long, f float, d double,
// e long double, P pointer, R reference, A count _ array, F return parameters E
// function, and C, the length and the label of a class.  A volatile type starts
// with V and a const one with K (VK both): const char* is PKc, const T& RK.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendType(CChars* psz, SCPP65Type* psType)
{
	CChars	szClass;
	size	i;
	char*	szIdentifier;
	int		iLength;

	if (psType->bVolatile)
	{
		psz->Append('V');
	}
	if (psType->bConst)
	{
		psz->Append('K');
	}
	switch (psType->eKind)
	{
	case CPP65TK_Void:
		psz->Append('v');
		break;
	case CPP65TK_Bool:
		psz->Append('b');
		break;
	case CPP65TK_Integer:
		if (psType->pcEnum)
		{
			szClass.Init();
			AppendScopeNames(&szClass, psType->pcEnum->mpcScope);
			szIdentifier = mpcTokeniser->GetIdentifier(psType->pcEnum->mlliName, &iLength);
			szClass.Append(szIdentifier, iLength);
			psz->Append('N');
			psz->Append((int)szClass.Length());
			psz->Append(&szClass);
			szClass.Kill();
			break;
		}
		switch (psType->iBytes)
		{
		case 1:		psz->Append(psType->bSigned ? 'c' : 'h');	break;
		case 2:		psz->Append(psType->bSigned ? 's' : 't');	break;
		case 8:		psz->Append(psType->bSigned ? 'x' : 'y');	break;
		default:	psz->Append(psType->bSigned ? 'i' : 'j');	break;
		}
		break;
	case CPP65TK_Float:
		psz->Append((psType->iBytes == 4) ? 'f' : (psType->bLongDouble ? 'e' : 'd'));
		break;
	case CPP65TK_Pointer:
		psz->Append('P');
		AppendType(psz, psType->psBase);
		break;
	case CPP65TK_Reference:
		psz->Append('R');
		AppendType(psz, psType->psBase);
		break;
	case CPP65TK_Array:
		psz->Append('A');
		psz->Append(psType->iCount);
		psz->Append('_');
		AppendType(psz, psType->psBase);
		break;
	case CPP65TK_Function:
		psz->Append('F');
		AppendType(psz, psType->psBase);
		for (i = 0; i < psType->papsParameters->NumElements(); i++)
		{
			AppendType(psz, psType->papsParameters->GetPtr(i));
		}
		if (psType->bVariadic)
		{
			psz->Append('z');
		}
		psz->Append('E');
		break;
	case CPP65TK_Class:
		szClass.Init();
		AppendClassName(&szClass, psType->pcClass);
		psz->Append('C');
		psz->Append((int)szClass.Length());
		psz->Append(&szClass);
		szClass.Kill();
		break;
	case CPP65TK_MemberPointer:
		psz->Append('M');
		AppendType(psz, psType->pcClass->mpsType);
		AppendType(psz, psType->psBase);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The operator's part of a label: _Class__opIndex for operator[].
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPMangler::GetOperatorLabel(int64 lliName)
{
	ECppTokens	eToken;
	bool		bArray;

	bArray = lliName < -1 - NUM_CPP_TOKENS;
	eToken = (ECppTokens)(bArray ? (-1 - NUM_CPP_TOKENS - lliName) : (-1 - lliName));
	switch (eToken)
	{
	case CT_OpenSquareBracket:			return "Index";
	case CT_OpenRoundBracket:			return "Call";
	case CT_Add:						return "Add";
	case CT_Subtract:					return "Subtract";
	case CT_Multiply:					return "Multiply";
	case CT_Divide:						return "Divide";
	case CT_Modulus:					return "Modulus";
	case CT_BitwiseAnd:					return "And";
	case CT_BitwiseOr:					return "Or";
	case CT_BitwiseXor:					return "Xor";
	case CT_BitwiseNot:					return "Not";
	case CT_LogicalNot:					return "LogicalNot";
	case CT_LogicalAnd:					return "LogicalAnd";
	case CT_LogicalOr:					return "LogicalOr";
	case CT_LeftShift:					return "ShiftLeft";
	case CT_RightShift:					return "ShiftRight";
	case CT_Assignment:					return "Assign";
	case CT_AssignmentAdd:				return "AddAssign";
	case CT_AssignmentSubtract:			return "SubtractAssign";
	case CT_AssignmentMultiply:			return "MultiplyAssign";
	case CT_AssignmentDivide:			return "DivideAssign";
	case CT_AssignmentModulus:			return "ModulusAssign";
	case CT_AssignmentBitwiseAnd:		return "AndAssign";
	case CT_AssignmentBitwiseOr:		return "OrAssign";
	case CT_AssignmentBitwiseXor:		return "XorAssign";
	case CT_AssignmentLeftShift:		return "ShiftLeftAssign";
	case CT_AssignmentRightShift:		return "ShiftRightAssign";
	case CT_EqualTo:					return "Equal";
	case CT_NotEqualTo:					return "NotEqual";
	case CT_LessThan:					return "Less";
	case CT_LessThanEqualTo:			return "LessEqual";
	case CT_GreaterThan:				return "Greater";
	case CT_GreaterThanEqualTo:			return "GreaterEqual";
	case CT_Increment:					return "Increment";
	case CT_Decrement:					return "Decrement";
	case CT_MemberByPointer:			return "Arrow";
	case CT_MemberByPointerFunctionPointerIndirection:	return "ArrowMember";
	case CT_Comma:						return "Comma";
	case CT_new:						return bArray ? "NewArray" : "New";
	case CT_delete:						return bArray ? "DeleteArray" : "Delete";
	default:							return "Operator";		//ParseOperatorName takes no other token.
	}
}

