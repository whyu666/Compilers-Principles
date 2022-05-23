#include <iostream>
#include <utility>
#include "word.h"
#include "intermediate_code.h"
#include "symbol_line.h"

int tempNum = 0; //临时变量的序号
int lineNum = 100; //中间代码序号

//中间代码
IntermediateCodeLine lines[1000]; //中间代码行数组
int lineTop = 0; //数组尾

//符号表
SymbolLine symbolTable[1000]; //符号表
int tableTop = 0; //符号表尾

//词法分析获取的符号表
SymbolLine *symbolTableFromL; //符号表
int tableTopFromL = 0; //符号表尾

//循环栈，栈顶记录的是最内层的循环开始时的中间代码在数组中的下标
int loopStack[100]; //循环栈
int loopStackTop = 0; //循环对应的中间代码下标

//设置词法分析获取的符号表
void setSymbolTable(SymbolLine *temp, int top) {
    symbolTableFromL = temp;
    tableTopFromL = top;
}

//获取一个临时变量
string getTempV() {
    return "sys_temp" + to_string(tempNum++);
}

//获取一个符号在符号表中的下标
int getSymbolIdx(const string& sl) {
    for (int i = 0; i < tableTop; i++) {
        if (symbolTable[i].addr == sl) {
            return i;
        }
    }
    return -1;
}

//检查类型兼容性，并返回更高级的类型
string checkType(const string& addr1, const string& addr2) {
    //默认是int类型
    string type1 = "int";
    string type2 = "int";
    //根据变量名addr1和addr2从符号表查对应变量的类型，如果不在符号表中，则是数字，默认初始化的int
    int id1 = getSymbolIdx(addr1);
    if (id1 != -1) {
        type1 = symbolTable[id1].type;
    }
    int id2 = getSymbolIdx(addr2);
    if (id2 != -1) {
        type2 = symbolTable[id2].type;
    }
    if (type1 == "boolean" || type2 == "boolean") { //如果有一个是bool，则无法运算
        cerr << "ERROR: throw error for " << addr1 << "(" << type1 << ") cant compute with " << addr2 << "(" << type2 << ")..." << endl;
        exit(1);
    }
    if (type1 == "double" || type2 == "double") { //double和int进行计算，都转换成double类型
        return "double";
    }
    else {
        return "int";
    }
}

//产生式18对应的语义子程序
void SemanticSubroutine_18(const WORD& target) {

}

//产生式13对应的语义子程序
WORD SemanticSubroutine_13(const WORD& v1, const WORD& op, const WORD& v2, string leftPart) {
    string type = checkType(v1.addr, v2.addr); //类型检查
    WORD result;
    //获取临时变量
    result.addr = getTempV();
    result.key = std::move(leftPart);
    //进行运算检测（参与运算的数字）
    int v1_d, v2_d;
    //判断v1是否为符号表中的变量，是则从符号表中取值
    int id = getSymbolIdx(v1.addr);
    if (id == -1) {
        v1_d = stoi(v1.addr);
    }
    else {
        v1_d = stoi(symbolTable[id].value);
    }
    //判断v2是否为符号表中的变量，是则从符号表中取值
    id = getSymbolIdx(v2.addr);
    if (id == -1) {
        v2_d = stoi(v2.addr);
    }
    else {
        v2_d = stoi(symbolTable[id].value);
    }
    //进行符号运算（在extra中保存运算符号）
    if (op.extra == "+") {
        lines[lineTop].code = result.addr + " = " + v1.addr + " + " + v2.addr;
        lines[lineTop].lineNum = lineNum;
        lineTop++;
        lineNum++;
        result.variable = true;
        //临时变量加入符号表
        symbolTable[tableTop].addr = result.addr;
        symbolTable[tableTop].type = type;
        symbolTable[tableTop].value = std::to_string(v1_d + v2_d);
        tableTop++;
    }
    if (op.extra == "-") {
        lines[lineTop].code = result.addr + " = " + v1.addr + " - " + v2.addr;
        lines[lineTop].lineNum = lineNum;
        lineTop++;
        lineNum++;
        result.variable = true;
        //临时变量加入符号表
        symbolTable[tableTop].addr = result.addr;
        symbolTable[tableTop].type = type;
        symbolTable[tableTop].value = std::to_string(v1_d - v2_d);
        tableTop++;
    }
    if (op.extra == "*") {
        lines[lineTop].code = result.addr + " = " + v1.addr + " * " + v2.addr;
        lines[lineTop].lineNum = lineNum;
        lineTop++;
        lineNum++;
        result.variable = true;
        //临时变量加入符号表
        symbolTable[tableTop].addr = result.addr;
        symbolTable[tableTop].type = type;
        symbolTable[tableTop].value = std::to_string(v1_d * v2_d);
        tableTop++;
    }
    if (op.extra == "/") {
        lines[lineTop].code = result.addr + " = " + v1.addr + " / " + v2.addr;
        lines[lineTop].lineNum = lineNum;
        lineTop++;
        lineNum++;
        result.variable = true;
        //临时变量加入符号表
        symbolTable[tableTop].addr = result.addr;
        symbolTable[tableTop].type = type;
        symbolTable[tableTop].value = std::to_string(v1_d / v2_d);
        tableTop++;
    }
    return result;
}

//产生式19对应的语义子程序
void SemanticSubroutine_19(const WORD& v1, const WORD& v2, const WORD& type) {
    if (-1 != getSymbolIdx(v1.addr)) { //判断变量是否重复
        cerr << "throw error for multi declare of variable [" << v1.addr << "]..." << endl;
        exit(1);
    }
    //将变量添加到符号表
    symbolTable[tableTop].addr = v1.addr;
    //变量类型（int、double、boolean）保存在extra中
    symbolTable[tableTop].type = type.extra;
    //变量赋值
    string value;
    int id = getSymbolIdx(v2.addr);
    if (id == -1) {
        value = v2.addr;
    }
    else {
        value = symbolTable[id].value;
    }
    symbolTable[tableTop].value = value;
    tableTop++;
    //处理词法分析传进的符号表
    for (int i = 0; i < tableTopFromL; i++) {
        if (symbolTableFromL[i].value == v1.addr) {
            symbolTableFromL[i].vType = type.extra;
        }
    }
    //中间代码
    lines[lineTop].code = v1.addr + " = " + v2.addr;
    lines[lineTop].lineNum = lineNum;
    lineTop++;
    lineNum++;
}

//产生式6对应的语义子程序
void SemanticSubroutine_6() {
    //空出一行,等到循环体整体归约的时候补上
    lines[lineTop].code = "blank";
    lines[lineTop].lineNum = lineNum;
    //记录最内层循环开始的行数
    loopStack[loopStackTop] = lineTop;
    loopStackTop++;
    lineNum++;
    lineTop++;
}

//产生式1对应的语义子程序
void SemanticSubroutine_1(const WORD& t) {
    int startLine = loopStack[--loopStackTop];//找到最内层循环开始的行数
    lines[startLine].code = "if not " + t.extra + " then goto " + to_string(lineNum + 1); //补上空出来的条件跳转语句
    //产生跳回到while判断条件的中间代码
    lines[lineTop].code = "goto " + to_string(lines[startLine].lineNum);
    lines[lineTop].lineNum = lineNum;
    lineTop++;
    lineNum++;
}

//产生式9对应的语义子程序
WORD SemanticSubroutine_9(const WORD& v1, const WORD& op, const WORD& v2, string leftPart) {
    checkType(v1.addr, v2.addr); //类型检查
    WORD result;
    result.key = std::move(leftPart);
    //记录判断条件，补上空出来的条件跳转语句的时候会用到
    result.extra = v1.addr + " " + op.extra + " " + v2.addr;
    return result;
}

//语义分析，产生中间代码
WORD createCode(int productionId, const string& leftPart, WORD* wordsInStack, int top) {
    //封装返回的字符
    WORD result;
    result.key = leftPart;
    result.addr = wordsInStack[top - 1].addr;
    result.extra = wordsInStack[top - 1].extra;
    //执行语义子程序
    if (productionId == 18) {
        SemanticSubroutine_18(wordsInStack[top - 1]);
    }
    if (productionId == 13) {
        result = SemanticSubroutine_13(wordsInStack[top - 3], wordsInStack[top - 2], wordsInStack[top - 1], leftPart);
    }
    if (productionId == 19) {
        SemanticSubroutine_19(wordsInStack[top - 4], wordsInStack[top - 2], wordsInStack[top - 5]);
    }
    if (productionId == 6) {
        SemanticSubroutine_6();
    }
    if (productionId == 1) {
        SemanticSubroutine_1(wordsInStack[top - 5]);
    }
    if (productionId == 9) {
        result = SemanticSubroutine_9(wordsInStack[top - 3], wordsInStack[top - 2], wordsInStack[top - 1], leftPart);
    }
    return result;
}

//打印中间代码
void printIntermediateCodeLine() {
    cout << "------------------Middle Code--------------------" << endl;
    for (int i = 0; i < lineTop; i++) {
        cout << lines[i].lineNum << " : " << lines[i].code << endl;
    }
}

//打印符号表
void printSymbolTable() {
    cout << "-----------------------Symbol Table--------------------------" << endl;
    cout << "id    " << "name     " << "addr     " << "type" << endl;
    for (int i = 0; i < tableTopFromL; i++) {
        cout << i << "     " << symbolTableFromL[i].value << "        " << symbolTableFromL[i].addr << "        " << symbolTableFromL[i].vType << endl;
    }
}