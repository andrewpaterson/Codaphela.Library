#ifndef __DATA_FREE_H__
#define __DATA_FREE_H__


typedef void (*DataFree)(const void* pvData);


class CDataFree
{
public:
	virtual void DataWillBeFreed(void* pvData) = 0;
};


class CDataFreeCallBack : public CDataFree
{
private:
	DataFree	mDataFree;

public:
	void Init(DataFree fDataFree);
	void DataWillBeFreed(void* pvData);
};


#endif // __DATA_FREE_H__

