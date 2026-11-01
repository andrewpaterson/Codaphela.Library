#ifndef __SUPPORT_ASSERT_H__
#define __SUPPORT_ASSERT_H__
#include "StandardLib/Pointer.h"


#define AssertOPointer(e, a)	Validate(PrivateAssertOPointer(e, a, NULL, __LINE__, __FILE__))


bool PrivateAssertOPointer(CPointer pvExpected, CPointer pvActual, char* szPrefix, size iLine, char* szFile);


#endif // __SUPPORT_ASSERT_H__

