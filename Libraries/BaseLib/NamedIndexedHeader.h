#ifndef __NAMED_INDEXED_HEADER_H__
#define __NAMED_INDEXED_HEADER_H__


class CNamedIndexedHeader
{
private:
	int32	miNameLength;	//Written to file: the same width on every CPU.

public:
	void	Init(void* pvData, lsize uiDataSize);
	void	Init(const char* szName, int iNameLength, void* pvData, lsize uiDataSize);
	char*	GetName(void) const;
	void*	GetData(void) const;
	int		GetNameLength(void) const;
	int		GetHeaderSize(void) const;
	bool	HasName(void) const;
};


lsize NamedIndexedHeaderSize(char* szName, lsize uiDataSize);
lsize NamedIndexedHeaderSize(lsize uiDataSize);
lsize NamedIndexedHeaderSize(int iNameLength, lsize uiDataSize);


#endif // __NAMED_INDEXED_HEADER_H__

