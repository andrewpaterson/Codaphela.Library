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
#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayBlock.h"
#include "BaseLib/ArrayTemplateMinimal.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/ArrayIntMinimal.h"
#include "BaseLib/Float4x4.h"
#include "StandardLib/Object.h"


class CConnection : public CObject
{
CONSTRUCTABLE(CConnection);
DESTRUCTABLE(CConnection);
public:
	SFloat4x4			msWorldMatrix;
	SFloat4x4			msLocalMatrix;  //It's possible to work this out given a parent matrix.
	bool				mbSelected;
	bool				mbTopLevel;
	bool				mbParity;

	Ptr<CConnection> 	Init(void);
	void 				Free(void);
	void				Class(void);

	bool				Load(CObjectReader* pcFile);
	bool				Save(CObjectWriter* pcFile);
	void				Copy(CConnection* pcConnection);
};


#endif //__CONNECTION_H__

