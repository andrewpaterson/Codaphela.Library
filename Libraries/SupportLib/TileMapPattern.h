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
	size	miBX;
	size	miBY;
	size	miWidth;
	size	miHeight;

public:
	bool	Init(char* szConstantSource, size iType, char* szConstantName, char* szPattern);
	void	Init(char* szConstantSource, size iType, char* szConstantName);
	void	Kill(void);

	void	AddRow(char* szRowPattern);
	bool	Done(void);

	bool	IsNamed(size iType, char* szConstantName);
	bool	IsSource(char* szConstantSource);

	size	GetType(void) const;
	char*	GetConstantName(void) const;
	size	GetWidth(void) const;
	size	GetHeight(void) const;
	size	GetBX(void) const;
	size	GetBY(void) const;
	char	GetChar(size x, size y);
};


typedef CArrayTemplate<CTileMapPattern> CArrayTileMapPattern;


#endif // __TILE_MAP_PATTERN_H__

