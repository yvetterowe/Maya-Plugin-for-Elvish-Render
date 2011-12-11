#pragma once

#include <cstdio>
#include <string>

using std::string;
extern string StringPrintf(const char* format, ...);
extern void SStringPrintf(string* dst, const char* format, ...);
extern void StringAppendF(string* dst, const char* format, ...);

