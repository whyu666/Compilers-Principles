//reference: https://blog.csdn.net/qq_44753451/article/details/109682825

#include <iostream>
#include "LexicalAnalysis.h"
#include "GrammaticalAnalysis.h"
#include "SemanticAnalysis.h"

using namespace std;

int main() {
    //cout << "-------------------- Lexical Analysis ----------------------------" << endl;
    //char test[] = "double x = 5; int b = x + 5; int a = 0; while (a < 3) {int z = a + 1;}#";
    //char test[] = "boolean m = 1; int x = 5; int c = x + m; int a = 0; while(a < 3) {int z = a + 1;}#";
    //char test[] = "boolean x = 1;\n int b = 7 + 5;\n int c = b + 1;\n int a = 0;\n while (a < 3) {{ \n int z = a + 1; \n}#"; //错误数据：两个{{
    //char test[] = "// 这是单行注释\n /* 多行注释\n 多行注释 */ \n boolean x = 1.1.; int b = 7 + 5; int c = b + 1; int a = 0; while (a < 3) {int z = a + 1;}#";  //错误数据：1.1. && boolean型不能为0或1外的数据
    char test[] = "1+1*(2+3)#";
    scan(test);
    //printToken();
    //printSymbolTableL();
    //cout << endl;
    cout << "-------------------- Grammatical Analysis ----------------------------" << endl;
    cout << "-------------------- Semantic Analysis ----------------------------" << endl;
    setSymbolTable(getLSymbolTable(), getLSymbolTableSize());
    grammaticalAnalysis(getToken(), getTop());
    //grammaticalAnalysis_test(getToken(), getTop());
    //printIntermediateCodeLine();
    //printSymbolTable();
    return 0;
}