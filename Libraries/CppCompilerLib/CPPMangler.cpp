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
#include "CPPLambda.h"
#include "CPPTemplate.h"
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
// Functions are _name, operators _opName and conversion functions
// _opConvert_Type.  Members are _Class__name, constructors
// _Class__0 and destructors _Class__1; the vtable is _Class__2.  A template
// instance's class is Class__T and its arguments, mangled.  An overloaded member
// has its parameters on the end, as a function that is not a member always does,
// so adding an overload never moves another's label.  The base version of a
// constructor or destructor of a class with virtual bases (bBaseVersion) ends
// __B.  C++ names cannot contain __
// and cannot start with a digit, so none can clash.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendFunctionLabel(CChars* psz, CCPPFunction* pcFunction, bool bBaseVersion)
{
	CChars	szName;
	size	i;
	bool	bPlainName;

	if (pcFunction->mpcCovariantOf)
	{
		//A covariant thunk: its overrider's label and how far it moves the result.
		AppendFunctionLabel(psz, pcFunction->mpcCovariantOf, bBaseVersion);
		psz->Append("__CV");
		psz->Append(pcFunction->miCovariantAdjust);
		return;
	}
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
		if (pcFunction->mlliName == CPP_CONVERSION_NAME)
		{
			//Conversion functions have no parameters; they differ by what they convert to: opConvert_PKc is operator const char*.
			szName.Append('_');
			AppendType(&szName, pcFunction->mpsReturn);
		}
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
		//overloads); a const one ends in K, a volatile one in V, and one declared & or && in R or O.
		if (pcFunction->mbConstThis)
		{
			psz->Append("__K");
		}
		if (pcFunction->mbVolatileThis)
		{
			psz->Append("__V");
		}
		if (pcFunction->meRefQualifier != CPPRQ_None)
		{
			psz->Append((pcFunction->meRefQualifier == CPPRQ_Rvalue) ? "__O" : "__R");
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
	if (bBaseVersion)
	{
		psz->Append("__B");
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
	}
	else
	{
		psz->Append('_');
		if (!pcVariable->mbCLinkage)
		{
			AppendScopeNames(psz, pcVariable->mpcSymbol->mpcScope->GetNamespace());
		}
		psz->Append(szIdentifier, iLength);
	}
	if (pcVariable->mszArguments.Length() > 0)
	{
		//A variable template's instance, as a class template's is.
		psz->Append("__T");
		psz->Append(&pcVariable->mszArguments);
	}
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

	if (pcFunction->mpcClass && pcFunction->mpcClass->mpsLambda && pcFunction->mpcClass->mpsLambda->bGeneric)
	{
		//A generic lambda's operator() is made for each call's arguments, one after another: its label has its
		//parameters from the first, or a call written before the second was made would name the first by another label.
		return true;
	}
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
// _Class__4, the class's VTT (CPP65_VTT_LABEL): the vtables its constructors
// and destructor give their bases' base versions.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendVTTLabel(CChars* psz, CCPPClass* pcClass)
{
	AppendClassLabel(psz, pcClass, CPP65_VTT_LABEL);
}


//////////////////////////////////////////////////////////////////////////
//
// _Complete__5_offset_Class, the construction vtable of the pcClass subobject
// at iOffset in a pcComplete (CPP65_CONSTRUCTION_LABEL).
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendConstructionVtableLabel(CChars* psz, CCPPClass* pcComplete, CCPPClass* pcClass, int iOffset)
{
	AppendClassLabel(psz, pcComplete, CPP65_CONSTRUCTION_LABEL);
	psz->Append('_');
	psz->Append(iOffset);
	psz->Append('_');
	AppendClassName(psz, pcClass);
}

//////////////////////////////////////////////////////////////////////////
//
// A vtable entry's label: R_Pure for a pure virtual function, else the
// function's, or its thunk's when this is adjusted by iAdjust first: __t
// and the adjustment's negation, or __u and the adjustment when it moves this
// on.  AppendVtableEntry writes the thunk.
//
//////////////////////////////////////////////////////////////////////////
//
// The label of a global, a static data member or a static local (S, its
// number in its function, _ and its name; a shared function's has the
// function's label and __ before it).
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendVariableLabel(CChars* psz, CCPPVariable* pcVariable)
{
	if (pcVariable->meStorage == CPPVS_Static)
	{
		AppendStaticLocalLabel(psz, pcVariable, 'S');
		return;
	}
	AppendGlobalLabel(psz, pcVariable);
}


//////////////////////////////////////////////////////////////////////////
//
// The label of a guarded static local's guard: G, its number, _ and its
// name, as its own is S.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendGuardLabel(CChars* psz, CCPPVariable* pcVariable)
{
	AppendStaticLocalLabel(psz, pcVariable, 'G');
}


//////////////////////////////////////////////////////////////////////////
//
// A static local's label of the kind cKind (S its own, G its guard's).
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendStaticLocalLabel(CChars* psz, CCPPVariable* pcVariable, char cKind)
{
	char*	szName;
	int		iLength;

	szName = mpcTokeniser->GetIdentifier(pcVariable->mpcSymbol->mlliName, &iLength);
	if (pcVariable->mpcSharedIn)
	{
		//A shared function's: its label, __S and its number in the function.
		AppendFunctionLabel(psz, pcVariable->mpcSharedIn);
		psz->Append("__");
	}
	psz->Append(cKind);
	psz->Append(pcVariable->miOffset);
	psz->Append('_');
	psz->Append(szName, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
// The routine that constructs what szOf labels when the program starts
// (R_Initialise), or with bFinalise destroys it at exit (R_Finalise).
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendStartUpLabel(CChars* psz, bool bFinalise, const char* szOf)
{
	psz->Append(bFinalise ? "R_Finalise" : "R_Initialise");
	psz->Append(szOf);
}


//////////////////////////////////////////////////////////////////////////
//
// A global's R_Initialise or R_Finalise, by the unit's number for its
// routines (CCPPUnitData::GetStartUp).
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendStartUpLabel(CChars* psz, bool bFinalise, int iNumber)
{
	psz->Append(bFinalise ? "R_Finalise" : "R_Initialise");
	psz->Append(iNumber);
}


//////////////////////////////////////////////////////////////////////////
//
// The exit list node of what szOf labels, which puts it on an exit list
// once it is made.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendExitNodeLabel(CChars* psz, const char* szOf)
{
	psz->Append(szOf);
	psz->Append("__ExitNode");
}


//////////////////////////////////////////////////////////////////////////
//
// The temporary a global reference (szOf) is bound to.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendBoundLabel(CChars* psz, const char* szOf)
{
	psz->Append(szOf);
	psz->Append("__Bound");
}


//////////////////////////////////////////////////////////////////////////
//
// The type info of a type that isn't a class (a class's is
// AppendTypeInfoLabel's of it): _3 and the type mangled.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendTypeInfoLabel(CChars* psz, SCPP65Type* psType)
{
	psz->Append("_3");
	AppendType(psz, psType);
}


//////////////////////////////////////////////////////////////////////////
//
// The name string of the type info szInfo labels (type_info::name).
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendTypeNameLabel(CChars* psz, const char* szInfo)
{
	psz->Append(szInfo);
	psz->Append("__Name");
}


//////////////////////////////////////////////////////////////////////////
//
// The thunk that calls the virtual function in a vtable's slot iSlot
// through a pointer to a member, its vtable pointer at iPointerOffset:
// __vcall_8_3, every class's and unit's.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendVirtualCallLabel(CChars* psz, int iPointerOffset, int iSlot)
{
	psz->Append("__vcall_");
	psz->Append(iPointerOffset);
	psz->Append('_');
	psz->Append(iSlot);
}


//////////////////////////////////////////////////////////////////////////
//
// The function that destroys a thrown object of psType.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendThrownDestroyerLabel(CChars* psz, SCPP65Type* psType)
{
	psz->Append("R_DestroyThrown");
	AppendType(psz, psType);
}


//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendVtableEntryLabel(CChars* psz, CCPPFunction* pcFunction, int iAdjust)
{
	if (pcFunction->mbPure)
	{
		psz->Append("R_Pure");
		return;
	}
	AppendFunctionLabel(psz, pcFunction);
	if (iAdjust < 0)
	{
		psz->Append("__t");
		psz->Append(-iAdjust);
	}
	else if (iAdjust > 0)
	{
		//this moved on, to an overrider after the part (in a virtual base's part): __u and the adjustment.
		psz->Append("__u");
		psz->Append(iAdjust);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Class, or Class__T and the template arguments, after the classes and
// namespaces it is in (Outer__Class), or for a member class template's
// instance after the class that has it.  A class without a name is the unit,
// Unnamed and its number.
//
//////////////////////////////////////////////////////////////////////////
void CCPPMangler::AppendClassName(CChars* psz, CCPPClass* pcClass)
{
	char*	szName;
	int		iLength;

	if (pcClass->mpsLambda && !pcClass->mpsLambda->szName.Empty())
	{
		//The closure of a lambda in a function every unit may write: named for the function.
		psz->Append(&pcClass->mpsLambda->szName);
		return;
	}
	if (pcClass->IsUnnamed())
	{
		//A class without a name is the unit's own (its labels are local to the object).
		psz->Append("Unnamed");
		psz->Append((int)mpapcClasses->Find(pcClass));
		return;
	}
	if (pcClass->mpcTemplate && pcClass->mpcTemplate->mpcPrimary->mpcClass)
	{
		//A member class template's instance is its class's (P<char>::Q<long>), which may be an instance too.
		AppendClassName(psz, pcClass->mpcTemplate->mpcPrimary->mpcClass);
		psz->Append("__");
	}
	else if (pcClass->mpcEnclosing)
	{
		//A class template's instance's member class is its instance's (P<char>::Inner).
		AppendClassName(psz, pcClass->mpcEnclosing);
		psz->Append("__");
	}
	else if (pcClass->mpcSymbol)
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
// A function's name: its identifier, op and the operator's name, or
// opLiteral and a literal operator's suffix.
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
	if (lliName == CPP_DELETING_NAME)
	{
		psz->Append("opDeleting");
		return;
	}
	if (CPP_IS_LITERAL_NAME(lliName))
	{
		//operator "" _k is opLiteral_k.
		psz->Append("opLiteral");
		szName = mpcTokeniser->GetIdentifier(CPP_LITERAL_SUFFIX(lliName), &iLength);
		psz->Append(szName, iLength);
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
// Types in labels, the Itanium codes: v void, b bool, c char, a signed char, h
// unsigned char, w wchar_t, Ds char16_t, Di char32_t, s short, t unsigned short,
// i int, j unsigned, l long, m unsigned long, x long long, y unsigned long long
// (each its type's, however wide), f float, d double,
// e long double, P pointer, R reference, O rvalue reference (T&&), A count _
// array, F return parameters E function, and C, the length and the label of a class.  A volatile type starts
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
	case CPP65TK_NullPointer:
		//std::nullptr_t, as the Itanium ABI has it.
		psz->Append("Dn");
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
		switch (psType->eInteger)
		{
		case CPP65IK_Char:				psz->Append('c');		break;
		case CPP65IK_SignedChar:			psz->Append('a');		break;
		case CPP65IK_UnsignedChar:			psz->Append('h');		break;
		case CPP65IK_WChar:				psz->Append('w');		break;
		case CPP65IK_Char16:				psz->Append("Ds");		break;
		case CPP65IK_Char32:				psz->Append("Di");		break;
		case CPP65IK_Short:				psz->Append('s');		break;
		case CPP65IK_UnsignedShort:			psz->Append('t');		break;
		case CPP65IK_Int:				psz->Append('i');		break;
		case CPP65IK_UnsignedInt:			psz->Append('j');		break;
		case CPP65IK_Long:				psz->Append('l');		break;
		case CPP65IK_UnsignedLong:			psz->Append('m');		break;
		case CPP65IK_LongLong:			psz->Append('x');		break;
		case CPP65IK_UnsignedLongLong:		psz->Append('y');		break;
		default:						psz->Append(psType->bSigned ? 'i' : 'j');	break;
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
		psz->Append(psType->bRvalue ? 'O' : 'R');
		AppendType(psz, psType->psBase);
		break;
	case CPP65TK_Array:
		psz->Append('A');
		psz->Append(psType->iCount);
		psz->Append('_');
		AppendType(psz, psType->psBase);
		break;
	case CPP65TK_Function:
		if (psType->bVolatileFunction)
		{
			psz->Append('V');
		}
		psz->Append(psType->bConstFunction ? "KF" : "F");
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

	bArray = lliName <= CPP_ARRAY_OPERATOR_NAME(0);
	eToken = (ECppTokens)((bArray ? CPP_ARRAY_OPERATOR_NAME(0) : CPP_OPERATOR_NAME(0)) - lliName);
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

