#include "BaseLib/DebugOutput.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/PackFilePacker.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPackFilesPackerSimple(void)
{
	CFileUtil			cFileUtil;
	CPackFilePacker		cPacker;
	
	cFileUtil.RemoveDir("Output" _FS_ "Packer");
	cFileUtil.MakeDir("Output" _FS_ "Packer");
	cFileUtil.MakeDir("Output" _FS_ "Packer" _FS_ "Sounds");

	cPacker.Pack("Output" _FS_ "Packer" _FS_ "Sounds" _FS_ "Cheese.PAK", "Input" _FS_ "Game1" _FS_ "PAK" _FS_ "Cheese");
	AssertFile("Input" _FS_ "Game2" _FS_ "Sounds" _FS_ "Cheese.PAK", "Output" _FS_ "Packer" _FS_ "Sounds" _FS_ "Cheese.PAK");

	cPacker.Pack("Output" _FS_ "Packer" _FS_ "Sounds" _FS_ "Santa.PAK", "Input" _FS_ "Game1" _FS_ "PAK" _FS_ "Santa");
	AssertFile("Input" _FS_ "Game2" _FS_ "Sounds" _FS_ "Santa.PAK", "Output" _FS_ "Packer" _FS_ "Sounds" _FS_ "Santa.PAK");

	cPacker.Pack("Output" _FS_ "Packer" _FS_ "Models.PAK", "Input" _FS_ "Game1" _FS_ "PAK" _FS_ "Models");
	AssertFile("Input" _FS_ "Game2" _FS_ "Models.PAK", "Output" _FS_ "Packer" _FS_ "Models.PAK");

	cPacker.Pack("Output" _FS_ "Packer" _FS_ "Sounds.PAK", "Input" _FS_ "Game1" _FS_ "PAK" _FS_ "Sounds");
	AssertFile("Input" _FS_ "Game2" _FS_ "Sounds.PAK", "Output" _FS_ "Packer" _FS_ "Sounds.PAK");

	cPacker.Pack("Output" _FS_ "Packer" _FS_ "Textures.PAK", "Input" _FS_ "Game1" _FS_ "PAK" _FS_ "Textures");
	AssertFile("Input" _FS_ "Game2" _FS_ "Textures.PAK", "Output" _FS_ "Packer" _FS_ "Textures.PAK");

	cPacker.Pack("Output" _FS_ "Packer" _FS_ "Videos.PAK", "Input" _FS_ "Game1" _FS_ "PAK" _FS_ "Videos");
	AssertFile("Input" _FS_ "Game2" _FS_ "Videos.PAK", "Output" _FS_ "Packer" _FS_ "Videos.PAK");

	cPacker.Unpack("Output" _FS_ "Packer" _FS_ "Sounds" _FS_ "Cheese.PAK", "Output" _FS_ "Packer");
	AssertFile("Input" _FS_ "Game1" _FS_ "PAK" _FS_ "Cheese" _FS_ "Scream.txt", "Output" _FS_ "Packer" _FS_ "Scream.txt");
	AssertFile("Input" _FS_ "Game1" _FS_ "PAK" _FS_ "Cheese" _FS_ "Moose.txt", "Output" _FS_ "Packer" _FS_ "Moose.txt");

	cFileUtil.RemoveDir("Output" _FS_ "Packer");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPackFilesPacker(void)
{
	BeginTests();
	MemoryInit();

	TestPackFilesPackerSimple();

	MemoryKill();
	TestStatistics();
}

