#ifndef __FONT_CREATION_PARAMETERS_H__
#define __FONT_CREATION_PARAMETERS_H__


class CFontCreationParameters
{
public:
	bool		mbLeftToRightFirst;
	int			miSpaceWidth;
	int			miAscent;
	int			miDescent;

public:
	void		Init(int iCharWidth, int iCharHeight);
	void		Init(bool bLeftToRightFirst, int iSpaceWidth, int iAscent, int iDescent);
};


#endif // __FONT_CREATION_PARAMETERS_H__

