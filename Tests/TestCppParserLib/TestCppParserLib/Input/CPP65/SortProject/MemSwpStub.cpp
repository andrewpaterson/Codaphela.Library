#include "BaseLib/StringHelper.h"


//StringHelper.cpp has floating point, which the 65C64 generator does not do yet, so its MemSwp is here.
void MemSwp(const void* pv1, const void* pv2, lsize uiLength)
{
	lsize	i;
	char	cTemp;

	for (i = 0; i < uiLength; i++)
	{
		cTemp = ((char*)pv1)[i];
		((char*)pv1)[i] = ((char*)pv2)[i];
		((char*)pv2)[i] = cTemp;
	}
}

