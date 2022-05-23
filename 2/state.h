#ifndef __STATE_H__
#define __STATE_H__

#include "item.h"

#define MAX 10000

using namespace std;

typedef struct State {
    int id{};
    Item *items = (Item*)malloc(100 * sizeof(Item));
    int top = 0;
} State;

#endif