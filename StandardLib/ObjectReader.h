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
	virtual CSerialisedObject*  Read(char* szChunkName) =0;
	virtual BOOL				End(void);
};


#endif // __OBJECT_READER_H__

