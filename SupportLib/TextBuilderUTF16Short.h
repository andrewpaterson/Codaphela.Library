#ifndef __TEXT_BUILDER_UTF_16_SHORT_H__
#define __TEXT_BUILDER_UTF_16_SHORT_H__
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
#include "BaseLib/ArrayUint16.h"
#include "TextUTF16Short.h"
#include "TextBuilderUTFCommon.h"


class CTextBuilderUTF16Short : public CTextBuilderUTFCommon
{
CONSTRUCTABLE(CTextBuilderUTF16Short)
protected:
	CArrayUint16	mauiUTF16;

public:
	void		Init(void);
	void		Kill(void) override;
	
	size		NumElements(void) override;
	void*		GetData(void) override;
	uint16*		GetChars(void);

	void		Push(uint16 uiChar);

	bool		IsUTF16Short(void) override;
};


#endif // __TEXT_BUILDER_UTF_16_SHORT_H__

