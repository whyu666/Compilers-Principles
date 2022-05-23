#ifndef __LEXICALANALYSIS_H__
#define __LEXICALANALYSIS_H__

#include "word.h"
#include "symbol_line.h"

WORD* getToken();

int getTop();

bool digit(char target);

bool letter(char target);

int keyword(char target[]);

bool point(char target);

void deleteBlank(char *target);

void addToken(int code, const string& word);

void addTokenM(int code, string word, char* addr, bool v);

void addOPToken(int code, const string& word);

void printToken();

void scan(char *words);

void printSymbolTableL();

SymbolLine* getLSymbolTable();

int getLSymbolTableSize();

#endif