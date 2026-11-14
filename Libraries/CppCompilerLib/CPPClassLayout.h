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
#ifndef __C_P_P_CLASS_LAYOUT_H__
#define __C_P_P_CLASS_LAYOUT_H__
#include "CPPClass.h"


//Lays a class out as MSVC does (BaseLib reads and writes its classes as raw bytes): the bases in order (an
//empty base takes no space), the vtable pointer if the first base's isn't shared, then the fields, each
//aligned to at most #pragma pack's bytes.  The class's bases must be complete, its fields collected and its
//own vtable laid out (CCPPVtable::LayoutOwn).  It places things and reports nothing: what is wrong with a
//member was said when the member was declared.
class CCPPClassLayout
{
protected:
	CCPPClass*	mpcClass;
	int			miPack;				//#pragma pack's bytes, 0 for none.
	int			miPointerBytes;
	int			miBytes;			//Where the next thing goes.
	int			miAlign;
	int			miUnionBytes;		//A union's largest member.
	int			miUnitBytes;		//The open bit-field unit's size, 0 if none is open.
	int			miUnitOffset;
	int			miUnitBits;			//The open unit's bits that are taken.

public:
	void		Init(CCPPClass* pcClass, int iPack, int iPointerBytes);
	void		Kill(void);

	void		Layout(int iClassAlign);
	int			GetBytes(void);

	static int	TypeAlignment(SCPP65Type* psType);
	static int	PackAlignment(int iAlign, int iPack);

protected:
	void		Align(int iAlign);
	void		PlaceBases(void);
	void		PlaceVtablePointer(void);
	void		PlaceField(SCPPField* psField);
	void		PlaceBitField(SCPPField* psField);
};


#endif // __C_P_P_CLASS_LAYOUT_H__
