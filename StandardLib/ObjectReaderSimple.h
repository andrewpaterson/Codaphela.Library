#ifndef __OBJECT_READER_SIMPLE_H__
#define __OBJECT_READER_SIMPLE_H__
#include "ObjectIOSimple.h"
#include "ObjectReader.h"


class CObjectReaderSimple : public CObjectReader, public CObjectIOSimple
{
BASE_FUNCTIONS(CObjectReaderSimple);
protected:
	CFileBasic*		mpcFile;

public:
	void				Init(CFileBasic* pcFile);
	void				Kill(void);

	CSerialisedObject*  Read(char* szObjectName);
};


#endif // __OBJECT_READER_SIMPLE_H__

