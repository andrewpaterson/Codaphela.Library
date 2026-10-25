/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __KICAD_SCHEM_SYMBOL_H__
#define __KICAD_SCHEM_SYMBOL_H__
#include "BaseLib/ListVariable.h"
#include "KicadSchemProperties.h"
#include "KicadSchemTransform.h"
#include "KicadSchemSymbolInstance.h"


//A symbol placed on the sheet: one unit of a library symbol at a position, angle and mirror.  The library symbol is
//found in the file's lib_symbols by GetLibSymbolName: lib_name when the placed symbol has one (it was edited away
//from the library), otherwise lib_id.
class CKicadSchemSymbol
{
protected:
	CChars					mszLibId;
	CChars					mszLibName;
	CChars					mszUuid;
	SKicadSchemPoint		msPosition;
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
	void						Init(char* szLibId);
	void						Kill(void);

	void						SetLibName(char* szLibName);
	void						SetUuid(char* szUuid);
	void						SetPosition(SKicadSchemPoint* psPosition, float64 fAngle);
	void						SetMirror(EKicadSchemMirror eMirror);
	void						SetUnit(int32 iUnit);
	void						SetBodyStyle(int32 iBodyStyle);
	void						SetInBom(bool bInBom);
	void						SetOnBoard(bool bOnBoard);
	void						SetDoNotPopulate(bool bDoNotPopulate);
	CKicadSchemSymbolInstance*	AddInstance(char* szProject, char* szPath, char* szReference, int32 iUnit);

	char*						GetLibId(void);
	char*						GetLibName(void);
	char*						GetLibSymbolName(void);
	char*						GetUuid(void);
	SKicadSchemPoint*			GetPosition(void);
	float64						GetAngle(void);
	EKicadSchemMirror			GetMirror(void);
	int32						GetUnit(void);
	int32						GetBodyStyle(void);
	bool						IsInBom(void);
	bool						IsOnBoard(void);
	bool						IsDoNotPopulate(void);
	CKicadSchemProperties*		GetProperties(void);
	char*						GetReference(void);
	char*						GetValue(void);

	size						NumInstances(void);
	CKicadSchemSymbolInstance*	GetInstance(size uiIndex);

	bool						GetSheetPosition(SKicadSchemPoint* psDest, SKicadSchemPoint* psLibrary);
};


#endif // __KICAD_SCHEM_SYMBOL_H__

