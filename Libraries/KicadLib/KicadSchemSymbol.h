/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadLib

Codaphela KicadLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __KICAD_SCHEM_SYMBOL_H__
#define __KICAD_SCHEM_SYMBOL_H__
#include "BaseLib/ListVariable.h"
#include "KicadSchemProperties.h"
#include "KicadSchemTransform.h"
#include "KicadSchemSymbolInstance.h"


class CKicadSchemLibPin;


//A symbol placed on the sheet: one unit of a library symbol at a position, angle and mirror.  The library symbol is
//found in the file's lib_symbols by GetLibSymbolName: lib_name when the placed symbol has one (it was edited away
//from the library), otherwise lib_id.
class CKicadSchemSymbol
{
protected:
	CChars					mszLibId;
	CChars					mszLibName;
	CChars					mszUuid;
	SFloat64Vec2			msPosition;
	float64					mfAngle;
	EKicadSchemMirror		meMirror;
	int32					miUnit;
	int32					miBodyStyle;
	bool					mbInBom;
	bool					mbOnBoard;
	bool					mbDoNotPopulate;
	CKicadSchemProperties	mcProperties;
	CListVariable			macInstances;

public:
	void						Init(const char* szLibId);
	void						Kill(void);

	void						SetLibName(const char* szLibName);
	void						SetUuid(const char* szUuid);
	void						SetPosition(SFloat64Vec2* psPosition, float64 fAngle);
	void						SetMirror(EKicadSchemMirror eMirror);
	void						SetUnit(int32 iUnit);
	void						SetBodyStyle(int32 iBodyStyle);
	void						SetInBom(bool bInBom);
	void						SetOnBoard(bool bOnBoard);
	void						SetDoNotPopulate(bool bDoNotPopulate);
	CKicadSchemSymbolInstance*	AddInstance(const char* szProject, const char* szPath, const char* szReference, int32 iUnit);

	const char*					GetLibId(void) const;
	const char*					GetLibName(void) const;
	const char*					GetLibSymbolName(void) const;
	const char*					GetUuid(void) const;
	SFloat64Vec2*				GetPosition(void);
	float64						GetAngle(void) const;
	EKicadSchemMirror			GetMirror(void) const;
	int32						GetUnit(void) const;
	int32						GetBodyStyle(void) const;
	bool						IsUnitPin(CKicadSchemLibPin* pcPin);
	bool						IsInBom(void) const;
	bool						IsOnBoard(void) const;
	bool						IsDoNotPopulate(void) const;
	CKicadSchemProperties*		GetProperties(void);
	const char*					GetReference(void);
	const char*					GetValue(void);

	size						NumInstances(void) const;
	CKicadSchemSymbolInstance*	GetInstance(size uiIndex);

	bool						GetSheetPosition(SFloat64Vec2* psDest, SFloat64Vec2* psLibrary);
};


#endif // __KICAD_SCHEM_SYMBOL_H__

