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
// Its bones are a skeleton under the first, each after its parent, which
// is the same bone as its connection's parent.  The connection playing bone
// i comes straight after the first, goober_, which nothing uses.
//
//////////////////////////////////////////////////////////////////////
static void TestSceneFileGooberSkeleton(CSceneFileReader* pcReader)
{
	CChars	sz;
	size	i;
	int32	iParent;
	int32	iSkeleton;
	int32	iBone;
	int32	aiParents[67];

	AssertSize(1, pcReader->NumSkeletons());
	AssertTrue(pcReader->BeginSkeleton(0));
	AssertSize(67, pcReader->NumBones());
	for (i = 0; i < 67; i++)
	{
		AssertTrue(pcReader->BeginBone(i));
		sz.Init();
		AssertTrue(pcReader->ReadName(&sz));
		AssertTrue(sz.StartsWith("bone") || sz.StartsWith("Bone"));
		sz.Kill();
		iParent = pcReader->GetParent();
		aiParents[i] = iParent;
		if (i == 0)
		{
			AssertInt((int32)SCENE_FILE_NONE, iParent);
		}
		else
		{
			AssertTrue((iParent >= 0) && (iParent < (int32)i));
		}
		AssertTrue(pcReader->End());
	}
	AssertTrue(pcReader->End());

	AssertSize(68, pcReader->NumConnections());
	AssertTrue(pcReader->BeginConnection(0));
	AssertGooberName(pcReader, "goober_");
	AssertFalse(pcReader->GetSkeletonBone(&iSkeleton, &iBone));
	AssertTrue(pcReader->End());
	for (i = 1; i < 68; i++)
	{
		AssertTrue(pcReader->BeginConnection(i));
		AssertTrue(pcReader->GetSkeletonBone(&iSkeleton, &iBone));
		AssertInt((int32)0, iSkeleton);
		AssertInt((int32)i - 1, iBone);
		iParent = pcReader->GetParent();
		AssertInt((iBone == 0) ? (int32)SCENE_FILE_NONE : aiParents[iBone] + 1, iParent);
		AssertTrue(pcReader->End());
	}
}


//////////////////////////////////////////////////////////////////////
//
// A skinned mesh walking in a field of stars, converted from one of the
// old files.  Each track's first key being its bone's connection's local
// matrix is what shows they're on the right bones, and that the keys are
// relative to their parents.  Its IK chains and finger helpers aren't
// bones, and their effect is in the bones' keys, so they're left out.
//
//////////////////////////////////////////////////////////////////////
static void TestSceneFileGooberContents(void)
{
	CSceneFileReader	cReader;
	CChars				szChannels;
	float32				afLocals[68 * SCENE_FILE_FLOATS_IN_MATRIX];
	float32				afKeys[80 * SCENE_FILE_FLOATS_IN_MATRIX];
	float32				afTimes[80];
	int32				aiSkinBones[67];
	bool				abUsed[67];
	size				i;
	size				j;
	int32				iWidth;
	int32				iHeight;

	cReader.Init(GOOBER_FILE);
	AssertTrue(cReader.Open());
	TestSceneFileGooberSkeleton(&cReader);

	AssertSize(1, cReader.NumMeshes());
	AssertTrue(cReader.BeginMesh(0));
	AssertGooberName(&cReader, "goober_");
	AssertLSize(3241, cReader.NumPositions());
	AssertLSize(6134, cReader.NumPolygons());
	AssertLSize(6134 * 3, cReader.NumCorners());
	AssertLSize(0, cReader.NumNormals());
	AssertLSize(6134, cReader.NumSmoothGroups());
	AssertSize(1, cReader.NumUVLayers());
	AssertLSize(3241, cReader.NumSkinCounts());
	AssertLSize(8124, cReader.NumSkinWeights());
	AssertLSize(67, cReader.NumInverseMatrices());
	AssertInt((int32)0, cReader.GetSkinSkeleton());
	AssertLSize(67, cReader.NumSkinBones());
	AssertTrue(cReader.ReadSkinBones(aiSkinBones));
	memset(abUsed, 0, sizeof(abUsed));
	for (i = 0; i < 67; i++)
	{
		AssertTrue((aiSkinBones[i] >= 0) && (aiSkinBones[i] < 67) && !abUsed[aiSkinBones[i]]);
		abUsed[aiSkinBones[i]] = true;
	}
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

	for (i = 0; i < 68; i++)
	{
		AssertTrue(cReader.BeginConnection(i));
		AssertTrue(cReader.ReadLocalMatrix(&afLocals[i * SCENE_FILE_FLOATS_IN_MATRIX]));
		AssertTrue(cReader.End());
	}

	AssertSize(1, cReader.NumAnimations());
	AssertTrue(cReader.BeginAnimation(0));
	AssertInt((int32)0, cReader.GetAnimationSkeleton());
	AssertSize(67, cReader.NumTracks());
	for (i = 0; i < 67; i++)
	{
		AssertTrue(cReader.BeginTrack(i));
		AssertInt((int32)i, cReader.GetBone());
		AssertLSize(80, cReader.NumKeys());
		AssertTrue(cReader.ReadKeyTimes(afTimes));
		AssertTrue(cReader.ReadKeyMatrices(afKeys));
		AssertTrue(cReader.End());

		AssertFloat(0.0f, afTimes[0], 3);
		for (j = 0; j < SCENE_FILE_FLOATS_IN_MATRIX; j++)
		{
			AssertFloat(afLocals[(i + 1) * SCENE_FILE_FLOATS_IN_MATRIX + j], afKeys[j], 2);
		}
	}
	AssertTrue(cReader.End());

	AssertSize(1, cReader.NumInstances());
	AssertTrue(cReader.BeginInstance(0));
	AssertInt((int32)0, cReader.GetInstanceSkeleton());
	AssertLSize(67, cReader.NumInstanceConnections());
	AssertSize(1, cReader.NumInstanceMeshes());
	AssertTrue(cReader.BeginInstanceMesh(0));
	AssertInt((int32)0, cReader.GetInstanceMesh());
	AssertLSize(0, cReader.NumConnectionSlots());
	AssertTrue(cReader.End());
	AssertTrue(cReader.End());

	AssertSize(1, cReader.NumParticleSets());
	AssertTrue(cReader.BeginParticles(0));
	AssertLSize(1105, cReader.NumParticles());
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

