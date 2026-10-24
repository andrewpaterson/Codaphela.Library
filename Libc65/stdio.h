//Files and formatted output (stdio.cpp).  stdout and stderr are the console; files are the host's (host.h).
#ifndef __LIBC65_STDIO_H__
#define __LIBC65_STDIO_H__
#include <stddef.h>
#include <stdarg.h>


#define EOF			(-1)
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#define BUFSIZ		512
#define FILENAME_MAX	260


struct FILE;
typedef long		fpos_t;


extern "C"
{
extern FILE*	stdin;
extern FILE*	stdout;
extern FILE*	stderr;

FILE*	fopen(const char* szName, const char* szMode);
int		fclose(FILE* pcFile);
size_t	fread(void* pv, size_t uiSize, size_t uiCount, FILE* pcFile);
size_t	fwrite(const void* pv, size_t uiSize, size_t uiCount, FILE* pcFile);
int		fseek(FILE* pcFile, long iOffset, int iOrigin);
long	ftell(FILE* pcFile);
int		fflush(FILE* pcFile);
int		feof(FILE* pcFile);
int		fgetc(FILE* pcFile);
int		fputc(int c, FILE* pcFile);
int		fputs(const char* sz, FILE* pcFile);
int		puts(const char* sz);
int		putchar(int c);
int		remove(const char* szName);
int		rename(const char* szOld, const char* szNew);

int		printf(const char* szFormat, ...);
int		fprintf(FILE* pcFile, const char* szFormat, ...);
int		sprintf(char* szDest, const char* szFormat, ...);
int		snprintf(char* szDest, size_t uiSize, const char* szFormat, ...);
int		vsprintf(char* szDest, const char* szFormat, va_list vaArguments);
int		vsnprintf(char* szDest, size_t uiSize, const char* szFormat, va_list vaArguments);
int		vprintf(const char* szFormat, va_list vaArguments);
int		vfprintf(FILE* pcFile, const char* szFormat, va_list vaArguments);
}


#endif // __LIBC65_STDIO_H__
