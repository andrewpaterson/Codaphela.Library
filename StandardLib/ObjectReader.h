#ifndef __OBJECT_READER_H__
#define __OBJECT_READER_H__
#include "CoreLib/IndexedGeneral.h"
#include "SerialisedObjectReader.h"


class CObjectReader : public CSerialisedObjectReader
{
BASE_FUNCTIONS(CObjectReader);
public:
			void				Kill(void);

	virtual BOOL				Begin(void);
	virtual CSerialisedObject*  Read(char* szObjectName) =0;
	virtual CSerialisedObject*  Read(OIndex oi);
	virtual BOOL				End(void);
};


#endif // __OBJECT_READER_H__

