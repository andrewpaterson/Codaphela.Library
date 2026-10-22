#ifndef __OPERATION_ARRAY_H__
#define __OPERATION_ARRAY_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "W65Operation.h"


typedef CArrayTemplatePtr<CW65Operation> __CW65OperationArray;


class CW65OperationArray : public __CW65OperationArray
{
public:
	void Kill(void);
};


#endif // __OPERATION_ARRAY_H__


