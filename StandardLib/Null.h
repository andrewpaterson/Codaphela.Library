#ifndef __NULL_H__
#define __NULL_H__
#include "Pointer.h"


CPointerObject Null(void);

template<class M>
Ptr<M> Null(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> Null(void)
{
	Ptr<M>		pObject;
	return pObject;
}


#endif // __NULL_H__
