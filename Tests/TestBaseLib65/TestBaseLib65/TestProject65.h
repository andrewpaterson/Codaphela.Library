#ifndef __TEST_PROJECT_65_H__
#define __TEST_PROJECT_65_H__
#include "BaseLib/PrimitiveTypes.h"
#include "CppParserLib/CPPTarget.h"


//A test project (such as TestBaseLib) whose tests are compiled with the 65C64 generator and each run on the 65C64, or
//compiled for the W65C816 and run on it when geTestProject65Target says so.
struct STestProject65
{
	const char*		szTestProject;		//The test project's name: its directory is ../../<name>/<name> and its main is in <name>.cpp.
	const char**	aszLibraries;		//The commonlibraries projects it uses, in link order, NULL ended.
	const char**	aszExcluded;		//The tests that are not run, NULL ended.
	const char**	aszExcluded816;		//And the tests that are not run on the W65C816, NULL ended (or NULL).
	size			uiMinPrograms;		//At least this many tests are run,
	int				iMinTestsRun;		//and between them at least this many of their asserts.
	size			uiMinPrograms816;	//The same on the W65C816 (which does not run aszExcluded816).
	int				iMinTestsRun816;
};


extern ECPPTarget	geTestProject65Target;


void TestProject65RunTests(STestProject65* psProject);
void TestProject65ChooseTarget(int argc, char** argv);


#endif // __TEST_PROJECT_65_H__
