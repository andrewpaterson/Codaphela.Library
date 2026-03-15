#ifndef __DATA_FREE_H__
#define __DATA_FREE_H__


typedef void (*DataFree)(const void* pvData);


class CDataFree
{
public:
	virtual void FreeData(void* pvData) =0;
};


class CDataFreeCallBack : public CDataFree
{
protected:
	DataFree	mfDataFree;

public:
	void	Init(DataFree fDataFree);
	void	FreeData(void* pvData) override;
};


#endif // __DATA_FREE_H__

