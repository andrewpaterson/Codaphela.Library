#include "BaseLib/ChunkFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "SupportLib/SceneFileReader.h"
#include "SupportLib/SceneFileWriter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


#define SAMPLE_FILE		"Input" _FS_ "Scene" _FS_ "Sample.DRG"
#define OUTPUT_DIR		"Output" _FS_ "Scene"
#define OUTPUT_FILE		OUTPUT_DIR _FS_ "Sample.DRG"
#define BAD_FILE		OUTPUT_DIR _FS_ "Bad.DRG"


//A square pyramid: a quad and a triangle are enough to show polygons of different sizes.
static const float32	gafPositions[] = { 0, 0, 0,  1, 0, 0,  1, 1, 0,  0, 1, 0,  0.5f, 0.5f, 1 };
static const float32	gafNormals[] = { 0, 0, -1,  0, -0.5f, 0.5f };
static const uint32		gauiColours[] = { 0xFFFF0000, 0x80123456 };
static const int32		gaiPolygonSizes[] = { 4, 3 };
static const int32		gaiCornerPositions[] = { 3, 2, 1, 0,  0, 1, 4 };
static const int32		gaiCornerNormals[] = { 0, 0, 0, 0,  1, 1, -1 };
static const int32		gaiCornerColours[] = { 0, 0, 0, 0,  1, 1, 1 };
static const float32	gafUVs[] = { 0, 0,  1, 0,  1, 1,  0, 1 };
static const int32		gaiCornerUVs[] = { 0, 1, 2, 3,  0, 1, -1 };
static const float32	gafSecondUVs[] = { 0.25f, 0.75f };
static const int32		gaiSecondCornerUVs[] = { 0, 0, 0, 0,  0, 0, 0 };
static const int32		gaiMaterialSlots[] = { 1, 0 };
static const int32		gaiPolygonSlots[] = { 0, -1 };
static const uint32		gauiSmoothGroups[] = { 1, 0x80000002 };
static const int32		gaiSkinCounts[] = { 1, 1, 1, 1, 2 };
static const int32		gaiSkinMatrices[] = { 0, 0, 0, 0, 0, 1 };
static const float32	gafSkinWeights[] = { 1, 1, 1, 1, 0.75f, 0.25f };
static const float32	gafIdentity[] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
static const float32	gafMoved[] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  2, 3, 4, 1 };
static const uint8		gauiPixels[] = { 1, 2, 3,  4, 5, 6,  7, 8, 9,  10, 11, 12 };
static const float32	gafBackwards[] = { 1, 0.5f };
static const float32	gafRed[] = { 1, 0, 0, 0.5f };
static const float32	gafGrey[] = { 0.25f, 0.25f, 0.25f, 1 };
static const float32	gafKeyTimes[] = { 0, 0.5f };
static const int32		gaiInstanceConnections[] = { 1, 0 };
static const int32		gaiConnectionSlots[] = { 0, 1 };
static const int32		gaiInstanceMaterials[] = { 1 };
static const float32	gafParticlePositions[] = { 1, 2, 3,  4, 5, 6 };
static const float32	gafParticleColours[] = { 1, 1, 1,  0, 0.5f, 1 };
static const float32	gafParticleRadii[] = { 0.5f, 2 };
static const float32	gafParticleIntensities[] = { 1, 0.25f };


#define ArrayCount(a)	(sizeof(a) / sizeof((a)[0]))


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertFloats(const float32* pafExpected, const float32* pafActual, size uiNum)
{
	AssertTrue(memcmp(pafExpected, pafActual, uiNum * sizeof(float32)) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertInts(const int32* paiExpected, const int32* paiActual, size uiNum)
{
	AssertTrue(memcmp(paiExpected, paiActual, uiNum * sizeof(int32)) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertName(CSceneFileReader* pcReader, const char* szExpected)
{
	CChars	sz;

	sz.Init();
	AssertTrue(pcReader->ReadName(&sz));
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Uses everything the format has.
//
//////////////////////////////////////////////////////////////////////////
static void WriteSample(const char* szFilename)
{
	CSceneFileWriter	cWriter;
	float32				af4x4s[32];

	cWriter.Init(szFilename);
	AssertTrue(cWriter.Open());

	AssertTrue(cWriter.BeginMesh("Pyramid"));
	AssertTrue(cWriter.WritePositions(gafPositions, ArrayCount(gafPositions) / 3));
	AssertTrue(cWriter.WriteNormals(gafNormals, ArrayCount(gafNormals) / 3));
	AssertTrue(cWriter.WriteColours(gauiColours, ArrayCount(gauiColours)));
	AssertTrue(cWriter.WritePolygons(gaiPolygonSizes, ArrayCount(gaiPolygonSizes), gaiCornerPositions, ArrayCount(gaiCornerPositions)));
	AssertTrue(cWriter.WriteCornerNormals(gaiCornerNormals, ArrayCount(gaiCornerNormals)));
	AssertTrue(cWriter.WriteCornerColours(gaiCornerColours, ArrayCount(gaiCornerColours)));
	AssertTrue(cWriter.WriteUVLayer(gafUVs, ArrayCount(gafUVs) / 2, gaiCornerUVs, ArrayCount(gaiCornerUVs)));
	AssertTrue(cWriter.WriteUVLayer(gafSecondUVs, ArrayCount(gafSecondUVs) / 2, gaiSecondCornerUVs, ArrayCount(gaiSecondCornerUVs)));
	AssertTrue(cWriter.WriteMaterialSlots(gaiMaterialSlots, ArrayCount(gaiMaterialSlots)));
	AssertTrue(cWriter.WritePolygonSlots(gaiPolygonSlots, ArrayCount(gaiPolygonSlots)));
	AssertTrue(cWriter.WriteSmoothGroups(gauiSmoothGroups, ArrayCount(gauiSmoothGroups)));
	AssertTrue(cWriter.WriteSkin(gaiSkinCounts, ArrayCount(gaiSkinCounts), gaiSkinMatrices, gafSkinWeights, ArrayCount(gafSkinWeights)));
	memcpy(af4x4s, gafIdentity, sizeof(gafIdentity));
	memcpy(&af4x4s[16], gafMoved, sizeof(gafMoved));
	AssertTrue(cWriter.WriteInverseMatrices(af4x4s, 2));
	AssertTrue(cWriter.EndMesh());

	//Everything can be left out.
	AssertTrue(cWriter.BeginMesh("Empty"));
	AssertTrue(cWriter.EndMesh());

	AssertTrue(cWriter.BeginImage("Checks"));
	AssertTrue(cWriter.WritePixels(2, 2, "RGB", gauiPixels));
	AssertTrue(cWriter.EndImage());

	AssertTrue(cWriter.BeginMaterial("Red"));
	AssertTrue(cWriter.WriteMaterialColour(SFMC_Diffuse, gafRed));
	AssertTrue(cWriter.WriteMaterialColour(SFMC_Specular, gafGrey));
	AssertTrue(cWriter.WriteShininess(1.5f, 0.25f));
	AssertTrue(cWriter.WriteMaterialTexture(SFTX_Bump, 0));
	AssertTrue(cWriter.EndMaterial());

	AssertTrue(cWriter.BeginMaterial("Bare"));
	AssertTrue(cWriter.EndMaterial());

	AssertTrue(cWriter.BeginConnection("Root"));
	AssertTrue(cWriter.WriteWorldMatrix(gafIdentity));
	AssertTrue(cWriter.EndConnection());

	AssertTrue(cWriter.BeginConnection("Child"));
	AssertTrue(cWriter.WriteParent(0));
	AssertTrue(cWriter.WriteWorldMatrix(gafMoved));
	AssertTrue(cWriter.WriteLocalMatrix(gafMoved));
	AssertTrue(cWriter.WriteParity(true));
	AssertTrue(cWriter.EndConnection());

	AssertTrue(cWriter.BeginLight("Lamp"));
	AssertTrue(cWriter.WriteLightType(SFLT_Spot));
	AssertTrue(cWriter.WriteLightColour(gafGrey));
	AssertTrue(cWriter.WriteLightValues(1, 10, 0.5f));
	AssertTrue(cWriter.WriteLightDecay(SFLD_Square));
	AssertTrue(cWriter.WriteShadows(true));
	AssertTrue(cWriter.WriteAttached(1));
	AssertTrue(cWriter.EndLight());

	AssertTrue(cWriter.BeginCamera("Eye"));
	AssertTrue(cWriter.WriteCameraType(SFCT_Orthographic));
	AssertTrue(cWriter.WriteLens(1, 0.25f, 100));
	AssertTrue(cWriter.WriteAttached(0));
	AssertTrue(cWriter.EndCamera());

	AssertTrue(cWriter.BeginAnimation("Move"));
	AssertTrue(cWriter.WriteTrack(0, gafKeyTimes, af4x4s, 2));
	AssertTrue(cWriter.WriteTrack(1, gafKeyTimes, gafMoved, 1));
	AssertTrue(cWriter.EndAnimation());

	AssertTrue(cWriter.BeginInstance("Thing"));
	AssertTrue(cWriter.WriteConnections(gaiInstanceConnections, ArrayCount(gaiInstanceConnections)));
	AssertTrue(cWriter.WriteInstanceMesh(0, gaiConnectionSlots, ArrayCount(gaiConnectionSlots), gaiInstanceMaterials, ArrayCount(gaiInstanceMaterials)));
	AssertTrue(cWriter.WriteInstanceMesh(1, gaiConnectionSlots, 1, NULL, 0));
	AssertTrue(cWriter.EndInstance());

	AssertTrue(cWriter.WriteParticles("Stars", gafParticlePositions, gafParticleColours, gafParticleRadii, gafParticleIntensities, 2));
	AssertTrue(cWriter.WriteParticles("Dust", gafParticlePositions, NULL, NULL, NULL, 2));

	AssertTrue(cWriter.Close());
	cWriter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSampleMesh(CSceneFileReader* pcReader)
{
	float32	af[64];
	int32	ai[16];
	uint32	aui[8];
	float32	af4x4s[32];

	AssertName(pcReader, "Pyramid");

	AssertSize(5, pcReader->NumPositions());
	AssertTrue(pcReader->ReadPositions(af));
	AssertFloats(gafPositions, af, ArrayCount(gafPositions));

	AssertSize(2, pcReader->NumNormals());
	AssertTrue(pcReader->ReadNormals(af));
	AssertFloats(gafNormals, af, ArrayCount(gafNormals));

	AssertSize(2, pcReader->NumColours());
	AssertTrue(pcReader->ReadColours(aui));
	AssertTrue(memcmp(gauiColours, aui, sizeof(gauiColours)) == 0);

	AssertSize(2, pcReader->NumPolygons());
	AssertTrue(pcReader->ReadPolygonSizes(ai));
	AssertInts(gaiPolygonSizes, ai, 2);

	AssertSize(7, pcReader->NumCorners());
	AssertTrue(pcReader->ReadCornerPositions(ai));
	AssertInts(gaiCornerPositions, ai, 7);

	AssertSize(7, pcReader->NumCornerNormals());
	AssertTrue(pcReader->ReadCornerNormals(ai));
	AssertInts(gaiCornerNormals, ai, 7);

	AssertSize(7, pcReader->NumCornerColours());
	AssertTrue(pcReader->ReadCornerColours(ai));
	AssertInts(gaiCornerColours, ai, 7);

	AssertSize(2, pcReader->NumUVLayers());
	AssertTrue(pcReader->BeginUVLayer(0));
	AssertSize(4, pcReader->NumUVs());
	AssertTrue(pcReader->ReadUVs(af));
	AssertFloats(gafUVs, af, ArrayCount(gafUVs));
	AssertSize(7, pcReader->NumCornerUVs());
	AssertTrue(pcReader->ReadCornerUVs(ai));
	AssertInts(gaiCornerUVs, ai, 7);
	AssertTrue(pcReader->End());
	AssertTrue(pcReader->BeginUVLayer(1));
	AssertSize(1, pcReader->NumUVs());
	AssertTrue(pcReader->ReadUVs(af));
	AssertFloats(gafSecondUVs, af, 2);
	AssertTrue(pcReader->End());
	AssertFalse(pcReader->BeginUVLayer(2));

	AssertSize(2, pcReader->NumMaterialSlots());
	AssertTrue(pcReader->ReadMaterialSlots(ai));
	AssertInts(gaiMaterialSlots, ai, 2);

	AssertSize(2, pcReader->NumPolygonSlots());
	AssertTrue(pcReader->ReadPolygonSlots(ai));
	AssertInts(gaiPolygonSlots, ai, 2);

	AssertSize(2, pcReader->NumSmoothGroups());
	AssertTrue(pcReader->ReadSmoothGroups(aui));
	AssertTrue(memcmp(gauiSmoothGroups, aui, sizeof(gauiSmoothGroups)) == 0);

	AssertSize(5, pcReader->NumSkinCounts());
	AssertTrue(pcReader->ReadSkinCounts(ai));
	AssertInts(gaiSkinCounts, ai, 5);
	AssertSize(6, pcReader->NumSkinWeights());
	AssertTrue(pcReader->ReadSkinMatrices(ai));
	AssertInts(gaiSkinMatrices, ai, 6);
	AssertTrue(pcReader->ReadSkinWeights(af));
	AssertFloats(gafSkinWeights, af, 6);

	AssertSize(2, pcReader->NumInverseMatrices());
	AssertTrue(pcReader->ReadInverseMatrices(af4x4s));
	AssertFloats(gafIdentity, af4x4s, 16);
	AssertFloats(gafMoved, &af4x4s[16], 16);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSampleEmptyMesh(CSceneFileReader* pcReader)
{
	float32	af[16];

	AssertName(pcReader, "Empty");
	AssertSize(0, pcReader->NumPositions());
	AssertSize(0, pcReader->NumPolygons());
	AssertSize(0, pcReader->NumCorners());
	AssertSize(0, pcReader->NumUVLayers());
	AssertSize(0, pcReader->NumSkinWeights());
	AssertFalse(pcReader->ReadPositions(af));
	AssertFalse(pcReader->Failed());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSampleImage(CSceneFileReader* pcReader)
{
	CChars	szChannels;
	int32	iWidth;
	int32	iHeight;
	uint8	auiPixels[12];

	AssertName(pcReader, "Checks");
	AssertTrue(pcReader->ReadImageSize(&iWidth, &iHeight));
	AssertInt((int32)2, iWidth);
	AssertInt((int32)2, iHeight);
	szChannels.Init();
	AssertTrue(pcReader->ReadChannels(&szChannels));
	AssertString("RGB", szChannels.Text());
	szChannels.Kill();
	AssertSize(12, pcReader->NumPixelBytes());
	AssertTrue(pcReader->ReadPixels(auiPixels));
	AssertTrue(memcmp(gauiPixels, auiPixels, 12) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSampleMaterials(CSceneFileReader* pcReader)
{
	float32	afRGBA[4];
	float32	fPower;
	float32	fBase;

	AssertSize(2, pcReader->NumMaterials());

	AssertTrue(pcReader->BeginMaterial(0));
	AssertName(pcReader, "Red");
	AssertTrue(pcReader->ReadMaterialColour(SFMC_Diffuse, afRGBA));
	AssertFloats(gafRed, afRGBA, 4);
	AssertFalse(pcReader->ReadMaterialColour(SFMC_Ambient, afRGBA));
	AssertTrue(pcReader->ReadMaterialColour(SFMC_Specular, afRGBA));
	AssertFloats(gafGrey, afRGBA, 4);
	AssertTrue(pcReader->ReadShininess(&fPower, &fBase));
	AssertFloat(1.5f, fPower, 3);
	AssertFloat(0.25f, fBase, 3);
	AssertInt((int32)0, pcReader->GetMaterialTexture(SFTX_Bump));
	AssertInt((int32)SCENE_FILE_NONE, pcReader->GetMaterialTexture(SFTX_Diffuse));
	AssertTrue(pcReader->End());

	AssertTrue(pcReader->BeginMaterial(1));
	AssertName(pcReader, "Bare");
	AssertFalse(pcReader->ReadShininess(&fPower, &fBase));
	AssertTrue(pcReader->End());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSampleConnections(CSceneFileReader* pcReader)
{
	float32	af4x4[16];

	AssertSize(2, pcReader->NumConnections());

	AssertTrue(pcReader->BeginConnection(0));
	AssertName(pcReader, "Root");
	AssertInt((int32)SCENE_FILE_NONE, pcReader->GetParent());
	AssertTrue(pcReader->ReadWorldMatrix(af4x4));
	AssertFloats(gafIdentity, af4x4, 16);
	AssertFalse(pcReader->ReadLocalMatrix(af4x4));
	AssertFalse(pcReader->GetParity());
	AssertTrue(pcReader->End());

	AssertTrue(pcReader->BeginConnection(1));
	AssertName(pcReader, "Child");
	AssertInt((int32)0, pcReader->GetParent());
	AssertTrue(pcReader->ReadWorldMatrix(af4x4));
	AssertFloats(gafMoved, af4x4, 16);
	AssertTrue(pcReader->ReadLocalMatrix(af4x4));
	AssertFloats(gafMoved, af4x4, 16);
	AssertTrue(pcReader->GetParity());
	AssertTrue(pcReader->End());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSampleLightAndCamera(CSceneFileReader* pcReader)
{
	float32	afRGBA[4];
	float32	f1;
	float32	f2;
	float32	f3;

	AssertSize(1, pcReader->NumLights());
	AssertTrue(pcReader->BeginLight(0));
	AssertName(pcReader, "Lamp");
	AssertInt(SFLT_Spot, pcReader->GetLightType());
	AssertTrue(pcReader->ReadLightColour(afRGBA));
	AssertFloats(gafGrey, afRGBA, 4);
	AssertTrue(pcReader->ReadLightValues(&f1, &f2, &f3));
	AssertFloat(1, f1, 3);
	AssertFloat(10, f2, 3);
	AssertFloat(0.5f, f3, 3);
	AssertInt(SFLD_Square, pcReader->GetLightDecay());
	AssertTrue(pcReader->GetShadows());
	AssertInt((int32)1, pcReader->GetAttached());
	AssertTrue(pcReader->End());

	AssertSize(1, pcReader->NumCameras());
	AssertTrue(pcReader->BeginCamera(0));
	AssertName(pcReader, "Eye");
	AssertInt(SFCT_Orthographic, pcReader->GetCameraType());
	AssertTrue(pcReader->ReadLens(&f1, &f2, &f3));
	AssertFloat(1, f1, 3);
	AssertFloat(0.25f, f2, 3);
	AssertFloat(100, f3, 3);
	AssertInt((int32)0, pcReader->GetAttached());
	AssertTrue(pcReader->End());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSampleAnimation(CSceneFileReader* pcReader)
{
	float32	afTimes[2];
	float32	af4x4s[32];

	AssertSize(1, pcReader->NumAnimations());
	AssertTrue(pcReader->BeginAnimation(0));
	AssertName(pcReader, "Move");
	AssertSize(2, pcReader->NumTracks());

	AssertTrue(pcReader->BeginTrack(0));
	AssertInt((int32)0, pcReader->GetAttached());
	AssertSize(2, pcReader->NumKeys());
	AssertTrue(pcReader->ReadKeyTimes(afTimes));
	AssertFloats(gafKeyTimes, afTimes, 2);
	AssertTrue(pcReader->ReadKeyMatrices(af4x4s));
	AssertFloats(gafIdentity, af4x4s, 16);
	AssertFloats(gafMoved, &af4x4s[16], 16);
	AssertTrue(pcReader->End());

	AssertTrue(pcReader->BeginTrack(1));
	AssertInt((int32)1, pcReader->GetAttached());
	AssertSize(1, pcReader->NumKeys());
	AssertTrue(pcReader->End());

	AssertTrue(pcReader->End());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSampleInstance(CSceneFileReader* pcReader)
{
	int32	ai[4];

	AssertSize(1, pcReader->NumInstances());
	AssertTrue(pcReader->BeginInstance(0));
	AssertName(pcReader, "Thing");
	AssertSize(2, pcReader->NumInstanceConnections());
	AssertTrue(pcReader->ReadInstanceConnections(ai));
	AssertInts(gaiInstanceConnections, ai, 2);
	AssertSize(2, pcReader->NumInstanceMeshes());

	AssertTrue(pcReader->BeginInstanceMesh(0));
	AssertInt((int32)0, pcReader->GetInstanceMesh());
	AssertSize(2, pcReader->NumConnectionSlots());
	AssertTrue(pcReader->ReadConnectionSlots(ai));
	AssertInts(gaiConnectionSlots, ai, 2);
	AssertSize(1, pcReader->NumInstanceMaterials());
	AssertTrue(pcReader->ReadInstanceMaterials(ai));
	AssertInt((int32)1, ai[0]);
	AssertTrue(pcReader->End());

	AssertTrue(pcReader->BeginInstanceMesh(1));
	AssertInt((int32)1, pcReader->GetInstanceMesh());
	AssertSize(1, pcReader->NumConnectionSlots());
	AssertSize(0, pcReader->NumInstanceMaterials());
	AssertTrue(pcReader->End());

	AssertTrue(pcReader->End());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSampleParticles(CSceneFileReader* pcReader)
{
	float32	af[6];

	AssertSize(2, pcReader->NumParticleSets());

	AssertTrue(pcReader->BeginParticles(0));
	AssertName(pcReader, "Stars");
	AssertSize(2, pcReader->NumParticles());
	AssertTrue(pcReader->ReadParticlePositions(af));
	AssertFloats(gafParticlePositions, af, 6);
	AssertTrue(pcReader->ReadParticleColours(af));
	AssertFloats(gafParticleColours, af, 6);
	AssertTrue(pcReader->ReadParticleRadii(af));
	AssertFloats(gafParticleRadii, af, 2);
	AssertTrue(pcReader->ReadParticleIntensities(af));
	AssertFloats(gafParticleIntensities, af, 2);
	AssertTrue(pcReader->End());

	AssertTrue(pcReader->BeginParticles(1));
	AssertName(pcReader, "Dust");
	AssertSize(2, pcReader->NumParticles());
	AssertFalse(pcReader->ReadParticleColours(af));
	AssertTrue(pcReader->End());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSample(const char* szFilename)
{
	CSceneFileReader	cReader;

	cReader.Init(szFilename);
	AssertTrue(cReader.Open());

	AssertSize(2, cReader.NumMeshes());
	AssertTrue(cReader.BeginMesh(0));
	AssertSampleMesh(&cReader);
	AssertTrue(cReader.End());
	AssertTrue(cReader.BeginMesh(1));
	AssertSampleEmptyMesh(&cReader);
	AssertTrue(cReader.End());
	AssertFalse(cReader.BeginMesh(2));

	AssertSize(1, cReader.NumImages());
	AssertTrue(cReader.BeginImage(0));
	AssertSampleImage(&cReader);
	AssertTrue(cReader.End());

	AssertSampleMaterials(&cReader);
	AssertSampleConnections(&cReader);
	AssertSampleLightAndCamera(&cReader);
	AssertSampleAnimation(&cReader);
	AssertSampleInstance(&cReader);
	AssertSampleParticles(&cReader);

	AssertFalse(cReader.Failed());
	AssertTrue(cReader.Close());
	cReader.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The sample in Input was written once and is never rewritten by a test.
// Writing the same scene has to give the same bytes, and it has to go on
// being readable: this fails if the format, or CChunkFile's layout beneath
// it, changes.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneFileSample(void)
{
	WriteSample(OUTPUT_FILE);
	AssertFile(SAMPLE_FILE, OUTPUT_FILE);
	AssertSample(SAMPLE_FILE);
	AssertSample(OUTPUT_FILE);
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a mesh of one triangle, which each test then gets wrong somehow.
//
//////////////////////////////////////////////////////////////////////////
static void BeginTriangle(CSceneFileWriter* pcWriter)
{
	static const int32	aiSizes[] = { 3 };
	static const int32	aiCorners[] = { 0, 1, 2 };

	AssertTrue(pcWriter->BeginMesh("Triangle"));
	AssertTrue(pcWriter->WritePositions(gafPositions, 3));
	AssertTrue(pcWriter->WritePolygons(aiSizes, 1, aiCorners, 3));
}


//////////////////////////////////////////////////////////////////////////
//
// Whatever went wrong, Close fails and no file is left.
//
//////////////////////////////////////////////////////////////////////////
static void AssertNoFile(CSceneFileWriter* pcWriter)
{
	CFileUtil	cFileUtil;

	AssertFalse(pcWriter->Close());
	pcWriter->Kill();
	AssertFalse(cFileUtil.Exists(BAD_FILE));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSceneFileWriterMeshFailures(void)
{
	CSceneFileWriter	cWriter;
	static const int32	aiBadSizes[] = { 4 };
	static const int32	aiTooSmall[] = { 2 };
	static const int32	aiCorners[] = { 0, 1, 2 };
	static const int32	aiFarCorners[] = { 0, 1, 3 };
	static const int32	aiBelowNone[] = { 0, 1, -2 };
	static const int32	aiNoneCorners[] = { -1, -1, -1 };
	static const int32	aiFarSlot[] = { 0 };
	static const int32	aiCounts[] = { 2, 0, 0 };
	static const int32	aiNoCounts[] = { -1, 0, 0 };
	static const int32	aiMatrices[] = { 0, 0 };
	static const float32	afWeights[] = { 0.5f, 0.5f };

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMesh("Triangle"));
	AssertFalse(cWriter.WritePolygons(aiBadSizes, 1, aiCorners, 3));
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMesh("Triangle"));
	AssertFalse(cWriter.WritePolygons(aiTooSmall, 1, aiCorners, 2));
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMesh("Triangle"));
	AssertTrue(cWriter.WritePositions(gafPositions, 3));
	AssertTrue(cWriter.WritePolygons(gaiPolygonSizes + 1, 1, aiFarCorners, 3));
	AssertFalse(cWriter.EndMesh());
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMesh("Triangle"));
	AssertFalse(cWriter.WritePolygons(gaiPolygonSizes + 1, 1, aiBelowNone, 3));
	AssertNoFile(&cWriter);

	//Corner normals that don't exist, and the wrong number of them.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	BeginTriangle(&cWriter);
	AssertTrue(cWriter.WriteCornerNormals(aiCorners, 3));
	AssertFalse(cWriter.EndMesh());
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	BeginTriangle(&cWriter);
	AssertTrue(cWriter.WriteCornerNormals(aiNoneCorners, 2));
	AssertFalse(cWriter.EndMesh());
	AssertNoFile(&cWriter);

	//A UV layer whose corners refer to UVs that aren't there, or with too few corners.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	BeginTriangle(&cWriter);
	AssertFalse(cWriter.WriteUVLayer(gafUVs, 2, aiCorners, 3));
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	BeginTriangle(&cWriter);
	AssertFalse(cWriter.WriteUVLayer(gafUVs, 4, aiCorners, 2));
	AssertNoFile(&cWriter);

	//A polygon's slot that's not in the mesh's slots.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	BeginTriangle(&cWriter);
	AssertTrue(cWriter.WritePolygonSlots(aiFarSlot, 1));
	AssertFalse(cWriter.EndMesh());
	AssertNoFile(&cWriter);

	//Skin counts that don't add up, or go below zero, or weights with no matrices.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	BeginTriangle(&cWriter);
	AssertFalse(cWriter.WriteSkin(aiCounts, 3, aiMatrices, afWeights, 1));
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	BeginTriangle(&cWriter);
	AssertFalse(cWriter.WriteSkin(aiNoCounts, 3, aiMatrices, afWeights, 0));
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	BeginTriangle(&cWriter);
	AssertTrue(cWriter.WriteSkin(aiCounts, 3, aiMatrices, afWeights, 2));
	AssertFalse(cWriter.EndMesh());
	AssertNoFile(&cWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSceneFileWriterReferenceFailures(void)
{
	CSceneFileWriter	cWriter;
	static const int32	aiMaterial[] = { 0 };
	static const int32	aiSlot[] = { 1 };
	float32				af4x4s[32];

	memset(af4x4s, 0, sizeof(af4x4s));

	//These all look fine until the end, when what they refer to isn't there.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	BeginTriangle(&cWriter);
	AssertTrue(cWriter.WriteMaterialSlots(aiMaterial, 1));
	AssertTrue(cWriter.EndMesh());
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMaterial("Textured"));
	AssertTrue(cWriter.WriteMaterialTexture(SFTX_Diffuse, 0));
	AssertTrue(cWriter.EndMaterial());
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginConnection("Child"));
	AssertTrue(cWriter.WriteParent(1));
	AssertTrue(cWriter.EndConnection());
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginLight("Lamp"));
	AssertTrue(cWriter.WriteAttached(0));
	AssertTrue(cWriter.EndLight());
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginInstance("Thing"));
	AssertTrue(cWriter.WriteInstanceMesh(0, NULL, 0, NULL, 0));
	AssertTrue(cWriter.EndInstance());
	AssertNoFile(&cWriter);

	//A slot that isn't in the instance's own connections.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginInstance("Thing"));
	AssertFalse(cWriter.WriteInstanceMesh(0, aiSlot, 1, NULL, 0));
	AssertNoFile(&cWriter);

	//Times that go backwards.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginAnimation("Move"));
	AssertFalse(cWriter.WriteTrack(0, gafBackwards, af4x4s, 2));
	AssertNoFile(&cWriter);

	//A track, an attachment or a parent on nothing.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginAnimation("Move"));
	AssertFalse(cWriter.WriteTrack(SCENE_FILE_NONE, gafKeyTimes, af4x4s, 2));
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginLight("Light"));
	AssertFalse(cWriter.WriteAttached(SCENE_FILE_NONE));
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginConnection("Bone"));
	AssertFalse(cWriter.WriteParent(SCENE_FILE_NONE));
	AssertNoFile(&cWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSceneFileWriterUseFailures(void)
{
	CSceneFileWriter	cWriter;
	CFileUtil			cFileUtil;

	//In the wrong place.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertFalse(cWriter.WritePositions(gafPositions, 1));
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMesh("Mesh"));
	AssertFalse(cWriter.BeginMesh("Inside a mesh"));
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginImage("Image"));
	AssertFalse(cWriter.EndMaterial());
	AssertNoFile(&cWriter);

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMaterial("Material"));
	AssertFalse(cWriter.WriteAttached(0));
	AssertNoFile(&cWriter);

	//Not ended.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMesh("Mesh"));
	AssertNoFile(&cWriter);

	//No data.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMesh("Mesh"));
	AssertFalse(cWriter.WritePositions(NULL, 1));
	AssertNoFile(&cWriter);

	//Not open, and then nothing helps.
	cWriter.Init(BAD_FILE);
	AssertFalse(cWriter.BeginMesh("Mesh"));
	AssertFalse(cWriter.Close());
	cWriter.Kill();

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertFalse(cWriter.Open());
	AssertNoFile(&cWriter);

	//An exporter that ignores results still finds out at Close.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	cWriter.WritePositions(gafPositions, 1);
	cWriter.BeginMesh("Mesh");
	cWriter.WritePositions(gafPositions, 1);
	cWriter.EndMesh();
	AssertNoFile(&cWriter);

	//Forgetting Close leaves nothing that looks like a finished file.
	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMesh("Mesh"));
	cWriter.Kill();
	AssertFalse(cFileUtil.Exists(BAD_FILE));

	//Somewhere it can't write.
	cWriter.Init(OUTPUT_DIR _FS_ "NoSuchDirectory" _FS_ "Bad.DRG");
	AssertFalse(cWriter.Open());
	AssertFalse(cWriter.BeginMesh("Mesh"));
	AssertFalse(cWriter.Close());
	cWriter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Things can refer to what's written after them.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneFileWriterForwardReferences(void)
{
	CSceneFileWriter	cWriter;
	CFileUtil			cFileUtil;

	cWriter.Init(BAD_FILE);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginConnection("Child"));
	AssertTrue(cWriter.WriteParent(1));
	AssertTrue(cWriter.EndConnection());
	AssertTrue(cWriter.BeginConnection("Parent"));
	AssertTrue(cWriter.EndConnection());
	AssertTrue(cWriter.Close());
	cWriter.Kill();
	AssertTrue(cFileUtil.Exists(BAD_FILE));
	AssertTrue(cFileUtil.Delete(BAD_FILE));
}


//////////////////////////////////////////////////////////////////////////
//
// A chunk file made by hand, for what the writer won't make.
//
//////////////////////////////////////////////////////////////////////////
static void BeginHandMadeFile(CChunkFile* pcFile, chunkName iUserID)
{
	pcFile->Init(DiskFile(BAD_FILE));
	AssertTrue(pcFile->WriteOpen(iUserID));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteHandMadeChunk(CChunkFile* pcFile, const char* szName, const void* pvData, size uiSize)
{
	AssertTrue(pcFile->WriteChunkBegin());
	AssertTrue(pcFile->WriteData(pvData, uiSize));
	AssertTrue(pcFile->WriteChunkEnd(szName));
}


//////////////////////////////////////////////////////////////////////////
//
// A later file with things this doesn't know of still reads, and those are
// ignored.  An earlier one without things this knows of reads as having none.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneFileReaderUnknownAndMissing(void)
{
	CChunkFile			cFile;
	CSceneFileReader	cReader;
	int32				iVersion;
	float32				afPositions[3];

	iVersion = SCENE_FILE_VERSION;
	BeginHandMadeFile(&cFile, SCENE_FILE_ID);
	WriteHandMadeChunk(&cFile, SCENE_CHUNK_VERSION, &iVersion, sizeof(int32));
	WriteHandMadeChunk(&cFile, "SomethingNew", &iVersion, sizeof(int32));
	AssertTrue(cFile.WriteChunkBegin());
	WriteHandMadeChunk(&cFile, "AlsoNew", &iVersion, sizeof(int32));
	WriteHandMadeChunk(&cFile, SCENE_CHUNK_POSITIONS, gafPositions, sizeof(float32) * 3);
	AssertTrue(cFile.WriteChunkEnd(SCENE_CHUNK_MESH));
	AssertTrue(cFile.WriteClose());
	cFile.Kill();

	cReader.Init(BAD_FILE);
	AssertTrue(cReader.Open());
	AssertSize(1, cReader.NumMeshes());
	AssertSize(0, cReader.NumImages());
	AssertTrue(cReader.BeginMesh(0));
	AssertSize(1, cReader.NumPositions());
	AssertTrue(cReader.ReadPositions(afPositions));
	AssertFloats(gafPositions, afPositions, 3);
	AssertSize(0, cReader.NumNormals());
	AssertTrue(cReader.End());
	AssertFalse(cReader.Failed());
	AssertTrue(cReader.Close());
	cReader.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertWontOpen(void)
{
	CSceneFileReader	cReader;

	cReader.Init(BAD_FILE);
	AssertFalse(cReader.Open());
	AssertTrue(cReader.Failed());
	cReader.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSceneFileReaderFailures(void)
{
	CChunkFile			cFile;
	CFileUtil			cFileUtil;
	CSceneFileReader	cReader;
	CFileBasic			cBasic;
	int32				iVersion;
	float32				af[8];

	//No file.
	cFileUtil.Delete(BAD_FILE);
	AssertWontOpen();

	//Not a chunk file at all.
	cBasic.Init(DiskFile(BAD_FILE));
	AssertTrue(cBasic.Open(EFM_Write_Replace));
	AssertTrue(cBasic.WriteData("This is not a chunk file, not even a little bit of one, not at all, no.", 71));
	AssertTrue(cBasic.Close());
	cBasic.Kill();
	AssertWontOpen();

	//A chunk file that isn't a scene.
	iVersion = SCENE_FILE_VERSION;
	BeginHandMadeFile(&cFile, 0x00123456);
	WriteHandMadeChunk(&cFile, SCENE_CHUNK_VERSION, &iVersion, sizeof(int32));
	AssertTrue(cFile.WriteClose());
	cFile.Kill();
	AssertWontOpen();

	//With no version.
	BeginHandMadeFile(&cFile, SCENE_FILE_ID);
	AssertTrue(cFile.WriteClose());
	cFile.Kill();
	AssertWontOpen();

	//With a version from the future.
	iVersion = SCENE_FILE_VERSION + 1;
	BeginHandMadeFile(&cFile, SCENE_FILE_ID);
	WriteHandMadeChunk(&cFile, SCENE_CHUNK_VERSION, &iVersion, sizeof(int32));
	AssertTrue(cFile.WriteClose());
	cFile.Kill();
	AssertWontOpen();

	//A chunk whose size doesn't fit what it holds.
	iVersion = SCENE_FILE_VERSION;
	BeginHandMadeFile(&cFile, SCENE_FILE_ID);
	WriteHandMadeChunk(&cFile, SCENE_CHUNK_VERSION, &iVersion, sizeof(int32));
	AssertTrue(cFile.WriteChunkBegin());
	WriteHandMadeChunk(&cFile, SCENE_CHUNK_POSITIONS, gafPositions, 7);
	WriteHandMadeChunk(&cFile, SCENE_CHUNK_WORLD, gafIdentity, 8);
	AssertTrue(cFile.WriteChunkEnd(SCENE_CHUNK_MESH));
	AssertTrue(cFile.WriteClose());
	cFile.Kill();

	cReader.Init(BAD_FILE);
	AssertTrue(cReader.Open());
	AssertTrue(cReader.BeginMesh(0));
	AssertFalse(cReader.Failed());
	AssertSize(0, cReader.NumPositions());
	AssertTrue(cReader.Failed());
	AssertFalse(cReader.ReadPositions(af));
	AssertTrue(cReader.End());
	AssertTrue(cReader.Close());
	cReader.Kill();

	cReader.Init(BAD_FILE);
	AssertTrue(cReader.Open());
	AssertTrue(cReader.BeginMesh(0));
	AssertFalse(cReader.ReadWorldMatrix(af));
	AssertTrue(cReader.Failed());
	AssertTrue(cReader.End());
	AssertTrue(cReader.Close());
	cReader.Kill();

	//Closing what isn't open.
	cReader.Init(BAD_FILE);
	AssertFalse(cReader.Close());
	cReader.Kill();

	AssertTrue(cFileUtil.Delete(BAD_FILE));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSceneFile(void)
{
	CFileUtil	cFileUtil;
	SLogConfig	sLogConfig;

	BeginTests();

	cFileUtil.MakeDir("Output");
	cFileUtil.RemoveDir(OUTPUT_DIR);
	cFileUtil.MakeDir(OUTPUT_DIR);

	TestSceneFileSample();
	TestSceneFileWriterForwardReferences();
	TestSceneFileReaderUnknownAndMissing();

	sLogConfig = gcLogger.SetSilent();
	TestSceneFileWriterMeshFailures();
	TestSceneFileWriterReferenceFailures();
	TestSceneFileWriterUseFailures();
	TestSceneFileReaderFailures();
	gcLogger.SetConfig(&sLogConfig);

	cFileUtil.RemoveDir(OUTPUT_DIR);

	TestStatistics();
}

