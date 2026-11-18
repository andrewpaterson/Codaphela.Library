#ifndef __TEST_CPP_COMPILER_LIB_PARTS_H__
#define __TEST_CPP_COMPILER_LIB_PARTS_H__
#include "BaseLib/PrimitiveTypes.h"


//A long test file can be run in parts, each in a process of its own (main's -part and -calls, RunParallel.py): its test
//functions are called through TEST_PART, counted from its entry point's TEST_PART_BEGIN.  A part runs the calls -calls
//lists (by their numbers, from 0), or with -part p/n every n-th from the p-th; with neither, every call runs.  With
//TEST_PART_TIMES in the environment each call's time and tests are printed ("TESTPART <file> <call> <seconds> <tests>"), which RunParallel.py
//keeps to share the calls out evenly next time.  TestPartLabel names the process's part, for what it writes beside the
//others: its log's name without Log.txt (TESTLIB_LOG), or "" for the whole run.
void	TestPartBegin(const char* szFile);
bool	TestPartStarts(void);
void	TestPartEnds(void);
const char*	TestPartLabel(void);


#define TEST_PART_BEGIN(szFile)		TestPartBegin(szFile)
#define TEST_PART(call)				do { if (TestPartStarts()) { call; TestPartEnds(); } } while (0)


#endif // __TEST_CPP_COMPILER_LIB_PARTS_H__
