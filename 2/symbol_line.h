#ifndef __SYMBOL_LINE_H__
#define __SYMBOL_LINE_H__

#include <cstring>

using namespace std;

typedef struct SymbolLine {
    string addr;
    string value;
    string type;
    string vType;
} SymbolLine;

#endif