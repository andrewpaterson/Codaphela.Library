/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RenderLib

Codaphela RenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __RENDER_SCENE_H__
#define __RENDER_SCENE_H__
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/Scene.h"
#include "RenderFactory.h"


//The render mesh made for one editor's output, and what the editor's counters were when it was made.
class CRenderSceneMesh : public CObject
{
CONSTRUCTABLE(CRenderSceneMesh);
DESTRUCTABLE(CRenderSceneMesh);
public:
	Ptr<CMeshEditor>	mpEditor;
	Ptr<CRenderMesh>	mpMesh;
	size				muiStructureChanges;
	size				muiShapeChanges;
	size				muiModifierChanges;
	bool				mbUsed;  //By an instance, in this update.

	void	Init(Ptr<CMeshEditor> pEditor);
	void	Class(void) override;
	void	Free(void) override;

	bool	IsOutOfDate(void);
	void	Made(Ptr<CRenderMesh> pMesh);
};


//What a renderer has made for a scene's meshes.  Update, at a frame boundary, makes a render mesh for the output of each editor
//the scene's instances use, one for an editor that many share, and makes it again when the editor's counters say its output has
//changed.  Nothing has to tell it, as it looks.  A mesh is never changed in place: a new one replaces it.  A mesh the scene no
//longer uses is let go, and the graph frees it.  The editor's output is let go once its render mesh is made.
//
//The factory isn't an object in the graph, so the app keeps it for as long as this.
class CRenderScene : public CObject
{
CONSTRUCTABLE(CRenderScene);
DESTRUCTABLE(CRenderScene);
protected:
	Ptr<CScene>					mpScene;
	Ptr<CRenderer>				mpRenderer;
	CRenderFactory*				mpcFactory;
	CArray<CRenderSceneMesh>	mapMeshes;

public:
	void				Init(Ptr<CScene> pScene, CRenderFactory* pcFactory, Ptr<CRenderer> pRenderer);
	void				Class(void) override;
	void				Free(void) override;

	void				Update(void);

	Ptr<CRenderMesh>	GetMesh(Ptr<CMeshEditor> pEditor);  //NULL if the scene's instances didn't use it at the last update.
	size				NumMeshes(void);
	Ptr<CScene>			GetScene(void);
	Ptr<CRenderer>		GetRenderer(void);

protected:
	void					UpdateEditor(Ptr<CMeshEditor> pEditor);
	Ptr<CRenderSceneMesh>	FindMesh(Ptr<CMeshEditor> pEditor);
	void					RemoveUnused(void);
};


#endif // __RENDER_SCENE_H__

