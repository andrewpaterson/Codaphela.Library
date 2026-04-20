#ifndef __TEXT_RUN_H__
#define __TEXT_RUN_H__
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
#include "BaseLib/Int2.h"
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "Font.h"
#include "TextRunCommon.h"


class CText;
class CTextRun
{
CONSTRUCTABLE(CTextRun)
protected:
	SInt2						msPosition;
	CFont*						mpcFont;  //Not a Ptr<CFont> because CTextRun does not extend CObject.
	CText*						mpcText;  //Ditto Ptr<CText>
	CArrayTextRunCommonPtr		mapcText;

public:
	void			Init(SInt2* psPosition, CFont* mpcFont, CText* pcText);
	void 			Kill(void);

	bool			Load(CObjectReader* pcFile);
	bool			Save(CObjectWriter* pcFile);
	CMallocator*	GetMalloc(void);
};


typedef CArrayTemplate<CTextRun>	CArrayTextRun;


#endif // __TEXT_RUN_H__

