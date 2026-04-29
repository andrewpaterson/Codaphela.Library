#ifndef __TEXT_BUILDER_UTF_16_LONG_H__
#define __TEXT_BUILDER_UTF_16_LONG_H__
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
#include "BaseLib/ArrayUint32.h"
#include "TextUTF16Long.h"
#include "TextBuilderUTFCommon.h"


class CTextBuilderUTF16Long : public CTextBuilderUTFCommon
{
CONSTRUCTABLE(CTextBuilderUTF16Long)
protected:
	CArrayUint32	mauiUTF32;

public:
	void		Init(void);
	void		Kill(void) override;

	size		NumElements(void) override;
	void*		GetData(void) override;
	uint32*		GetChars(void);

	void		Push(uint32 uiChar);
	bool		IsUTF16Long(void) override;
};


#endif // __TEXT_BUILDER_UTF_16_LONG_H__

