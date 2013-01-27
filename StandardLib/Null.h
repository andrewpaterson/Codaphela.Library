#ifndef __NULL_H__
#define __NULL_H__
#include "Pointer.h"


CPointerObject Null(void);

template<class M>
CPointer<M> Null(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
CPointer<M> Null(void)
{
	CPointer<M>		pObject;
	return pObject;
}


#endif // __NULL_H__
