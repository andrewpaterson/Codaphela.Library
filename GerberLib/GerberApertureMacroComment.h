#ifndef __GERBER_APERTURE_MACRO_COMMENT_H__
#define __GERBER_APERTURE_MACRO_COMMENT_H__
#include "BaseLib/Chars.h"
#include "GerberApertureMacro.h"


class CGerberApertureMacroComment : public CGerberApertureMacro
{
protected:
	CChars	szComment;

public:
	void	Init(size uiLength);
	void	Kill(void);

	bool	IsComment(void);

	char*	Text(void);
};


#endif // __GERBER_APERTURE_MACRO_COMMENT_H__

