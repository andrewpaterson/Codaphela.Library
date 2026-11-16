#ifndef __MEMORY_CACHE_DESCRIPTOR_H__
#define __MEMORY_CACHE_DESCRIPTOR_H__
#include "PrimitiveTypes.h"


class SMemoryCacheDescriptor
{
private:
	SMemoryCacheDescriptor*		mpsNext;
	SMemoryCacheDescriptor*		mpsPrev;
	lsize						miSize;  //Not including the sizeof this.

public:
	void						SetNext(SMemoryCacheDescriptor* psNext);
	void						SetPrev(SMemoryCacheDescriptor* psPrev);
	void						SetSize(lsize iSize);

	void						Clear(void);
	void						ClearNext(void);
	void						ClearPrev(void);

	lsize						GetSize(void) const;
	SMemoryCacheDescriptor*		GetNext(void) const;
	SMemoryCacheDescriptor*		GetPrev(void) const;

	bool						IsSize(lsize iSize);
	bool						IsNext(SMemoryCacheDescriptor* psDesc);
	bool						IsPrev(SMemoryCacheDescriptor* psDesc);
};


#endif // __MEMORY_CACHE_DESCRIPTOR_H__

