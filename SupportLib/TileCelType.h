#ifndef __TILE_CEL_TYPE_H__
#define __TILE_CEL_TYPE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayEmbeddedInt.h"


class CTileCelType
{
protected:
	CArrayEmbeddedInt	maiCelTypes;
	char				mcPattern;
	bool				mbNot;  //False: CelTypre[0] ||  CelTypre[1] ||  CelTypre[2] ...
								//True: !CelTypre[0] && !CelTypre[1] && !CelTypre[2] ...

public:
	void	Init(char cPattern, int iCelType);
	void	Init(char cPattern, bool bNot);
	void	Kill(void);

	void	AddCelType(int iCelType);

	bool	IsPattern(char cPattern);
	bool	IsNegative(void);
	size	NumCelTypes(void);
	int		GetCelType(size iIndex);
};


typedef CArrayTemplate<CTileCelType> CArrayTileCelType;


#endif // __TILE_CEL_TYPE_H__

