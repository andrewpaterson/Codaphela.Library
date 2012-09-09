#ifndef __SERIALISED_OBJECT_READER_H__
#define __SERIALISED_OBJECT_READER_H__
#include "Unknown.h"
#include "SerialisedObject.h"


class CSerialisedObjectReader : public CUnknown
{
BASE_FUNCTIONS(CSerialisedObjectReader);
public:

	CSerialisedObject*  ReadSerialised(CFileReader* pcReader);
};


#endif // __SERIALISED_OBJECT_READER_H__

