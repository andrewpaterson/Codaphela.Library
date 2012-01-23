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
#ifndef __INSTANCE_CONVERTER_H__
#define __INSTANCE_CONVERTER_H__
#include "SupportLib/Instance.h"
#include "SupportLib/Scene.h"
#include "GraphicsInstance.h"
#include "CameraInstance.h"
#include "LightInstance.h"
#include "MeshInstance.h"


#define CONVERSION_LIGHT_RANGE 3000

class CSceneConverter;
class CInstanceConverter
{
protected:
	CSceneConverter* mpcSceneConverter;
	int			miConnectionID;

public:
	void Init(CSceneConverter* pcSceneConverter);
	void Kill(void);

	BOOL Convert(CInstance* pcInstance);
	BOOL ConvertMeshAndLinks(CGraphicsInstance** ppcGraphicsInstance, CMeshInstance** ppcMeshInstance, int iMeshIndex, CArraySimpleInt* pcConnectionIndices);
	BOOL ConvertCamera(CCameraInstance** ppcCameraInstance, int iCameraIndex, CArraySimpleInt* pcConnectionIndices);
	BOOL ConvertLight(CLightInstance** ppcLightInstance, int iLightIndex, CArraySimpleInt* pcConnectionIndices);
	BOOL ConvertLinkInstance(CMeshInstance* pcMeshInstance, CMeshObject* pcMeshObject, CGraphicsInstance* pcGraphicsInstance, CArrayPointer* pcConnectionsAndIndices);
	BOOL ConvertSequence(int iConnectionIndex, SMatrix* psMatrix);
	void ConvertConnectionsAndIndices(CArrayPointer* pcConnectionAndIndex, CArraySimpleInt* pcConnectionIndices);
};


#endif // __INSTANCE_CONVERTER_H__