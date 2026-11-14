#include "BaseLib/SharedLibrary.h"
#include "TestLib/Assert.h"


#if defined(_WIN32)
#define TEST_SHARED_LIBRARY		"kernel32.dll"
#define TEST_SHARED_FUNCTION	"GetTickCount"
#elif defined(LINUX_GNU)
#define TEST_SHARED_LIBRARY		"libc.so.6"
#define TEST_SHARED_FUNCTION	"strlen"
#endif


//////////////////////////////////////////////////////////////////////////
//
// A library the system always has opens and has a function it always has,
// and not one it hasn't.  One that isn't there doesn't open.  The 65C64
// opens none.
//
//////////////////////////////////////////////////////////////////////////
void TestSharedLibraryOpen(void)
{
	CSharedLibrary	cLibrary;

	cLibrary.Init();
	AssertFalse(cLibrary.IsOpen());
	AssertNull(cLibrary.GetFunction("Anything"));

#ifdef TEST_SHARED_LIBRARY
	AssertTrue(cLibrary.Open(TEST_SHARED_LIBRARY));
	AssertTrue(cLibrary.IsOpen());
	AssertNotNull(cLibrary.GetFunction(TEST_SHARED_FUNCTION));
	AssertNull(cLibrary.GetFunction("NoFunctionIsCalledThis"));
	cLibrary.Close();
	AssertFalse(cLibrary.IsOpen());
	AssertNull(cLibrary.GetFunction(TEST_SHARED_FUNCTION));
#endif

	AssertFalse(cLibrary.Open("NoLibraryIsCalledThis"));
	AssertFalse(cLibrary.IsOpen());
	cLibrary.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSharedLibrary(void)
{
	BeginTests();

	TestSharedLibraryOpen();

	TestStatistics();
}

