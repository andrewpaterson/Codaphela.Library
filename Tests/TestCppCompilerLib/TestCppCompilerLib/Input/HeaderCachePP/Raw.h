#ifndef RAW_H
#define RAW_H
#define RAW_TEXT R"x(two
lines \
#endif "and" )y" more)x"
const char* Raw = RAW_TEXT R"(one
// not a comment
)"; int RawLine = __LINE__;
#endif
