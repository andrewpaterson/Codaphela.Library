#ifndef __OBJECT_FILE_GENERAL__
#define __OBJECT_FILE_GENERAL__


#define OBJECT_FILE_EXTENSION	"DRG"

//Magic file values.
#define BASIC_OBJECT_FILE		'\0SAB'
#define CHUNKED_OBJECT_FILE		'\0KHC'

#define OBJECT_DATA				'\0TAD'
#define OBJECT_FROM_HEAP		'\0AEH'

#define OBJECT_POINTER_NULL		'\0LUN'
#define OBJECT_POINTER_ID		'\0XDI'
#define OBJECT_POINTER_NAMED	'\0MAN'

#define OBJECT_UNNAMED_FILE		"Unnamed"


//Returns false from the reading or writing function, logging the error, if result is false.  sz holds the object's
//identifier for the message.
#define ObjectFileErrorCheck(result, object, ...) \
if (!result) \
{ \
	CChars			sz; \
\
	sz.Init(); \
	object->PrintIdentifier(&sz); \
	gcLogger.Error2(__VA_ARGS__); \
	sz.Kill(); \
	return false; \
}


#endif // __OBJECT_FILE_GENERAL__

