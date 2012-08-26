#ifndef __OBJECT_IO_SIMPLE_H__
#define __OBJECT_IO_SIMPLE_H__
#include "ObjectIO.h"


class CObjectIOSimple : public CObjectIO
{
public:
	void FileName(char* szSerialisedName, char* szWorkingDirectory, CChars* pszFullDirectory, CChars* pszFullFileName);
};


#endif // __OBJECT_IO_SIMPLE_H__

