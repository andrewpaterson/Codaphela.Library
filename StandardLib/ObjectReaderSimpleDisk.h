#ifndef __OBJECT_READER_SIMPLE_DISK_H__
#define __OBJECT_READER_SIMPLE_DISK_H__
#include "ObjectIOSimple.h"
#include "ObjectReaderSimple.h"


class CObjectReaderSimpleDisk : public CObjectReaderSimple
{
BASE_FUNCTIONS(CObjectReaderSimpleDisk);
protected:
	CChars		mszFullDirectory;
	CFileBasic	mcFile;

public:
	void				Init(char* szDirectory);
	void				Kill(void);

	CSerialisedObject*  Read(char* szObjectName);
};


#endif // __OBJECT_READER_SIMPLE_DISK_H__

