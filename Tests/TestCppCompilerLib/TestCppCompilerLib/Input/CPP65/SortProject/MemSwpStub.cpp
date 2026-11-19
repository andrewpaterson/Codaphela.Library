#include "BaseLib/StringHelper.h"


//StringHelper.cpp has floating point, which the 65C64 generator does not do yet, so its MemSwp is here.  Its length is
//StringHelper.h's: a size on the W65C816 (16 bits, as size_t is there), an lsize elsewhere.
#if defined(__W65C816__)
typedef size	MemSwpLength;
#else
typedef lsize	MemSwpLength;
#endif // __W65C816__


void MemSwp(const void* pv1, const void* pv2, MemSwpLength uiLength)
{
	MemSwpLength	i;
	char			cTemp;

	for (i = 0; i < uiLength; i++)
	{
		cTemp = ((char*)pv1)[i];
		((char*)pv1)[i] = ((char*)pv2)[i];
		((char*)pv2)[i] = cTemp;
	}
}

