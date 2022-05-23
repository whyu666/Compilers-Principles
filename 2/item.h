#ifndef __ITEM_H__
#define __ITEM_H__

#include <cstring>
using namespace std;

typedef struct Item {
    int pId;
    int idx; //在分析栈中最后一个元素的下标
} Item;

#endif