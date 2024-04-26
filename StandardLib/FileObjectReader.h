#ifndef __FILE_OBJECT_READER_H__
#define __FILE_OBJECT_READER_H__
#include "ObjectIO.h"
#include "ExternalObjectReader.h"


class CFileObjectReader : public CExternalObjectReader, public CObjectIO
{
CONSTRUCTABLE(CFileObjectReader);
protected:
	CFileBasic*		mpcFile;

public:
	void				Init(CFileBasic* pcFile);
	void				Kill(void);

	CSerialisedObject*  Read(char* szObjectName);
	CSerialisedObject*	Read(OIndex oi);
};


#endif // __FILE_OBJECT_READER_H__

