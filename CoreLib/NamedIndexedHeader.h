#ifndef __NAMED_INDEXED_HEADER_H__
#define __NAMED_INDEXED_HEADER_H__


class CNamedIndexedHeader
{
private:
	int miNameLength;

public:
	void	Init(void* pvData, unsigned int uiDataSize);
	void	Init(char* szName, int iNameLength, void* pvData, unsigned int uiDataSize);
	char*	GetName(void);
	void*	GetData(void);
	int		GetNameLength(void);
	int		GetHeaderSize(void);
	BOOL	HasName(void);
};


size_t NamedIndexedHeaderSize(char* szName, unsigned int uiDataSize);
size_t NamedIndexedHeaderSize(unsigned int uiDataSize);
size_t NamedIndexedHeaderSize(int iNameLength, unsigned int uiDataSize);


#endif // __NAMED_INDEXED_HEADER_H__

