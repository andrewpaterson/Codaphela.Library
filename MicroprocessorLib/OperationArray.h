#ifndef __OPERATION_ARRAY_H__
#define __OPERATION_ARRAY_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "Operation.h"


typedef CArrayTemplatePtr<COperation> __COperationArray;


class COperationArray : public __COperationArray
{
public:
	void Kill(void);
};


#endif // __OPERATION_ARRAY_H__


