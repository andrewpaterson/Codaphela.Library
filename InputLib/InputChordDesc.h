/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INPUT_CHORD_DESC_H__
#define __INPUT_CHORD_DESC_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/SetType.h"
#include "StandardLib/Action.h"
#include "InputCategoryGeneric.h"


class CInputSourceDesc;
class CInputChordCriteriaDesc;
class CInputChordActiveDesc;
class CInputChordCollectiveDesc;
class CInputChordDesc : public CUnknown
{
CONSTRUCTABLE(CInputChordDesc);
protected:
	CInputChordCriteriaDesc*	mpcRootCriteriaDesc;
	CChars						mszActionName;

public:
	void 						Init(char* szActionName);
	void 						Kill(void);

	CInputChordActiveDesc*		AsActiveOrInactive(void);
	void 						AsActive(CInputCategoryGeneric* pcGeneric);
	void 						AsInactive(CInputCategoryGeneric* pcGeneric);
	void 						AsActive(CInputSourceDesc* pcSourceDesc, int iDescriptionID);
	void 						AsInactive(CInputSourceDesc* pcSourceDesc, int iDescriptionID);
	CInputChordCollectiveDesc*	AsOrdered(void);
	CInputChordCollectiveDesc*	AsGroup(void);
	CInputChordCriteriaDesc*	GetRootCriteriaDesc(void);
	char*						GetActionName(void);
};


typedef CSetType<CInputChordDesc>	CSetInputChordDesc;


#endif // __INPUT_CHORD_DESC_H__

