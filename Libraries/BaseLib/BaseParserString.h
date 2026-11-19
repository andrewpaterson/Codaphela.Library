#ifndef __BASE_PARSER_STRING_H__
#define __BASE_PARSER_STRING_H__
#include "Chars.h"
#include "PrimitiveTypes.h"


struct SBaseParserString
{
	CChars	szString;
	lsize	uiLine;
	lsize	uiColumn;

	void		Init(lsize uiColumn, lsize uiLine);
	void		Kill(void);
	const char*	GetString(void) const;
};


#endif // __BASE_PARSER_STRING_H__

