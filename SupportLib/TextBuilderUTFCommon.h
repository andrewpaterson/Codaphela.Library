#ifndef __TEXT_BUILDER_UTF_COMMON_H__
#define __TEXT_BUILDER_UTF_COMMON_H__
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


class CTextBuilderUTFCommon : public CKillable
{
CONSTRUCTABLE(CTextBuilderUTFCommon)
public:
			void		Init(void);
			void		Kill(void) override;  //This should do nothing.

	virtual size		NumElements(void) =0;
	virtual void*		GetData(void) =0;

	virtual bool		IsUTF16Short(void);
	virtual bool		IsUTF16Long(void);
};


#endif // __TEXT_BUILDER_UTF_COMMON_H__

