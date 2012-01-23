/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __GRAPHICS_INSTANCE_H__
#define __GRAPHICS_INSTANCE_H__
#include "GraphicsObject.h"
#include "LightInstance.h"
#include "GraphicsViewport.h"


#define GRAPH_INST_FLAGS_DRAW			0x01
#define GRAPH_INST_FLAGS_REVERSE_CULL	0x02


struct CGraphPrimInstance
{
	int				iFlags;
};


typedef CArrayTemplate<CGraphPrimInstance> CGraphPrimInstanceArray;

class CWorld;
class CGraphicsInstance : public CUnknown
{
BASE_FUNCTIONS(CGraphicsInstance);
protected:
	CGraphicsObject*			mpcGraphicsObject;		//Pointer to the graphics object.
	CArrayPtrMatrices			mapMatrices;	 		//Pointers to the controlling matrices.
	CArrayPtrMaterials			mapMaterials;    		//Pointers to the materials
	CArrayPtrStates				mapStates;		 		//Pointers to the states.
	int							miFlags;
	CArrayPtrLight				mapLights;
	CWorld*						mpcWorld;

	//Should still add lights...
	//Possibly also some sort of environment modifiers...
public:
	void 				Init(void);
	void 				Init(CWorld* pcWorld);
	void 				Kill(void);
	void 				Draw(void);
	void 				Set(CGraphicsObject* pcGraphicsObject);
	void 				AddMatrixPtr(SMatrix* psMatrix);
	SMatrix*			GetMatrix(int iNum);
	void				ReverseCullAsNecessary(void);
	void				UnreverseCullAsNecessary(void);
	BOOL				CanDraw(void);
	void				Update(void);

	void				SetFlag(int iFlag, int iFlagValue);
	CGraphicsObject*	GetGraphicsObject(void);

};


typedef CArrayTemplate<CGraphicsInstance*> CGraphicsInstancePtrArray;


#endif //__GRAPHICS_INSTANCE_H__

