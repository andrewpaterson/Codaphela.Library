#ifndef __OBJECT_IO_H__
#define __OBJECT_IO_H__
#include "BaseLib/Chars.h"
#include "BaseLib/IndexedGeneral.h"
#include "ObjectFileGeneral.h"


class CSerialisedObject;
class CObjectIO
{
public:
	static char*	Unnamed(OIndex oi, CChars* pszUnnamed);
	static char*	Unnamed(CSerialisedObject* pcSerialised, CChars* pszUnnamed);
	static char*	Filename(const char* szSerialisedName, const char* szWorkingDirectory, CChars* pszFullDirectory, CChars* pszFullFilename);
};


#endif // __OBJECT_IO_H__

