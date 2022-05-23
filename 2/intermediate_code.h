#ifndef __INTERMEDIATECODE_H__
#define __INTERMEDIATECODE_H__

#include <cstring>
using namespace std;

typedef struct IntermediateCodeLine {
    int lineNum; //行号
    string code; //三地址码
} IntermediateCodeLine;

#endif