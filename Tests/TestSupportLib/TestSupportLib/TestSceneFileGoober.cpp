#include "BaseLib/FileUtil.h"
#include "SupportLib/SceneFileReader.h"
#include "TestLib/Assert.h"


#define GOOBER_FILE		"Input" _FS_ "Scene" _FS_ "Goober.DRG"


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
static void AssertGooberName(CSceneFileReader* pcReader, const char* szExpected)
{
	CChars	sz;

	sz.Init();
	AssertTrue(pcReader->ReadName(&sz));
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////
//
// A skinned mesh walking in a field of stars, converted from one of the
// old files.  Each track's first key being its connection's world matrix
// is what shows they're on the right connections.
//
//////////////////////////////////////////////////////////////////////
static void TestSceneFileGooberContents(void)
{
	CSceneFileReader	cReader;
	CChars				szChannels;
	float32				afWorlds[141 * SCENE_FILE_FLOATS_IN_MATRIX];
	float32				afKeys[80 * SCENE_FILE_FLOATS_IN_MATRIX];
	float32				afTimes[80];
	size				i;
	size				j;
	int32				iWidth;
	int32				iHeight;

	cReader.Init(GOOBER_FILE);
	AssertTrue(cReader.Open());

	AssertSize(1, cReader.NumMeshes());
	AssertTrue(cReader.BeginMesh(0));
	AssertGooberName(&cReader, "goober_");
	AssertSize(3241, cReader.NumPositions());
	AssertSize(6134, cReader.NumPolygons());
	AssertSize(6134 * 3, cReader.NumCorners());
	AssertSize(0, cReader.NumNormals());
	AssertSize(6134, cReader.NumSmoothGroups());
	AssertSize(1, cReader.NumUVLayers());
	AssertSize(3241, cReader.NumSkinCounts());
	AssertSize(8124, cReader.NumSkinWeights());
	AssertSize(67, cReader.NumInverseMatrices());
	AssertTrue(cReader.End());

	AssertSize(3, cReader.NumImages());
	szChannels.Init();
	AssertTrue(cReader.BeginImage(0));
	AssertGooberName(&cReader, "Goober_D.tga");
	AssertTrue(cReader.ReadImageSize(&iWidth, &iHeight));
	AssertInt((int32)2048, iWidth);
	AssertInt((int32)2048, iHeight);
	AssertTrue(cReader.ReadChannels(&szChannels));
	AssertString("RGBA", szChannels.Text());
	AssertTrue(cReader.End());
	szChannels.Kill();

	AssertSize(1, cReader.NumMaterials());
	AssertTrue(cReader.BeginMaterial(0));
	AssertInt((int32)0, cReader.GetMaterialTexture(SFTX_Diffuse));
	AssertInt((int32)1, cReader.GetMaterialTexture(SFTX_Bump));
	AssertInt((int32)2, cReader.GetMaterialTexture(SFTX_Reflection));
	AssertTrue(cReader.End());

	AssertSize(0, cReader.NumLights());
	AssertSize(0, cReader.NumCameras());

	AssertSize(141, cReader.NumConnections());
	for (i = 0; i < 141; i++)
	{
		AssertTrue(cReader.BeginConnection(i));
		AssertTrue(cReader.ReadWorldMatrix(&afWorlds[i * SCENE_FILE_FLOATS_IN_MATRIX]));
		AssertTrue(cReader.End());
	}

	AssertSize(1, cReader.NumAnimations());
	AssertTrue(cReader.BeginAnimation(0));
	AssertSize(140, cReader.NumTracks());
	for (i = 0; i < 140; i++)
	{
		AssertTrue(cReader.BeginTrack(i));
		AssertInt((int32)i + 1, cReader.GetAttached());
		AssertSize(80, cReader.NumKeys());
		AssertTrue(cReader.ReadKeyTimes(afTimes));
		AssertTrue(cReader.ReadKeyMatrices(afKeys));
		AssertTrue(cReader.End());

		AssertFloat(0.0f, afTimes[0], 3);
		for (j = 0; j < SCENE_FILE_FLOATS_IN_MATRIX; j++)
		{
			AssertFloat(afWorlds[(i + 1) * SCENE_FILE_FLOATS_IN_MATRIX + j], afKeys[j], 2);
		}
	}
	AssertTrue(cReader.End());

	AssertSize(1, cReader.NumInstances());
	AssertTrue(cReader.BeginInstance(0));
	AssertSize(67, cReader.NumInstanceConnections());
	AssertSize(1, cReader.NumInstanceMeshes());
	AssertTrue(cReader.BeginInstanceMesh(0));
	AssertInt((int32)0, cReader.GetInstanceMesh());
	AssertSize(67, cReader.NumConnectionSlots());
	AssertTrue(cReader.End());
	AssertTrue(cReader.End());

	AssertSize(1, cReader.NumParticleSets());
	AssertTrue(cReader.BeginParticles(0));
	AssertSize(1105, cReader.NumParticles());
	AssertTrue(cReader.End());

	AssertFalse(cReader.Failed());
	cReader.Kill();
}


//////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////
void TestSceneFileGoober(void)
{
	BeginTests();

	TestSceneFileGooberContents();

	TestStatistics();
}

