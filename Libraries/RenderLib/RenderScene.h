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
#include "CookedMesh.h"
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
	Ptr<CCookedMesh>	mpCooked;  //The editor's output cooked elsewhere (CRenderScene::SetCooked), until a render mesh is made of it.
	size				muiCookedStructure;  //What the editor's counters were when it was given.
	size				muiCookedShape;
	size				muiCookedModifiers;

	void	Init(Ptr<CMeshEditor> pEditor);
	void	Class(void) override;
	void	Free(void) override;

	bool				IsOutOfDate(void);
	void				Made(Ptr<CRenderMesh> pMesh);
	void				SetCooked(Ptr<CCookedMesh> pCooked);
	Ptr<CCookedMesh>	TakeCooked(void);  //NULL if none was given, or the editor has changed since it was.
};


//What a renderer has made for a scene's meshes.  Update, at a frame boundary, makes a render mesh for the output of each editor
//the scene's instances use, one for an editor that many share, and makes it again when the editor's counters say its output has
//changed.  Nothing has to tell it, as it looks.  A mesh is never changed in place: a new one replaces it.  A mesh the scene no
//longer uses is let go, and the graph frees it.  The editor's output is let go once its render mesh is made.
//
//Cooking an output, and making a render mesh of it, are done in Update, on the thread that draws: a few milliseconds
//to a few tenths of a second for a large mesh.  Whoever made the output on another thread can do them there too, and
//give what comes of it for the editor as it is then: its output cooked (SetCooked; the render mesh is still made here,
//which for a renderer on a graphics card is the part that must be), or the render mesh itself (SetMesh, made by this
//scene's factory for its renderer).  Either is for an editor one of the scene's instances uses by the next update;
//what's given for another is let go then, as any mesh the scene doesn't use is.
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
	void				SetCooked(Ptr<CMeshEditor> pEditor, Ptr<CCookedMesh> pCooked);
	void				SetMesh(Ptr<CMeshEditor> pEditor, Ptr<CRenderMesh> pMesh);
	CRenderFactory*		GetFactory(void) const;

	Ptr<CRenderMesh>	GetMesh(Ptr<CMeshEditor> pEditor);  //NULL if the scene's instances didn't use it at the last update.
	size				NumMeshes(void);
	Ptr<CScene>			GetScene(void) const;
	Ptr<CRenderer>		GetRenderer(void) const;

protected:
	void					UpdateEditor(Ptr<CMeshEditor> pEditor);
	Ptr<CRenderSceneMesh>	FindMesh(Ptr<CMeshEditor> pEditor);
	Ptr<CRenderSceneMesh>	FindOrAddMesh(Ptr<CMeshEditor> pEditor);
	void					RemoveUnused(void);
};


#endif // __RENDER_SCENE_H__

