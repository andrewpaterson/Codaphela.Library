#ifndef __INDEX_NEW_OLD_H__
#define __INDEX_NEW_OLD_H__
#include "BaseLib/ArrayTemplate.h"
#include "CoreLib/IndexedGeneral.h"


class CIndexNewOld
{
public:
	OIndex	moiNew;
	OIndex	moiOld;

	void Init(OIndex oiNew, OIndex oiOld);
};


typedef CArrayTemplate<CIndexNewOld>	CArrayIndexNewOld;


#endif // __INDEX_NEW_OLD_H__

