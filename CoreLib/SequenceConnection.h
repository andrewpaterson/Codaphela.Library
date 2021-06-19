#ifndef __SEQUENCE_CONNECTION_H__
#define __SEQUENCE_CONNECTION_H__
#include "IndexedGeneral.h"


class CSequenceConnection
{
public:
	virtual OIndex	GetNext(void) =0;
	virtual OIndex	PeekNext(void) =0;

};


#endif // !__SEQUENCE_CONNECTION_H__

