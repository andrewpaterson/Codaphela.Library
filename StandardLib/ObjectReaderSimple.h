#ifndef __OBJECT_READER_SIMPLE_H__
#define __OBJECT_READER_SIMPLE_H__
#include "ObjectIOSimple.h"
#include "ObjectReader.h"


class CObjectReaderSimple : public CObjectReader, public CObjectIOSimple
{
BASE_FUNCTIONS(CObjectReaderSimple);
public:
	void				Init(char* szDirectory);
	void				Kill(void);

	BOOL				Begin(void);
	CSerialisedObject*  Read(char* szObjectName);
	BOOL				End(void);
};


#endif // __OBJECT_READER_SIMPLE_H__

