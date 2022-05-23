#ifndef __WORD_H__
#define __WORD_H__
#include <cstring>
using namespace std;

typedef struct WORD {
    int code;
    string key;
    string addr = "#";
    string position;
    int lineNum;
    string extra;
    bool variable;
} WORD;

#endif