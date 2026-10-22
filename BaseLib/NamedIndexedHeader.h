#ifndef __NAMED_INDEXED_HEADER_H__
#define __NAMED_INDEXED_HEADER_H__


class CNamedIndexedHeader
{
private:
	int miNameLength;

public:
	void	Init(void* pvData, size uiDataSize);
	void	Init(const char* szName, int iNameLength, void* pvData, size uiDataSize);
	char*	GetName(void);
	void*	GetData(void);
	int		GetNameLength(void);
	int		GetHeaderSize(void);
	bool	HasName(void);
};


size NamedIndexedHeaderSize(char* szName, size uiDataSize);
size NamedIndexedHeaderSize(size uiDataSize);
size NamedIndexedHeaderSize(int iNameLength, size uiDataSize);


#endif // __NAMED_INDEXED_HEADER_H__

