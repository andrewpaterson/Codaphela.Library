#include "BaseLib/LeakDetector.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Unknowns.h"
#include "TestLib/Assert.h"


void TestPlainTextEditor(void);
void TestBumpMapper(void);
void TestImage(void);
void TestImageOpacity(void);
void TestImageCel(void);
void TestImageCelsSource(void);
void TestImageWriter(void);
void TestImageBMP(void);
void TestImageTGA(void);
void TestImagePNG(void);
void TestCompressionRLE(void);
void TestImageColour(void);
void TestImageCombiner(void);
void TestImageDivider(void);
void TestImageDraw(void);
void TestImageModifierDrawBox(void);
void TestImageGreyToRGB(void);
void TestImageModifierStack(void);
void TestImageReader(void);
void TestImageRecolour(void);
void TestImageRGBToGrey(void);
void TestImageSwizzle(void);
void TestRectangleBestPacker(void);
void TestRectangle(void);
void TestRectanglePow2Packer(void);
void TestRectangleLinePacker(void);
void TestRectangleBestPackerWithRotation(void);
void TestImageToR3G3B2A(void);
void TestImageSFTWriter(void);
void TestImageCelsFile(void);
void TestFont(void);
void TestTrueTypeFont(void);
void TestFontFamily(void);
void TestTextMarkupParser(void);
void TestGeometryMap(void);
void TestMaps(void);
void TestMapViewport(void);
void TestTileMapGenerator(void);
void TestHalfSpace(void);
void TestMeshColours(void);
void TestMeshUVCoords(void);
void TestMeshMaterials(void);
void TestPolygon(void);
void TestSphereShape(void);
void TestTriangleShape(void);
void TestMeshFaceReturn(void);
void TestMeshPolygons(void);
void TestMeshShapes(void);
void TestMeshConnectivity(void);
void TestNormalGeneration(void);
void TestMeshOBJReader(void);
void TestImageResampler(void);
void TestSound(void);
void TestSoundWAV(void);
void TestSoundResampler(void);
void TestSoundCel(void);
void TestModPlayer(void);


int __cdecl main(void)
{
	CFileUtil	cFileUtil;
	int			iResult;

	BreakOnAllocation(0);
	InitTotalStatistics();
	InitAllocatorDebug(4 MB);

	TypesInit();
	TypeConverterInit();
	MemoryInit();

	cFileUtil.RemoveDir("Output");
	cFileUtil.MakeDir("Output");
	
	TestImage();
	TestImageOpacity();
	TestImageColour();
	TestImageReader();
	TestImageWriter();
	TestImageBMP();
	TestImageTGA();
	TestImagePNG();
	TestCompressionRLE();
	TestImageGreyToRGB();
	TestImageRGBToGrey();
	TestImageDivider();
	TestBumpMapper();
	TestPlainTextEditor();
	TestImageCel();
	TestRectangle();
	TestRectangleBestPacker();
	TestRectanglePow2Packer();
	TestRectangleLinePacker();
	TestRectangleBestPackerWithRotation();
	TestImageModifierStack();
	TestImageResampler();
	TestImageModifierDrawBox();
	TestImageDraw();
	TestImageRecolour();
	TestImageSwizzle();
	TestImageCombiner();
	TestImageToR3G3B2A();
	TestImageSFTWriter();
	TestImageCelsFile();
	TestMaps();
	TestMapViewport();
	TestTileMapGenerator();
	TestFont();
	TestTrueTypeFont();
	TestFontFamily();
	TestTextMarkupParser();
	TestGeometryMap();
	TestImageCelsSource();
	TestHalfSpace();
	TestPolygon();
	TestMeshColours();
	TestMeshUVCoords();
	TestMeshMaterials();
	TestSphereShape();
	TestTriangleShape();
	TestMeshFaceReturn();
	TestMeshPolygons();
	//TestMeshShapes();  Write is not consistent.
	TestMeshConnectivity();
	TestNormalGeneration();
	//TestMeshOBJReader();  Source OBJ file is missing
	TestSound();
	TestSoundWAV();
	TestSoundResampler();
	TestSoundCel();
	TestModPlayer();

	cFileUtil.RemoveDir("Output");

	MemoryKill();
	TypeConverterKill();
	TypesKill();

	iResult = TestTotalStatistics();
	LeakDetection();
	return iResult;
}

