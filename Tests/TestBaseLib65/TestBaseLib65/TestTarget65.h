#ifndef __TEST_TARGET_65_H__
#define __TEST_TARGET_65_H__
#include "CppCompilerLib/CPPTarget.h"


//The processor a 65 test (TestBaseLib65, TestStandardLib65, TestSupportLib65, HCP65's TestLibc65) compiles for and
//runs on, as its command line names it.
bool		TestTarget65Choose(int argc, char** argv, ECPPTarget* peDest);
const char*	TestTarget65Name(ECPPTarget eTarget);


#endif // __TEST_TARGET_65_H__
