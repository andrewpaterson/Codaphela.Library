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
#ifndef __INSTANCE_H__
#define __INSTANCE_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/LinkedListTemplate.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayBlock.h"
#include "BaseLib/ArrayTemplateMinimal.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/ArrayIntMinimal.h"
#include "StandardLib/NamedObject.h"
#include "TrackerTypes.h"


class CInstance : public CNamedObject
{
CONSTRUCTABLE(CInstance);
public:

	ETrackerType		meType;  //Which Tracker to look this object up in.
	int					miObjectIndex;
	CArrayIntMinimal	maiConnections;  //Matricies, just one unless the the object is a skinned mesh.
	
	void 	Init(void);
	void 	Free(void);

	BOOL	Load(CObjectReader* pcFile);
	BOOL	Save(CObjectWriter* pcFile);
	void	Copy(CInstance* pcInstance);
};


#endif // __INSTANCE_H__

