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
#include "CPPClassLayout.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassLayout::Init(CCPPClass* pcClass, int iPack, int iPointerBytes)
{
	mpcClass = pcClass;
	miPack = iPack;
	miPointerBytes = iPointerBytes;
	miBytes = 0;
	miAlign = 1;
	miUnionBytes = 0;
	miUnitBytes = 0;
	miUnitOffset = 0;
	miUnitBits = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassLayout::Kill(void)
{
	mpcClass = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An alignment limited by #pragma pack(iPack) (0 for none).
//
//////////////////////////////////////////////////////////////////////////
int CCPPClassLayout::PackAlignment(int iAlign, int iPack)
{
	return ((iPack > 0) && (iAlign > iPack)) ? iPack : iAlign;
}


//////////////////////////////////////////////////////////////////////////
//
// A type's alignment in a class, as MSVC lays classes out: a scalar is
// aligned to its size (8 for long long and double), a pointer or reference
// to 4, an array as its element and a class to its largest member's
// alignment.
//
//////////////////////////////////////////////////////////////////////////
int CCPPClassLayout::TypeAlignment(SCPP65Type* psType)
{
	if (psType->IsClass())
	{
		return (psType->pcClass->miAlign < 1) ? 1 : psType->pcClass->miAlign;
	}
	if (psType->IsArray())
	{
		return TypeAlignment(psType->psBase);
	}
	if (psType->IsReference() || psType->IsPointer() || psType->IsMemberPointer())
	{
		return 4;
	}
	if (psType->iBytes >= 8)
	{
		return 8;
	}
	if (psType->iBytes >= 4)
	{
		return 4;
	}
	return (psType->iBytes < 1) ? 1 : psType->iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The next thing goes at a multiple of iAlign, and the class is aligned at
// least as it is.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassLayout::Align(int iAlign)
{
	miBytes = (miBytes + iAlign - 1) & ~(iAlign - 1);
	miAlign = (iAlign > miAlign) ? iAlign : miAlign;
}


//////////////////////////////////////////////////////////////////////////
//
// Places everything, and sets the class's alignment and whether it is
// empty.  iClassAlign is the class's alignas, 0 for none: it is kept
// whatever #pragma pack says.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassLayout::Layout(int iClassAlign)
{
	size	i;

	PlaceBases();
	PlaceVtablePointer();
	mpcClass->mcVtable.LayoutSecondaries(mpcClass);
	for (i = 0; i < mpcClass->macFields.NumElements(); i++)
	{
		PlaceField(mpcClass->macFields.Get(i));
	}
	if (mpcClass->mbUnion)
	{
		miBytes = miUnionBytes;
	}
	miAlign = (iClassAlign > miAlign) ? iClassAlign : miAlign;
	miBytes = (miBytes + miAlign - 1) & ~(miAlign - 1);

	mpcClass->mbEmpty = (mpcClass->macFields.NumElements() == 0) && !mpcClass->mcVtable.HasPointer();
	for (i = 0; i < mpcClass->macBases.NumElements(); i++)
	{
		mpcClass->mbEmpty &= mpcClass->macBases.Get(i)->pcClass->mbEmpty;
	}
	mpcClass->miAlign = miAlign;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's size: an object is never smaller than a byte.
//
//////////////////////////////////////////////////////////////////////////
int CCPPClassLayout::GetBytes(void)
{
	return (miBytes > 0) ? miBytes : 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The bases in order.  An empty base takes no space.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassLayout::PlaceBases(void)
{
	SCPPBase*	psBase;
	size		i;

	for (i = 0; i < mpcClass->macBases.NumElements(); i++)
	{
		psBase = mpcClass->macBases.Get(i);
		if (psBase->pcClass->mbEmpty)
		{
			psBase->iOffset = 0;
			continue;
		}
		Align(PackAlignment(psBase->pcClass->miAlign, miPack));
		psBase->iOffset = miBytes;
		miBytes += psBase->pcClass->mpsType->iBytes;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The class's own vtable's pointer is the first base's where that base is
// at the object's start and has one; otherwise a class with virtual
// functions gets one after its bases.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassLayout::PlaceVtablePointer(void)
{
	CCPPVtable*	pcVtable;

	pcVtable = &mpcClass->mcVtable;
	if (mpcClass->mpcBase && mpcClass->mpcBase->mcVtable.HasPointer() && (mpcClass->macBases.Get(0)->iOffset == 0))
	{
		pcVtable->miPointerOffset = mpcClass->mpcBase->mcVtable.miPointerOffset;
	}
	else if (pcVtable->mapcFunctions.NumElements() > 0)
	{
		Align(PackAlignment(miPointerBytes, miPack));
		pcVtable->miPointerOffset = miBytes;
		miBytes += miPointerBytes;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A field goes after the one before it, aligned as its type is or as its
// alignas asks if that is stricter; in a union at 0.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassLayout::PlaceField(SCPPField* psField)
{
	int		iFieldAlign;

	if (psField->IsBitField())
	{
		PlaceBitField(psField);
		return;
	}

	miUnitBytes = 0;
	iFieldAlign = PackAlignment(TypeAlignment(psField->psType), miPack);
	iFieldAlign = (psField->iAlign > iFieldAlign) ? psField->iAlign : iFieldAlign;
	if (mpcClass->mbUnion)
	{
		miBytes = 0;
		miAlign = (iFieldAlign > miAlign) ? iFieldAlign : miAlign;
	}
	else
	{
		Align(iFieldAlign);
	}
	psField->iOffset = miBytes;
	miBytes += psField->psType->iBytes;
	if (mpcClass->mbUnion && (miBytes > miUnionBytes))
	{
		miUnionBytes = miBytes;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Bit-fields share a unit the size of their type, aligned as it is, from
// its lowest bit up: one goes in the open unit when its type is the same
// size and its bits fit, otherwise it opens a unit after what has been
// placed.  A width of 0 closes the unit.  In a union each is at bit 0 of
// offset 0.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClassLayout::PlaceBitField(SCPPField* psField)
{
	int		iTypeBytes;
	int		iFieldAlign;

	iTypeBytes = psField->psType->iBytes;
	if (psField->iBitWidth == 0)
	{
		psField->iOffset = miBytes;
		miUnitBytes = 0;
		return;
	}
	iFieldAlign = PackAlignment(TypeAlignment(psField->psType), miPack);
	miAlign = (iFieldAlign > miAlign) ? iFieldAlign : miAlign;
	if (mpcClass->mbUnion)
	{
		psField->iOffset = 0;
		psField->iBitOffset = 0;
		if (iTypeBytes > miUnionBytes)
		{
			miUnionBytes = iTypeBytes;
		}
		return;
	}
	if ((miUnitBytes == iTypeBytes) && (miUnitBits + psField->iBitWidth <= iTypeBytes * 8))
	{
		psField->iOffset = miUnitOffset;
		psField->iBitOffset = miUnitBits;
		miUnitBits += psField->iBitWidth;
		return;
	}
	miBytes = (miBytes + iFieldAlign - 1) & ~(iFieldAlign - 1);
	psField->iOffset = miBytes;
	psField->iBitOffset = 0;
	miUnitBytes = iTypeBytes;
	miUnitOffset = miBytes;
	miUnitBits = psField->iBitWidth;
	miBytes += iTypeBytes;
}

