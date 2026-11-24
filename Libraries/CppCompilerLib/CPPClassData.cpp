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
#include "CPPUnit.h"
#include "CPPClassData.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::Kill(void)
{
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__2: the addresses of the virtual functions, written the first time an
// object of the class is made.  Its functions are generated.  Pure virtual
// functions are R_Pure, which stops.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::EnsureVtable(CCPPClass* pcClass)
{
	CArrayCPPVtableEntries	asEntries;
	CChars					szLabel;

	if (pcClass->mcVtable.mbWritten)
	{
		return;
	}
	pcClass->mcVtable.mbWritten = true;

	//Each unit that makes one of a class's objects writes its vtable in a COMDAT section
	//(the linker keeps one); a class without a name has its own.
	szLabel.Init();
	mpcUnit->mcMangler.AppendVtableLabel(&szLabel, pcClass);
	if (!mpcUnit->Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	if (pcClass->IsUnnamed())
	{
		CCPPUnitData::AppendSection(mpcUnit->mcData.GetText(), "rodata", &szLabel, NULL);
	}
	else
	{
		CCPPUnitData::AppendSection(mpcUnit->mcData.GetText(), "rodata", &szLabel, ", comdat");
		CCPPUnitData::AppendGlobal(mpcUnit->mcData.GetText(), &szLabel);
	}
	mpcUnit->mcData.GetText()->Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
	mpcUnit->mcData.GetText()->AppendNewLine();

	asEntries.Init();
	pcClass->mcVtable.GetEntries(&asEntries);
	AppendVtableParts(pcClass, &szLabel, &asEntries);
	asEntries.Kill();
	szLabel.Kill();

	EnsureTypeInfo(pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable's words (CCPPVtable::GetEntries), with the label after the first
// part's header.  The headers have pcClass's type info.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AppendVtableParts(CCPPClass* pcClass, CChars* pszLabel, CArrayCPPVtableEntries* pasEntries)
{
	SCPPVtableEntry*	psEntry;
	size				i;
	bool				bLabelled;

	bLabelled = false;
	for (i = 0; i < pasEntries->NumElements(); i++)
	{
		psEntry = pasEntries->Get(i);
		if (psEntry->eKind == CPPVE_Offset)
		{
			mpcUnit->mcData.GetText()->Append("\t\t.long ")->Append(psEntry->iAdjust);
			mpcUnit->mcData.GetText()->AppendNewLine();
			continue;
		}
		if (psEntry->eKind == CPPVE_Header)
		{
			AppendVtableHeader(pcClass, psEntry->iVtablePointer);
		}
		else
		{
			AppendVtableEntry(psEntry->pcFunction, psEntry->iAdjust);
		}
		if (!bLabelled)
		{
			mpcUnit->mcData.GetText()->Append(*pszLabel);
			mpcUnit->mcData.GetText()->Append(':');
			mpcUnit->mcData.GetText()->AppendNewLine();
			bLabelled = true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__4, the VTT of a class with virtual bases (CPP65_VTT_LABEL): the
// class's vtable, then the entries of each base its constructors and
// destructor make with a base version, the non-virtual bases' then the
// virtual bases' (VTTIndex).  A base subobject's entries are its
// construction vtable then its own non-virtual bases' (VTTEntries), which is
// what the base version is passed.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::EnsureVTT(CCPPClass* pcClass)
{
	CChars				szLabel;
	CChars				szWords;
	SCPPBase*			psBase;
	SCPPVirtualBase*	psVirtual;
	size				i;

	szLabel.Init();
	mpcUnit->mcMangler.AppendVTTLabel(&szLabel, pcClass);
	if (!mpcUnit->Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	EnsureVtable(pcClass);

	//The construction vtables are written first, each in its own section.
	szWords.Init();
	szWords.Append("\t\t.long ");
	mpcUnit->mcMangler.AppendVtableLabel(&szWords, pcClass);
	szWords.AppendNewLine();
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual && psBase->pcClass->HasVirtualBases())
		{
			AppendVTTEntries(pcClass, psBase->pcClass, psBase->iOffset, &szWords);
		}
	}
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		psVirtual = pcClass->macVirtualBases.Get(i);
		if (psVirtual->pcClass->HasVirtualBases())
		{
			AppendVTTEntries(pcClass, psVirtual->pcClass, psVirtual->iOffset, &szWords);
		}
	}

	if (pcClass->IsUnnamed())
	{
		CCPPUnitData::AppendSection(mpcUnit->mcData.GetText(), "rodata", &szLabel, NULL);
	}
	else
	{
		CCPPUnitData::AppendSection(mpcUnit->mcData.GetText(), "rodata", &szLabel, ", comdat");
		CCPPUnitData::AppendGlobal(mpcUnit->mcData.GetText(), &szLabel);
	}
	mpcUnit->mcData.GetText()->Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
	mpcUnit->mcData.GetText()->AppendNewLine();
	mpcUnit->mcData.GetText()->Append(szLabel);
	mpcUnit->mcData.GetText()->Append(':');
	mpcUnit->mcData.GetText()->AppendNewLine();
	mpcUnit->mcData.GetText()->Append(szWords);
	szWords.Kill();
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The VTT entries (in pszWords) of the pcClass subobject at iOffset in a
// pcComplete: its construction vtable, which is written, then its non-
// virtual bases' entries.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AppendVTTEntries(CCPPClass* pcComplete, CCPPClass* pcClass, int iOffset, CChars* pszWords)
{
	CCPPVtable				cVtable;
	CArrayCPPVtableEntries	asEntries;
	CChars					szLabel;
	SCPPBase*				psBase;
	size					i;

	szLabel.Init();
	mpcUnit->mcMangler.AppendConstructionVtableLabel(&szLabel, pcComplete, pcClass, iOffset);
	pszWords->Append("\t\t.long ");
	pszWords->Append(szLabel);
	pszWords->AppendNewLine();
	if (mpcUnit->Emit(&szLabel))
	{
		if (pcComplete->IsUnnamed())
		{
			CCPPUnitData::AppendSection(mpcUnit->mcData.GetText(), "rodata", &szLabel, NULL);
		}
		else
		{
			CCPPUnitData::AppendSection(mpcUnit->mcData.GetText(), "rodata", &szLabel, ", comdat");
			CCPPUnitData::AppendGlobal(mpcUnit->mcData.GetText(), &szLabel);
		}
		mpcUnit->mcData.GetText()->Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
		mpcUnit->mcData.GetText()->AppendNewLine();
		cVtable.Init();
		cVtable.LayoutConstruction(pcClass, pcComplete, iOffset);
		asEntries.Init();
		cVtable.GetEntries(&asEntries);
		AppendVtableParts(pcClass, &szLabel, &asEntries);
		asEntries.Kill();
		cVtable.Kill();
		EnsureTypeInfo(pcClass);
	}
	szLabel.Kill();

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual && psBase->pcClass->HasVirtualBases())
		{
			AppendVTTEntries(pcComplete, psBase->pcClass, iOffset + psBase->iOffset, pszWords);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many words of VTT a base version of a constructor or destructor of
// pcClass is given: its construction vtable, then its non-virtual bases'.
//
//////////////////////////////////////////////////////////////////////////
size CCPPClassData::VTTEntries(CCPPClass* pcClass)
{
	SCPPBase*	psBase;
	size		i;
	size		uiEntries;

	uiEntries = 1;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual && psBase->pcClass->HasVirtualBases())
		{
			uiEntries += VTTEntries(psBase->pcClass);
		}
	}
	return uiEntries;
}


//////////////////////////////////////////////////////////////////////////
//
// Where in pcClass's VTT (in words) the entries for its base pcBase are, a
// direct non-virtual base or a virtual base that has virtual bases of its
// own; -1 if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
int CCPPClassData::VTTIndex(CCPPClass* pcClass, CCPPClass* pcBase)
{
	SCPPBase*			psBase;
	SCPPVirtualBase*	psVirtual;
	size				i;
	size				uiIndex;

	uiIndex = 1;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual && psBase->pcClass->HasVirtualBases())
		{
			if (psBase->pcClass == pcBase)
			{
				return (int)uiIndex;
			}
			uiIndex += VTTEntries(psBase->pcClass);
		}
	}
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		psVirtual = pcClass->macVirtualBases.Get(i);
		if (psVirtual->pcClass->HasVirtualBases())
		{
			if (psVirtual->pcClass == pcBase)
			{
				return (int)uiIndex;
			}
			uiIndex += VTTEntries(psVirtual->pcClass);
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The two words before each part of a vtable, for dynamic_cast: how far the
// object's start is from the vtable pointer using that part (iVtablePointer is
// where that pointer is in the object), then the class's type info (a
// construction vtable's is its subobject's class's, which typeid and
// dynamic_cast give while it is made).
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AppendVtableHeader(CCPPClass* pcClass, int iVtablePointer)
{
	mpcUnit->mcData.GetText()->Append("\t\t.long ");
	mpcUnit->mcData.GetText()->Append(-iVtablePointer);
	mpcUnit->mcData.GetText()->AppendNewLine();
	mpcUnit->mcData.GetText()->Append("\t\t.long ");
	mpcUnit->mcMangler.AppendTypeInfoLabel(mpcUnit->mcData.GetText(), pcClass);
	mpcUnit->mcData.GetText()->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__3: the class's type info, for typeid and dynamic_cast: its name and
// kind (CPP65_TYPE_INFO_NAME_OFFSET), then every subobject of an object of the
// class, the object itself first: the subobject class's type info, its
// offset in the object and its flags (CPP65_TYPE_INFO_PUBLIC,
// CPP65_TYPE_INFO_VIRTUAL, which virtual base it is or is in, and in the
// class's own entry where its vtable pointer is).  A zero ends it.  Each unit that needs it writes
// it in a COMDAT section (the linker keeps one) so its address identifies the
// class.  The type infos of the bases are written too.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::EnsureTypeInfo(CCPPClass* pcClass)
{
	CChars		szLabel;
	CChars		szName;
	CCPPClass*	pcVirtual;
	size		i;
	uint32		uiVtable;

	if (pcClass->mbTypeInfo)
	{
		return;
	}
	pcClass->mbTypeInfo = true;

	szLabel.Init();
	mpcUnit->mcMangler.AppendTypeInfoLabel(&szLabel, pcClass);
	if (!mpcUnit->Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	szName.Init();
	AppendTypeInfoName(&szLabel, pcClass->mpsType, !pcClass->IsUnnamed(), &szName);
	if (pcClass->IsUnnamed())
	{
		CCPPUnitData::AppendSection(mpcUnit->mcData.GetText(), "rodata", &szLabel, NULL);
	}
	else
	{
		CCPPUnitData::AppendSection(mpcUnit->mcData.GetText(), "rodata", &szLabel, ", comdat");
		CCPPUnitData::AppendGlobal(mpcUnit->mcData.GetText(), &szLabel);
	}
	szLabel.Kill();
	mpcUnit->mcData.GetText()->Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
	mpcUnit->mcData.GetText()->AppendNewLine();
	mpcUnit->mcMangler.AppendTypeInfoLabel(mpcUnit->mcData.GetText(), pcClass);
	mpcUnit->mcData.GetText()->Append(':');
	mpcUnit->mcData.GetText()->AppendNewLine();
	AppendTypeInfoHeader(&szName, CPP65TIK_Class);
	uiVtable = 0;
	if (pcClass->IsPolymorphic() || (pcClass->macVirtualBases.NumElements() > 0))
	{
		uiVtable = ((uint32)pcClass->GetVtablePointerOffset() + 1) << CPP65_TYPE_INFO_VTABLE_SHIFT;
	}
	AppendTypeInfoBases(pcClass, 0, CPP65_TYPE_INFO_PUBLIC, uiVtable);
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		pcVirtual = pcClass->macVirtualBases.Get(i)->pcClass;
		AppendTypeInfoBases(pcVirtual, pcClass->macVirtualBases.Get(i)->iOffset, CPP65_TYPE_INFO_VIRTUAL | (mpcUnit->GetClasses()->IsPublicBase(pcClass, pcVirtual) ? CPP65_TYPE_INFO_PUBLIC : 0),
							((uint32)pcClass->GetVirtualBaseIndex(pcVirtual) + 1) << CPP65_TYPE_INFO_IN_VIRTUAL_SHIFT);
	}
	mpcUnit->mcData.GetText()->Append("\t\t.long 0");
	mpcUnit->mcData.GetText()->AppendNewLine();
	szName.Kill();

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		EnsureTypeInfo(pcClass->macBases.Get(i)->pcClass);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type info entries for the class's subobject at iOffset and its non-
// virtual bases' (a virtual base's are the complete object's, once), with
// its flags (uiFlags): whether only public bases lead to it, and whether it
// is a virtual base.  uiHigh is the flags word's bits from 8 up: which
// virtual base the subobject is or is in, which its bases have too, and for
// the class's own entry where its vtable pointer is, which they don't.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AppendTypeInfoBases(CCPPClass* pcClass, int iOffset, uint32 uiFlags, uint32 uiHigh)
{
	SCPPBase*	psBase;
	size		i;

	mpcUnit->mcData.GetText()->Append("\t\t.long ");
	mpcUnit->mcMangler.AppendTypeInfoLabel(mpcUnit->mcData.GetText(), pcClass);
	mpcUnit->mcData.GetText()->AppendNewLine();
	mpcUnit->mcData.GetText()->Append("\t\t.long ");
	mpcUnit->mcData.GetText()->Append(iOffset);
	mpcUnit->mcData.GetText()->AppendNewLine();
	mpcUnit->mcData.GetText()->Append("\t\t.long ");
	mpcUnit->mcData.GetText()->Append((int)(uiFlags | uiHigh));
	mpcUnit->mcData.GetText()->AppendNewLine();
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual)
		{
			AppendTypeInfoBases(psBase->pcClass, iOffset + psBase->iOffset, (psBase->eAccess == CPPA_Public) ? (uiFlags & CPP65_TYPE_INFO_PUBLIC) : 0, uiHigh & CPP65_TYPE_INFO_IN_VIRTUAL_MASK);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A type info's header, in the section being written: its name's label
// (pszName, AppendTypeInfoName) and its kind's word (an ECPP65TypeInfoKind,
// and for a pointer CPP65_TYPE_INFO_POINTEE_CONST and _VOLATILE).
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AppendTypeInfoHeader(CChars* pszName, uint32 uiKind)
{
	mpcUnit->mcData.GetText()->Append("\t\t.long ");
	mpcUnit->mcData.GetText()->Append(pszName);
	mpcUnit->mcData.GetText()->AppendNewLine();
	mpcUnit->mcData.GetText()->Append("\t\t.long ");
	mpcUnit->mcData.GetText()->Append((int)uiKind);
	mpcUnit->mcData.GetText()->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The name a type info points to (pszInfo's, with __Name: *pszName), a
// string of its own written before it: the type as the program writes it
// (CCPPTypeWriter, shown).  COMDAT and global with a type info that is (bShared).
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AppendTypeInfoName(CChars* pszInfo, SCPP65Type* psType, bool bShared, CChars* pszName)
{
	CChars			szText;
	CCPPTypeWriter	cWriter;
	size			i;

	mpcUnit->mcMangler.AppendTypeNameLabel(pszName, pszInfo->Text());
	szText.Init();
	cWriter.Init(mpcUnit, CPPTW_Shown);
	cWriter.AppendType(psType, &szText);
	CCPPUnitData::AppendSection(mpcUnit->mcData.GetText(), "rodata", pszName, bShared ? ", comdat" : NULL);
	if (bShared)
	{
		CCPPUnitData::AppendGlobal(mpcUnit->mcData.GetText(), pszName);
	}
	mpcUnit->mcData.GetText()->Append(pszName);
	mpcUnit->mcData.GetText()->Append(':');
	mpcUnit->mcData.GetText()->AppendNewLine();
	mpcUnit->mcData.GetText()->Append("\t\t.byte ");
	for (i = 0; i < szText.Length(); i++)
	{
		mpcUnit->mcData.GetText()->Append((int)(uint8)szText.GetChar(i));
		mpcUnit->mcData.GetText()->Append(", ");
	}
	mpcUnit->mcData.GetText()->Append('0');
	mpcUnit->mcData.GetText()->AppendNewLine();
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The routine an asm statement is: its text in a routine of the unit's own
// (local to its object), which the statement calls.  Its label is in
// *pszLabel.  [dcl.asm]
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AsmRoutine(const char* szText, CChars* pszLabel)
{
	pszLabel->Append("__asm");
	pszLabel->Append(mpcUnit->miLabels++);
	mpcUnit->mszThunks.Append(pszLabel)->Append(":\n");
	mpcUnit->mpcTarget->AppendAsmRoutine(&mpcUnit->mszThunks, szText);
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable entry: the function, or a thunk that adds iAdjust to this and jumps to
// it.  Each thunk is written once.  Pure virtual functions are R_Pure.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AppendVtableEntry(CCPPFunction* pcFunction, int iAdjust)
{
	CChars	szLabel;
	CChars	szFunction;

	mpcUnit->mcData.GetText()->Append("\t\t.long ");
	if (pcFunction->mbPure)
	{
		mpcUnit->mcMangler.AppendVtableEntryLabel(mpcUnit->mcData.GetText(), pcFunction, iAdjust);
		mpcUnit->mbPureVirtual = true;
		mpcUnit->mcData.GetText()->AppendNewLine();
		return;
	}

	mpcUnit->RequireFunction(pcFunction);
	szLabel.Init();
	mpcUnit->mcMangler.AppendVtableEntryLabel(&szLabel, pcFunction, iAdjust);
	if (iAdjust != 0)
	{
		if (mpcUnit->Emit(&szLabel))
		{
			AppendThunkLabel(&szLabel, !pcFunction->IsUnitFunction());
			szFunction.Init();
			mpcUnit->mcMangler.AppendFunctionLabel(&szFunction, pcFunction);
			mpcUnit->mpcTarget->AppendAdjustThunk(&mpcUnit->mszThunks, iAdjust, szFunction.Text());
			szFunction.Kill();
		}
	}
	mpcUnit->mcData.GetText()->Append(szLabel);
	mpcUnit->mcData.GetText()->AppendNewLine();
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The label of the thunk a pointer to a virtual member function points to (as
// MSVC's vcall thunks): it jumps to the function in the object's vtable
// (CCPPTarget::AppendVirtualCallThunk).  Thunks are named by the vtable
// pointer's offset and the slot, __vcall_8_3, so every class and unit shares
// them (comdat).
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AppendVirtualCallThunk(CChars* psz, CCPPFunction* pcFunction)
{
	CChars	szLabel;

	szLabel.Init();
	mpcUnit->mcMangler.AppendVirtualCallLabel(&szLabel, pcFunction->mpcClass->mcVtable.miPointerOffset, pcFunction->miSlot);
	if (mpcUnit->Emit(&szLabel))
	{
		AppendThunkLabel(&szLabel, true);
		mpcUnit->mpcTarget->AppendVirtualCallThunk(&mpcUnit->mszThunks, pcFunction->mpcClass->mcVtable.miPointerOffset, CPP65_VTABLE_ENTRY_BYTES * pcFunction->miSlot);
	}
	psz->Append(szLabel);
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A thunk's section and label, to be followed by its code.  One every unit
// may write is global, and comdat so the linker keeps one.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AppendThunkLabel(CChars* pszLabel, bool bComdat)
{
	if (bComdat)
	{
		CCPPUnitData::AppendSection(&mpcUnit->mszThunks, "text", pszLabel, ", comdat");
		CCPPUnitData::AppendGlobal(&mpcUnit->mszThunks, pszLabel);
	}
	else
	{
		CCPPUnitData::AppendSection(&mpcUnit->mszThunks, "text", pszLabel, NULL);
	}
	mpcUnit->mszThunks.Append(*pszLabel);
	mpcUnit->mszThunks.Append(':');
	mpcUnit->mszThunks.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The label of a type's type info, the address typeid gives, written if it
// isn't yet: a class's is _Class__3 (EnsureTypeInfo), any other type's _3
// and the type mangled, in a COMDAT section, so its address is the type's in
// the whole program (no name starts with a digit).  Its name and kind, then
// for a pointer its pointee's type info, for a pointer to a member the
// member's type's and the class's (CPP65_TYPE_INFO_NAME_OFFSET); the kind
// word of both says whether what is pointed to is const or volatile.  The type
// infos it points to are written first, each a section of its own.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassData::AppendTypeidLabel(CChars* psz, SCPP65Type* psType)
{
	CChars				szLabel;
	CChars				szName;
	CChars				szFirst;
	CChars				szSecond;
	ECPP65TypeInfoKind	eKind;
	uint32				uiPointee;

	if (psType->IsClass())
	{
		EnsureTypeInfo(psType->pcClass);
		mpcUnit->mcMangler.AppendTypeInfoLabel(psz, psType->pcClass);
		return;
	}
	szLabel.Init();
	mpcUnit->mcMangler.AppendTypeInfoLabel(&szLabel, psType);
	if (mpcUnit->Emit(&szLabel))
	{
		szFirst.Init();
		szSecond.Init();
		uiPointee = 0;
		switch (psType->eKind)
		{
		case CPP65TK_Pointer:
			eKind = CPP65TIK_Pointer;
			uiPointee = (psType->psBase->bConst ? CPP65_TYPE_INFO_POINTEE_CONST : 0) | (psType->psBase->bVolatile ? CPP65_TYPE_INFO_POINTEE_VOLATILE : 0);
			AppendTypeidLabel(&szFirst, psType->psBase->Unqualified());
			break;
		case CPP65TK_MemberPointer:
			eKind = CPP65TIK_MemberPointer;
			uiPointee = (psType->psBase->bConst ? CPP65_TYPE_INFO_POINTEE_CONST : 0) | (psType->psBase->bVolatile ? CPP65_TYPE_INFO_POINTEE_VOLATILE : 0);
			AppendTypeidLabel(&szFirst, psType->psBase->Unqualified());
			AppendTypeidLabel(&szSecond, psType->pcClass->mpsType);
			break;
		case CPP65TK_Array:		eKind = CPP65TIK_Array;		break;
		case CPP65TK_Function:	eKind = CPP65TIK_Function;	break;
		case CPP65TK_NullPointer:	eKind = CPP65TIK_NullPointer;	break;
		default:				eKind = psType->pcEnum ? CPP65TIK_Enum : CPP65TIK_Fundamental;	break;
		}
		szName.Init();
		AppendTypeInfoName(&szLabel, psType, true, &szName);
		CCPPUnitData::AppendSection(mpcUnit->mcData.GetText(), "rodata", &szLabel, ", comdat");
		CCPPUnitData::AppendGlobal(mpcUnit->mcData.GetText(), &szLabel);
		mpcUnit->mcData.GetText()->Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
		mpcUnit->mcData.GetText()->AppendNewLine();
		mpcUnit->mcData.GetText()->Append(&szLabel);
		mpcUnit->mcData.GetText()->Append(':');
		mpcUnit->mcData.GetText()->AppendNewLine();
		AppendTypeInfoHeader(&szName, (uint32)eKind | uiPointee);
		if (szFirst.Length() > 0)
		{
			mpcUnit->mcData.GetText()->Append("\t\t.long ")->Append(&szFirst);
			mpcUnit->mcData.GetText()->AppendNewLine();
		}
		if (szSecond.Length() > 0)
		{
			mpcUnit->mcData.GetText()->Append("\t\t.long ")->Append(&szSecond);
			mpcUnit->mcData.GetText()->AppendNewLine();
		}
		szName.Kill();
		szSecond.Kill();
		szFirst.Kill();
	}
	psz->Append(&szLabel);
	szLabel.Kill();
}

