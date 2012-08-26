#ifndef __OBJECT_READER_H__
#define __OBJECT_READER_H__
#include "CoreLib/IndexedGeneral.h"
#include "Unknown.h"
#include "SerialisedObject.h"


class CObjectReader : public CUnknown
{
BASE_FUNCTIONS(CObjectReader);
protected:
	CChars	mszFullDirectory;

public:
			void				Init(char* szFullDirectory);
			void				Kill(void);

	virtual BOOL				Begin(void);
	virtual CSerialisedObject*  Read(char* szObjectName) =0;
	virtual CSerialisedObject*  Read(OIndex oi);
	virtual BOOL				End(void);

protected:
			CSerialisedObject*  ReadSerialised(CFileReader* pcReader);
};


#endif // __OBJECT_READER_H__

