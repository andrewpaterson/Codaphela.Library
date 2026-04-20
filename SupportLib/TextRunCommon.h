#ifndef __TEXT_RUN_COMMON_H__
#define __TEXT_RUN_COMMON_H__
/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Constructable.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/Killable.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"


class CTextRunCommon : public CKillable
{
CONSTRUCTABLE(CTextRunCommon)
protected:
	size		muiNumChars;
	int16		miStepToEnd;

public:
			void		Init(size uiNumChars);
			void		Kill(void) override;  //This should do nothing.

	virtual bool		Load(CObjectReader* pcFile) =0;
	virtual bool		Save(CObjectWriter* pcFile) =0;
};


typedef	CArrayTemplatePtr<CTextRunCommon>	CArrayTextRunCommonPtr;


#endif // __TEXT_RUN_COMMON_H__

