#ifndef __TILE_CEL_CHAR_SOURCE_H__
#define __TILE_CEL_CHAR_SOURCE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/LinkedListTemplate.h"
#include "BaseLib/Chars.h"
#include "TileCelSource.h"


class CTileCelCharSource : public CTileCelSource
{
protected:
	char	mc;

public:
	void	Init(char c);
	void	Kill(void) override;
	void	Print(CChars* psz) override;

	char	GetChar(void);

	bool	Matches(void* pvData) override;
};


#endif // __TILE_CEL_CHAR_SOURCE_H__

