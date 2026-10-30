#ifndef __NAMED_INDEXED_HEADER_H__
#define __NAMED_INDEXED_HEADER_H__


class CNamedIndexedHeader
{
private:
	int miNameLength;

public:
	void	Init(void* pvData, lsize uiDataSize);
	void	Init(const char* szName, int iNameLength, void* pvData, lsize uiDataSize);
	char*	GetName(void);
	void*	GetData(void);
	int		GetNameLength(void);
	int		GetHeaderSize(void);
	bool	HasName(void);
};


lsize NamedIndexedHeaderSize(char* szName, lsize uiDataSize);
lsize NamedIndexedHeaderSize(lsize uiDataSize);
lsize NamedIndexedHeaderSize(int iNameLength, lsize uiDataSize);


#endif // __NAMED_INDEXED_HEADER_H__

