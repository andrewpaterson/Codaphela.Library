#ifndef __TILE_MAP_PATTERN_H__
#define __TILE_MAP_PATTERN_H__
#include "BaseLib/Chars.h"


class CTileMapPattern
{
protected:
	size	miType;
	char*	mszConstantName;
	char*	mszConstantSource;
	CChars	mszPattern;

public:
	bool	Init(char* szConstantSource, size iType, char* szConstantName, char* szPattern);
	void	Init(char* szConstantSource, size iType, char* szConstantName);
	void	Kill(void);

	void	AddRow(char* szRowPattern);
	bool	Done(void);

	bool	IsNamed(size iType, char* szConstantName);
};


typedef CArrayTemplate<CTileMapPattern> CArrayTileMapPattern;


#endif // __TILE_MAP_PATTERN_H__

