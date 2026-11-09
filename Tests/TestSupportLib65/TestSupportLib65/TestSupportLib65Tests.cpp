#include "TestLib/Assert.h"
#include "../../TestBaseLib65/TestBaseLib65/TestProject65.h"


//////////////////////////////////////////////////////////////////////////
//
// TestSupportLib's tests, with SupportLib, StandardLib, BaseLib and TestLib, each run on the 65C64 (see
// TestProject65.cpp).
//
//////////////////////////////////////////////////////////////////////////


static const char* gaszTestSupportLib65Libraries[] = { "TestLib", "SupportLib", "StandardLib", "BaseLib", NULL };


//The tests that are not run.
static const char* gaszTestSupportLib65Excluded[] =
{
	//More than the 4G instructions each program gets here.  They pass when given longer: TestTrueTypeFont in 5.1G
	//65C64 instructions, TestModPlayer in 9.8G, TestFont in 16.6G, TestSoundResampler in 16.8G, TestMeshShapes in under
	//20G, TestImageResampler in 24.0G, TestImageOpacity in 24.6G and TestNormalGeneration in 35.2G.  TestModPlayer
	//passes on the W65C816 too (TEST_PROJECT65_STEPS=60000), in 76.3G cycles, since CModPlayer::RenderSong renders into the sound a block at a
	//time (it kept the whole song as float64s, about 30MB, more than the W65C816's 16MB).
	"TestTrueTypeFont",
	"TestModPlayer",
	"TestFont",
	"TestSoundResampler",
	"TestMeshShapes",
	"TestImageResampler",
	"TestImageOpacity",
	"TestNormalGeneration",

	//Its 18MB combined image (it raises the debug allocation limit to 32MB) does not fit in the 48MB heap: SavePNG
	//cannot allocate memory to write it, after 51.6G instructions.
	"TestImageCombiner",

	NULL
};


//And the tests that are not run on the W65C816.
static const char* gaszTestSupportLib65Excluded816[] =
{
	//More than the 4G instructions each program gets here (the W65C816's code is slower).  They pass when given longer
	//(TEST_PROJECT65_STEPS=30000): TestImageSFTWriter in 17.3G cycles, TestRectangleBestPackerWithRotation in 18.6G.
	"TestImageSFTWriter",
	"TestRectangleBestPackerWithRotation",

	NULL
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSupportLib65Tests(void)
{
	STestProject65	sProject;

	BeginTests();

	sProject.szTestProject = "TestSupportLib";
	sProject.aszLibraries = gaszTestSupportLib65Libraries;
	sProject.aszExcluded = gaszTestSupportLib65Excluded;
	sProject.aszExcluded816 = gaszTestSupportLib65Excluded816;
	sProject.uiMinPrograms = 48;
	sProject.iMinTestsRun = 25116;
	sProject.uiMinPrograms816 = 46;
	sProject.iMinTestsRun816 = 16725;
	TestProject65RunTests(&sProject);

	TestStatistics();
}
