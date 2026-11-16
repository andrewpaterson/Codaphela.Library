// TestProceduralLib.cpp : Defines the entry point for the console application.
//

#include "BaseLib/LeakDetector.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


void TestMeshMeasures(void);
void TestIsosurfaceGrid(void);
void TestMarchingCubes(void);
void TestIsosurfaceFit(void);
void TestDualContouring(void);
void TestIsosurfaceOctree(void);
void TestMeshShapeEditor(void);
void TestMeshShapeNormals(void);
void TestShapeScene(void);
void TestSolidGenerators(void);
void TestSolidOperators(void);
void TestSolidMaterials(void);
void TestSolidFiltering(void);


int __cdecl main(void)
{
	int iResult;

	BreakOnAllocation(0);
	InitTotalStatistics();

	TypesInit();
	TypeConverterInit();
	MemoryInit();

	TestMeshMeasures();
	TestIsosurfaceGrid();
	TestMarchingCubes();
	TestIsosurfaceFit();
	TestDualContouring();
	TestIsosurfaceOctree();
	TestMeshShapeEditor();
	TestMeshShapeNormals();
	TestShapeScene();
	TestSolidGenerators();
	TestSolidOperators();
	TestSolidMaterials();
	TestSolidFiltering();

	MemoryKill();
	TypeConverterKill();
	TypesKill();

	iResult = TestTotalStatistics();
	LeakDetection();
	return iResult;
}

