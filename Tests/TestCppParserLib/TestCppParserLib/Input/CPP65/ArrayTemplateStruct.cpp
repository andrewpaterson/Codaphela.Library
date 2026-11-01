#include "BaseLib/ArrayTemplate.h"
#include "ArrayTemplateProject/ArrayBlockStub.h"


//Not trivially constructible, so CPostMalloc constructs each element with placement new.
struct SPoint
{
	short	x;
	short	y;
	int		iMade;

	SPoint() { x = 1; y = 2; iMade = 7; }
	int Sum(void) { return x + y + iMade; }
};


class CPoints : public CArrayTemplate<SPoint>
{
public:
	int Total(void)
	{
		int		iTotal;
		size	i;

		iTotal = 0;
		for (i = 0; i < NumElements(); i++)
		{
			iTotal += Get(i)->Sum();
		}
		return iTotal;
	}
};


int main()
{
	CPoints		cPoints;
	SPoint*		psPoint;

	cPoints.Init();
	cPoints.Add();
	psPoint = cPoints.Add();
	psPoint->x = 30;
	cPoints.Add()->y = -2;
	if (sizeof(SPoint) != 8 || cPoints.ElementSize() != 8)
	{
		return -1;
	}
	return cPoints.Total() * 10 + (int)cPoints.NumElements();
}
