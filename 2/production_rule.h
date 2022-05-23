#ifndef __PRODUCTION_RULE_H__
#define __PRODUCTION_RULE_H__

#include <cstring>
using namespace std;

typedef struct Production {
    string leftPart;
    string rightPart[10];
    int rightPartLength;
} Production;

#endif