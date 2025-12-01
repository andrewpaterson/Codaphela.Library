#ifndef __INSETS_H__
#define __INSETS_H__


//BORDER_EDGE_INDEX
#define BEI_Top 0
#define BEI_Right 1
#define BEI_Bottom 2
#define BEI_Left 3


//BORDER_CORNER_INDEX
#define BCI_TopLeft 0
#define BCI_TopRight 1
#define BCI_BottomRight 2
#define BCI_BottomLeft 3


struct SInsets
{
	int maiWidths[4];

	void Init(void);
	void Init(int* paiWidths);
}; 


#endif // __INSETS_H__

