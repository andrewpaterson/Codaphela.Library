#ifndef __MALLOCATORS_H__
#define __MALLOCATORS_H__
#include "MapStringTemplate.h"
#include "Mallocator.h"


class CMallocators
{
protected:
	CMapStringTemplate<CMallocator*>	mmszcMallocators;

public:
	void Init(void);
	void Kill(void);

	BOOL AddMallocator(CMallocator* pcMallocator);
};


#endif // __MALLOCATORS_H__

