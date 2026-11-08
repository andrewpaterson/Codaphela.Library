/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "CTReservedWord.h"
#include "CPP65Generator.h"


//The generator's labels: mangled names of functions, classes and types.


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
void CCPP65Generator::AppendFunctionLabel(CChars* psz, SCPP65Function* psFunction)
{
	CChars	szName;
	size	i;
	bool	bPlainName;

	szName.Init();
	if (psFunction->eKind == CPP65FK_Constructor)
	{
		szName.Append('0');
	}
	else if (psFunction->eKind == CPP65FK_Destructor)
	{
		szName.Append('1');
	}
	else
	{
		AppendFunctionName(&szName, psFunction->lliName);
	}

	//main, and the global operator new and delete the generator calls by name, are just their names.
	bPlainName = !psFunction->psClass && !psFunction->pcNamespace && (szName.Equals("main") ||
			((psFunction->mapsParameters.NumElements() == 1) && (szName.Equals("opNew") || szName.Equals("opNewArray") || szName.Equals("opDelete") || szName.Equals("opDeleteArray"))));
	if (psFunction->psClass)
	{
		AppendClassLabel(psz, psFunction->psClass, szName.Text());
	}
	else
	{
		psz->Append('_');
		if (!psFunction->bCLinkage)
		{
			AppendScopeNames(psz, psFunction->pcNamespace);
		}
		psz->Append(&szName);
	}
	szName.Kill();

	if (psFunction->mszArguments.Length() > 0)
	{
		psz->Append("__T");
		psz->Append(&psFunction->mszArguments);
	}
	if (((!psFunction->psClass && !psFunction->psTemplate) || (psFunction->mszArguments.Length() > 0) || IsOverloaded(psFunction)) && !psFunction->bCLinkage && !bPlainName)
	{
		//A function that is not a member has its parameters in its label, as units can declare different sets of
		//overloads.  Without parameters (only one such overload can exist) or with C linkage it is just its name.
		//So does an instance of a function template, as Add<M>(void) and Add<M>(size) are both Add__T_..., and an
		//overloaded member (hand-written assembly calls it by a label that doesn't depend on the order of its
		//overloads); a const one ends in K.
		if (psFunction->bConstThis)
		{
			psz->Append("__K");
		}
		if ((psFunction->mapsParameters.NumElements() > 0) || psFunction->bVariadic)
		{
			psz->Append("__F");
			for (i = 0; i < psFunction->mapsParameters.NumElements(); i++)
			{
				AppendMangledType(psz, psFunction->mapsParameters.GetPtr(i));
			}
			if (psFunction->bVariadic)
			{
				//f(int) and f(int, ...) are different overloads.
				psz->Append('z');
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Another function (not a template) of the same class (or none), kind and
// name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsOverloaded(SCPP65Function* psFunction)
{
	SCPP65Function*	psOther;

	for (psOther = FirstFunctionOfName(psFunction->lliName); psOther; psOther = psOther->psNextOfName)
	{
		if ((psOther != psFunction) && (psOther->psClass == psFunction->psClass) && (psOther->eKind == psFunction->eKind) && !psOther->psTemplate)
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
void CCPP65Generator::AppendClassLabel(CChars* psz, SCPP65Class* psClass, const char* szPart)
{
	psz->Append('_');
	AppendClassName(psz, psClass);
	psz->Append("__");
	psz->Append(szPart);
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__2, the class's vtable (CPP65_VTABLE_LABEL).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVtableLabel(CChars* psz, SCPP65Class* psClass)
{
	AppendClassLabel(psz, psClass, CPP65_VTABLE_LABEL);
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__3, the class's type info (CPP65_TYPE_INFO_LABEL).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendTypeInfoLabel(CChars* psz, SCPP65Class* psClass)
{
	AppendClassLabel(psz, psClass, CPP65_TYPE_INFO_LABEL);
}


//////////////////////////////////////////////////////////////////////////
//
// Class, or Class__T and the template arguments, after the classes and
// namespaces it is in (Outer__Class).  A class without a name is the unit,
// Unnamed and its number.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendClassName(CChars* psz, SCPP65Class* psClass)
{
	char*	szName;
	int		iLength;

	if (psClass->lliName < 0)
	{
		//A class without a name is the unit's own (its labels are local to the object).
		psz->Append("Unnamed");
		psz->Append((int)mapsClasses.Find(psClass));
		return;
	}
	if (psClass->pcSymbol)
	{
		AppendScopeNames(psz, psClass->pcSymbol->mpcScope);
	}
	szName = mpcTokeniser->GetIdentifier(psClass->lliName, &iLength);
	psz->Append(szName, iLength);
	if (psClass->mszArguments.Length() > 0)
	{
		psz->Append("__T");
		psz->Append(&psClass->mszArguments);
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
void CCPP65Generator::AppendScopeNames(CChars* psz, CCPPScope* pcScope)
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
// The namespace pcScope is in (or is), and NULL for the global one.
//
//////////////////////////////////////////////////////////////////////////
CCPPScope* CCPP65Generator::NamespaceOf(CCPPScope* pcScope)
{
	CCPPScope*	pcNamespace;

	pcNamespace = pcScope->GetEnclosingNamespace();
	return (pcNamespace && pcNamespace->mpcParent) ? pcNamespace : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A function's name: its identifier, or op and the operator's name.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendFunctionName(CChars* psz, int64 lliName)
{
	char*	szName;
	int		iLength;

	if (lliName == CPP65_CONVERSION_NAME)
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
void CCPP65Generator::AppendMangledType(CChars* psz, SCPP65Type* psType)
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
		AppendMangledType(psz, psType->psBase);
		break;
	case CPP65TK_Reference:
		psz->Append('R');
		AppendMangledType(psz, psType->psBase);
		break;
	case CPP65TK_Array:
		psz->Append('A');
		psz->Append(psType->iCount);
		psz->Append('_');
		AppendMangledType(psz, psType->psBase);
		break;
	case CPP65TK_Function:
		psz->Append('F');
		AppendMangledType(psz, psType->psBase);
		for (i = 0; i < psType->papsParameters->NumElements(); i++)
		{
			AppendMangledType(psz, psType->papsParameters->GetPtr(i));
		}
		if (psType->bVariadic)
		{
			psz->Append('z');
		}
		psz->Append('E');
		break;
	case CPP65TK_Class:
		szClass.Init();
		AppendClassName(&szClass, psType->psClass);
		psz->Append('C');
		psz->Append((int)szClass.Length());
		psz->Append(&szClass);
		szClass.Kill();
		break;
	case CPP65TK_MemberPointer:
		psz->Append('M');
		AppendMangledType(psz, psType->psClass->psType);
		AppendMangledType(psz, psType->psBase);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A function name component's name: its identifier, or a negative number for an
// operator (see GetOperatorLabel).  -1 if it is neither.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::GetFunctionName(CCPPSyntaxNameComponent* pcComponent)
{
	CCPPToken*	pcToken;
	ECppTokens	eToken;
	bool		bArray;

	if (pcComponent && (pcComponent->meKind == CPPNC_Conversion))
	{
		return CPP65_CONVERSION_NAME;
	}
	if (!pcComponent || (pcComponent->meKind != CPPNC_Operator))
	{
		return GetComponentName(pcComponent);
	}
	pcToken = mpcTokeniser->Get(pcComponent->miName);
	if (!pcToken || !pcToken->IsReservedWord())
	{
		return -1;
	}
	eToken = ((CCTReservedWord*)pcToken)->GetToken();
	bArray = (pcComponent->miLastToken > pcComponent->miName) && ((eToken == CT_new) || (eToken == CT_delete));
	return GetOperatorName(eToken, bArray);
}


//////////////////////////////////////////////////////////////////////////
//
// Operators are named by their token: -1 - token, and -1 - NUM_CPP_TOKENS - token
// for new[] and delete[].
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::GetOperatorName(ECppTokens eToken, bool bArray)
{
	return bArray ? (-1 - NUM_CPP_TOKENS - (int64)eToken) : (-1 - (int64)eToken);
}


//////////////////////////////////////////////////////////////////////////
//
// The operator's part of a label: _Class__opIndex for operator[].
//
//////////////////////////////////////////////////////////////////////////
const char* CCPP65Generator::GetOperatorLabel(int64 lliName)
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
