#include "TestLib/Assert.h"
#include "TestProject65.h"


//////////////////////////////////////////////////////////////////////////
//
// TestBaseLib's tests, with BaseLib and TestLib, each run on the 65C64 (see TestProject65.cpp).
//
//////////////////////////////////////////////////////////////////////////


static const char* gaszTestBaseLib65Libraries[] = { "TestLib", "BaseLib", NULL };


//The tests that are not run.
static const char* gaszTestBaseLib65Excluded[] =
{
	//CNativeThread and CConditionVariable have no 65C64 versions (it runs one thread).
	"TestCodabase",
	"TestWriteTransaction",
	"TestReadTransaction",
	"TestTransactionConcurrency",

	//Hundreds of millions to billions of instructions natively, far too many to simulate here.
	"TestNumber",
	"TestFloatPrinter",
	"TestNamedIndexedData",
	"TestIndexTreeAccess",
	"TestFat32",
	"TestFatBench",		//Private: run with TEST_PROJECT65_ONLY.
	"TestFatStress",
	"TestFat32Bugs",		//14G cycles on the 65C64, 44G on the W65C816.

	//Expects a bigger block, malloc'ed after free, at another address (as MSVC and glibc give it).  Libc65's malloc
	//joins the block just freed to the free memory after it, so the bigger block starts in the same place.
	"TestTemporaryMemory",

	NULL
};


//The tests that are not run on the W65C816.
static const char* gaszTestBaseLib65Excluded816[] =
{
	//Allocates more than the W65C816's 16MB (24 bit addresses) holds.
	"TestListVariable",

	//More than the 4G instructions each program gets here (the W65C816's code is slower).  It passes when given longer
	//(TEST_PROJECT65_STEPS).
	"TestMapStringIntAndPointer",

	NULL
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBaseLib65Tests(void)
{
	STestProject65	sProject;

	BeginTests();

	sProject.szTestProject = "TestBaseLib";
	sProject.aszLibraries = gaszTestBaseLib65Libraries;
	sProject.aszExcluded = gaszTestBaseLib65Excluded;
	sProject.aszExcluded816 = gaszTestBaseLib65Excluded816;
	sProject.uiMinPrograms = 101;
	sProject.iMinTestsRun = 30001;
	sProject.uiMinPrograms816 = 155;
	sProject.iMinTestsRun816 = 30001;
	TestProject65RunTests(&sProject);

	TestStatistics();
}
