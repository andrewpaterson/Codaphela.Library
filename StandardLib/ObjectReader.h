#ifndef __OBJECT_READER_H__
#define __OBJECT_READER_H__
#include "CoreLib/IndexedGeneral.h"
#include "SerialisedObject.h"
#include "Unknown.h"


class CObjectReader : public CUnknown
{
BASE_FUNCTIONS(CObjectReader);
public:
			void				Kill(void);

	virtual BOOL				Begin(void);
	virtual CSerialisedObject*  Read(char* szObjectName) =0;
	virtual CSerialisedObject*  Read(OIndex oi) =0;
	virtual BOOL				End(void);

protected:
	CSerialisedObject*  ReadSerialised(CFileReader* pcReader);

};


#endif // __OBJECT_READER_H__

