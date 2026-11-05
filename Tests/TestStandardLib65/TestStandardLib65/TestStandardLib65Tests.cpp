#include "TestLib/Assert.h"
#include "../../TestBaseLib65/TestBaseLib65/TestProject65.h"


//////////////////////////////////////////////////////////////////////////
//
// TestStandardLib's tests, with StandardLib, BaseLib and TestLib, each run on the 65C64 (see TestProject65.cpp).
//
//////////////////////////////////////////////////////////////////////////


static const char* gaszTestStandardLib65Libraries[] = { "TestLib", "StandardLib", "BaseLib", NULL };


//The tests that are not run.
static const char* gaszTestStandardLib65Excluded[] =
{
	//More than the 4G instructions each program gets here.  They pass when given longer: TestCompressionZLib in 5.1G
	//65C64 instructions, TestMapObject in 9.7G, TestSetObject in 12.7G and TestArrayObject in 20.0G.
	"TestCompressionZLib",
	"TestArrayObject",
	"TestSetObject",
	"TestMapObject",

	NULL
};


//And the tests that are not run on the W65C816.
static const char* gaszTestStandardLib65Excluded816[] =
{
	//More than the 4G instructions each program gets here (3.4G on the 65C64; the W65C816's code is slower).  They
	//pass when given longer (TEST_PROJECT65_STEPS).
	"TestSetUnknown",
	"TestMapUnknownUnknown",

	NULL
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStandardLib65Tests(void)
{
	STestProject65	sProject;

	BeginTests();

	sProject.szTestProject = "TestStandardLib";
	sProject.aszLibraries = gaszTestStandardLib65Libraries;
	sProject.aszExcluded = gaszTestStandardLib65Excluded;
	sProject.aszExcluded816 = gaszTestStandardLib65Excluded816;
	sProject.uiMinPrograms = 57;
	sProject.iMinTestsRun = 4877;
	sProject.uiMinPrograms816 = 55;
	sProject.iMinTestsRun816 = 4662;
	TestProject65RunTests(&sProject);

	TestStatistics();
}
