#ifndef __PRIMITIVE_CHARACTER_H__
#define __PRIMITIVE_CHARACTER_H__
#include "BaseLib/Logger.h"
#include "Primitive.h"


PRIMITIVE_TYPE_DEFINITION(Char8, char8, PT_char8, WriteChar, ReadChar)
PRIMITIVE_TYPE_DEFINITION(Char16, char16, PT_char16, WriteWChar, ReadWChar)


#endif // __PRIMITIVE_CHARACTER_H__

