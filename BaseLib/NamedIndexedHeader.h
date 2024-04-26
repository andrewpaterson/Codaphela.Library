#ifndef __NAMED_INDEXED_HEADER_H__
#define __NAMED_INDEXED_HEADER_H__


class CNamedIndexedHeader
{
private:
	int miNameLength;

public:
	void	Init(void* pvData, uint32 uiDataSize);
	void	Init(char* szName, int iNameLength, void* pvData, uint32 uiDataSize);
	char*	GetName(void);
	void*	GetData(void);
	int		GetNameLength(void);
	int		GetHeaderSize(void);
	bool	HasName(void);
};


size_t NamedIndexedHeaderSize(char* szName, uint32 uiDataSize);
size_t NamedIndexedHeaderSize(uint32 uiDataSize);
size_t NamedIndexedHeaderSize(int iNameLength, uint32 uiDataSize);


#endif // __NAMED_INDEXED_HEADER_H__

