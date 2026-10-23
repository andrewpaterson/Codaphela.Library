//Character classification for the 65C64, ASCII only.
#include <ctype.h>


int isdigit(int c)		{ return (c >= '0') && (c <= '9'); }
int islower(int c)		{ return (c >= 'a') && (c <= 'z'); }
int isupper(int c)		{ return (c >= 'A') && (c <= 'Z'); }
int isalpha(int c)		{ return islower(c) || isupper(c); }
int isalnum(int c)		{ return isalpha(c) || isdigit(c); }
int isxdigit(int c)		{ return isdigit(c) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')); }
int isblank(int c)		{ return (c == ' ') || (c == '\t'); }
int isspace(int c)		{ return (c == ' ') || ((c >= '\t') && (c <= '\r')); }
int iscntrl(int c)		{ return ((c >= 0) && (c < ' ')) || (c == 127); }
int isprint(int c)		{ return (c >= ' ') && (c < 127); }
int isgraph(int c)		{ return (c > ' ') && (c < 127); }
int ispunct(int c)		{ return isgraph(c) && !isalnum(c); }
int tolower(int c)		{ return isupper(c) ? c - 'A' + 'a' : c; }
int toupper(int c)		{ return islower(c) ? c - 'a' + 'A' : c; }
