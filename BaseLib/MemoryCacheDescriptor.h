#ifndef __MEMORY_CACHE_DESCRIPTOR_H__
#define __MEMORY_CACHE_DESCRIPTOR_H__
#include "PrimitiveTypes.h"


class SMemoryCacheDescriptor
{
private:
	SMemoryCacheDescriptor*		mpsNext;
	SMemoryCacheDescriptor*		mpsPrev;
	size						miSize;  //Not including the sizeof this.

public:
	void						SetNext(SMemoryCacheDescriptor* psNext);
	void						SetPrev(SMemoryCacheDescriptor* psPrev);
	void						SetSize(size iSize);

	void						Clear(void);
	void						ClearNext(void);
	void						ClearPrev(void);

	size						GetSize(void);
	SMemoryCacheDescriptor*		GetNext(void);
	SMemoryCacheDescriptor*		GetPrev(void);

	bool						IsSize(size iSize);
	bool						IsNext(SMemoryCacheDescriptor* psDesc);
	bool						IsPrev(SMemoryCacheDescriptor* psDesc);
};


#endif // __MEMORY_CACHE_DESCRIPTOR_H__

