#ifndef __CHAR_ENCODING_H__
#define __CHAR_ENCODING_H__


enum ECharEncoding
{
	CE_US_ASCII,
	CE_UTF_8,
	CE_UTF_16,
	CE_ISO_8859_1,
	CE_Unkown,
	CE_NoSet,
};


class CCharEncoding
{
public:
	static ECharEncoding	ParseEncoding(char* szEncodingString);
};


#endif // __CHAR_ENCODING_H__

