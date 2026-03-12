#ifndef __DATA_FREE_H__
#define __DATA_FREE_H__


typedef void (*DataFree)(const void* pvData);


class CDataFree
{
public:
	virtual void DataWillBeFreed(void* pvData) =0;
};


#endif // __DATA_FREE_H__

