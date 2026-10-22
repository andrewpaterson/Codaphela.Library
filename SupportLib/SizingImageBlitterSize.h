#ifndef __SIZING_IMAGE_BLITTER_H__
#define __SIZING_IMAGE_BLITTER_H__
#include "BaseLib/Int32Vec2.h"
#include "BaseLib/ArrayInt32.h"


class CSubImage;
class CSizingImageBlitterSize
{
public:
	//Built for the last size the cel was drawn at that was not its own.
	SInt32Vec2		msSizedTo;
	CArrayInt32		maiSizedSourceX;	//For each destination column the source image column drawn into it.
	CArrayInt32		maiSizedSourceY;	//For each destination row the row of the cel's image drawn into it, outside the image in the cel's transparent borders.
	CArrayInt32		maiSizedDestX;		//For each column of the full cel the first destination column drawn from it or a column to its right.

	void	Init(void);
	void	Kill(void);

	bool	PopulateSizingTables(CSubImage* pcSubImage, int32 iDestWidth, int32 iDestHeight);

	int32*	GetSizedSourceX(void);
	int32*	GetSizedSourceY(void);
	int32*	GetSizedDestX(void);

protected:
	bool	Matches(int32 iDestWidth, int32 iDestHeight);
	void	CacheBresenhamIndices(CArrayInt32* paiSamples, int32 iDestLength, int32 iFullLength, int32 iOffset);
};


#endif // __SIZING_IMAGE_BLITTER_H__

