//Character classification for the 65C64, ASCII only.
#ifndef __LIBC65_CTYPE_H__
#define __LIBC65_CTYPE_H__


extern "C"
{
int		isalnum(int c);
int		isalpha(int c);
int		isblank(int c);
int		iscntrl(int c);
int		isdigit(int c);
int		isgraph(int c);
int		islower(int c);
int		isprint(int c);
int		ispunct(int c);
int		isspace(int c);
int		isupper(int c);
int		isxdigit(int c);
int		tolower(int c);
int		toupper(int c);
}


#endif // __LIBC65_CTYPE_H__
