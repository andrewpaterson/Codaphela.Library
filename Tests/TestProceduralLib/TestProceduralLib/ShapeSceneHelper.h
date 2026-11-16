#ifndef __SHAPE_SCENE_HELPER_H__
#define __SHAPE_SCENE_HELPER_H__
#include "SupportLib/Camera.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/Scene.h"
#include "SupportLib/Shape.h"
#include "SupportLib/SolidTexture.h"


//Scenes of shapes drawn by the raster renderer, for the tests that look at what was drawn.


void			ShapeSceneInstance(Ptr<CScene> pScene, Ptr<CMeshEditor> pEditor);
void			ShapeSceneAddTextured(Ptr<CScene> pScene, Ptr<CShape> pShape, Ptr<CSolidTexture> pTexture, float32 fTexelsPerUnit);
Ptr<CCamera>	ShapeSceneCamera(SFloat32Vec3* psEye);
size			ShapeSceneDraw(Ptr<CScene> pScene, SFloat32Vec3* psEye, uint32* puiPixels, int iWidth, int iHeight);


#endif // __SHAPE_SCENE_HELPER_H__

