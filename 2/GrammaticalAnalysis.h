#ifndef __GRAMMATICALANALYSIS_H__
#define __GRAMMATICALANALYSIS_H__

#include "item.h"
#include "production_rule.h"
#include "state.h"
#include "analysis_table_cell.h"
#include "word.h"
#include "symbol.h"

Item* copyItems(State target);

bool isNonTerminal(const string& target);

bool itemRepeat(State state, int pId, int idx);

int* existLeftPro(const string& target);

State CLOSURE(State target);

State GOTO(State state, const string& x);

bool checkTwoState(State s1, State s2);

bool checkItemInCC(State state);

//void printState(State tempState);

void getCanonicalCollection();

int getStateId(State target);

int getVtnId(const string& target);

int getTerminalId(const string& target);

void initAnalysisTable();

bool inFOLLOW(const string& v);

void printAnalysisTable();

void developAnalysisTable();

void printSymbolString(WORD *symbolStack, int stackTop, WORD *symbolNotInStack, int waitIn, int notInTop);

void printStateString(int *stateStack, int stackTop);

void analysis(WORD target[], int wordNum);

void grammaticalAnalysis(WORD* target, int wordNum);

void grammaticalAnalysis_test(WORD* target, int wordNum);

#endif