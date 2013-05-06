#ifndef __OBJECT_IO_H__
#define __OBJECT_IO_H__
#include "BaseLib/Chars.h"
#include "ObjectFileGeneral.h"


class CSerialisedObject;
class CObjectIO
{
public:
	char* Unnamed(CSerialisedObject* pcSerialised, CChars* pszUnnamed);
};


#endif // __OBJECT_IO_H__



