#ifndef __OBJECT_IO_H__
#define __OBJECT_IO_H__
#include "BaseLib/Chars.h"
#include "ObjectFileGeneral.h"


class CSerialisedObject;
class CObjectIO
{
public:
	static char*	Unnamed(CSerialisedObject* pcSerialised, CChars* pszUnnamed);
	static char*	FileName(char* szSerialisedName, char* szWorkingDirectory, CChars* pszFullDirectory, CChars* pszFullFileName);
};


#endif // !__OBJECT_IO_H__

