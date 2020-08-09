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
#ifndef __INPUT_CHORD_CRITERIA_DESC_H__
#define __INPUT_CHORD_CRITERIA_DESC_H__
#include "BaseLib/ArrayIntAndPointer.h"
#include "StandardLib/Unknown.h"
#include "StandardLib/ArrayType.h"
#include "InputCategoryGeneric.h"
#include "InputSourceEvaluator.h"
#include "InputChordDesc.h"


enum EInputChordCriteriaDescType
{
	ICCDT_Active,
	ICCDT_Inactive,
	ICCDT_Group,
	ICCDT_Ordered,
};


class CInputSourceDesc;
class CInputDeviceDesc;
class CInputVirtualSourceDesc;
class CInputChordCriteriaDesc : public CUnknown
{
CONSTRUCTABLE(CInputChordCriteriaDesc);
protected:
	EInputChordCriteriaDescType		meType;
	CInputChordDesc*				mpcChordDesc;

public:
	void 							Init(EInputChordCriteriaDescType eType, CInputChordDesc* pcChordDesc);
	void 							Kill(void);
	virtual void					GetInputSourceDescs(CArrayIntAndPointer* apcDest, CInputDeviceDesc* pcDeviceDesc) =0;

	EInputChordCriteriaDescType		GetType(void);
	CInputChordDesc*				GetChordDesc(void);
};


typedef CArrayUnknown /*Type<CInputChordCriteriaDesc>*/	CArrayInputChordCriteriaDesc;


enum EInputChordCriteriaDescActiveType
{
	ICCDAT_SpecificSource,
	ICCDAT_GenericSource,
	ICCDAT_AnyToggleSource,
};


class CInputChordActiveDesc : public CInputChordCriteriaDesc
{
public:
	CONSTRUCTABLE(CInputChordActiveDesc);

	CInputSourceDesc*					mpcSourceDesc;  //Only valy valid for ICCDAT_SpecificSource.
	int									miDescriptionID;  //Only valy valid for ICCDAT_SpecificSource.
	CInputCategoryGeneric*				mpcGeneric;  //Only valy valid for ICCDAT_GenericSource.
	EInputChordType						meChordType;  //Only valy valid for ICCDAT_SpecificSource and ICCDAT_GenericSource.
	EInputChordCriteriaDescActiveType	meActiveType;

	void 				Init(CInputSourceDesc* pcSourceDesc, int iDescriptionID, EInputChordCriteriaDescType eType, CInputChordDesc* pcChordDesc, EInputChordType eChordType);
	void 				Init(CInputCategoryGeneric* pcSource, EInputChordCriteriaDescType eType, CInputChordDesc* pcChordDesc, EInputChordType eChordType);
	void 				Init(EInputChordCriteriaDescType eType, CInputChordDesc* pcChordDesc);
	void 				Kill(void);
	void 				GetInputSourceDescs(CArrayIntAndPointer* apcDest, CInputDeviceDesc* pcDeviceDesc);
	CInputSourceDesc*	GetSourceDesc(void);
	int					GetDescriptionID(void);
};


class CInputChordCollectiveDesc : public CInputChordCriteriaDesc
{
public:
	CONSTRUCTABLE(CInputChordCollectiveDesc);

	CArrayInputChordCriteriaDesc	macCriteria;

	void 						Init(EInputChordCriteriaDescType eType, CInputChordDesc* pcChordDesc);
	void 						Kill(void);

	CInputChordActiveDesc*		AddActiveOrInactive(void);
	CInputChordCollectiveDesc*	AddGroupOrOrdered(void);
	CInputChordActiveDesc*		AddActive(CInputSourceDesc* pcSourceDesc, int iDescriptionID);
	CInputChordActiveDesc*		AddInactive(CInputSourceDesc* pcSourceDesc, int iDescriptionID);
	CInputChordCollectiveDesc*	AddGroup(void);
	void						RemoveLast(void);
	void						GetInputSourceDescs(CArrayIntAndPointer* apcDest, CInputDeviceDesc* pcDeviceDesc);
};


#endif // __INPUT_CHORD_CRITERIA_DESC_H__

