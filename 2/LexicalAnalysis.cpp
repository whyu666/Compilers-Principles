#include <iostream>
#include <cstring>
#include <utility>
#include "word.h"
#include "symbol.h"
#include "symbol_line.h"

using namespace std;

#define KEYWORD_NUM 8
#define MAX_NUM 200

WORD token[100]; /* NOLINT */
int top = 0;
int line = 1;

SymbolLine symbolTableL[100];
int tableTopL = 0;

char keywords[KEYWORD_NUM][20] = {"if", "else", "main", "while", "int", "return", "double", "boolean"};

WORD* getToken() {
    return token;
}

SymbolLine* getLSymbolTable() {
    return symbolTableL;
}

int getLSymbolTableSize() {
    return tableTopL;
}

void printSymbolTableL() {
    cout << "-----------------------Symbol Table--------------------------" << endl;
    cout << "id    " << "name     " << "addr     " << endl;
    for (int i = 0; i < tableTopL; i++) {
        cout << i << "     " << symbolTableL[i].value << "        " << symbolTableL[i].addr << endl;
    }
}

int getTop() {
    return top;
}

bool digit(char target) {
    if (target >= '0' && target <= '9') {
        return true;
    }
    return false;
}

bool letter(char target) {
    if (target >= 'a' && target <= 'z' || target >= 'A' && target <= 'Z' || target == '_') {
        return true;
    }
    return false;
}

int keyword(char target[]) {
    for (int i = 0; i < KEYWORD_NUM; i++) {
        if (strcmp(target, keywords[i]) == 0) {
            return i + 1;
        }
    }
    return -1;
}

bool point(char target) {
    if (target == '.') {
        return true;
    }
    return false;
}

void deleteBlank(char *target) {
    while (*target == ' ' || *target == 10 || *target == '\n') { //*target == 10：LF（输入的是换行键）
        if(*target == '\n') {
            line++;
        }
        target++;
    }
}

void addNewSymbolLine(const string& target) {
    for (int i = 0; i < tableTopL; i++) {
        if (symbolTableL[i].value == target) {
            return;
        }
    }
    symbolTableL[tableTopL].addr = to_string(tableTopL);
    symbolTableL[tableTopL].type = "variables";
    symbolTableL[tableTopL].value = target;
    symbolTableL[tableTopL].vType = "unknown";
    tableTopL++;
}

void savePosition() { //记录单词位置
    int leftTop;
    if (top > 10) {
        leftTop = 10;
    }
    else {
        leftTop = top;
    }
    token[top - 1].lineNum = line; //记录所在行数
    for (int i = 0; i < leftTop - 1; i++) { //最多记录附近单词
        if (token[top - leftTop + i].lineNum != token[top - 1].lineNum) {
            continue;
        }
        token[top - 1].position = token[top - 1].position + " " + token[top - leftTop + i].extra;
    }
    token[top - 1].position = token[top - 1].position + " <" + token[top - 1].extra + ">"; //记录当前单词
}

void addToken(int code, const string& word) {
    WORD w;
    w.code = code;
    w.key = string(word);
    w.extra = string(word);
    w.lineNum = line;
    token[top++] = w;
    savePosition(); //记录单词位置
}

void addTokenM(int code, string word, char* addr, bool v) {
    WORD w;
    w.code = code;
    w.addr = addr;
    w.key = string(std::move(word));
    w.variable = v;
    w.lineNum = line;
    w.extra = string(addr);
    token[top++] = w;
    savePosition();
}

void addOPToken(int code, const string& word) {
    WORD w;
    w.code = code;
    w.key = string(word);
    w.extra = string(word);
    w.lineNum = line;
    token[top++] = w;
    savePosition();
}

void printToken() {
    for (int i = 0; i < top; i++) {
        cout << "(" << token[i].code << ", " << token[i].key << ", " << token[i].addr << ")" << endl;
    }
}

void scan(char *words) {
    while (true) {
        deleteBlank(words); //删除空格和换行符
        if (*words == '#') { //#：结束，退出循环
            break;
       }
        if (*words == '/') { //处理注释
            if (*(words + 1) == '/') { //处理的是//型注释
                words++;
                words++;
                while (*words != '\n') {
                    words++;
                }
                line++;
                addToken(200, "//");
                words++;
            }
            else if (*(words + 1) == '*') { //处理的是多行注释
                words++;
                words++;
                while (*words != '*' && *(words + 1) != '/') {
                    if (*words == '\n') {
                        line++;
                    }
                    words++;
                }
                addToken(201, "/**/");
                words++;
                words++;
            }
        }
        if (letter(*words)) { //处理字母
            int i = 0;
            char temp[MAX_NUM] = "1";
            while (letter(*words) || digit(*words)) {
                temp[i] = *words;
                words++;
                i++;
            }
            int code = keyword(temp); //判断是否为关键字
            if(code != -1) {
               addToken(code, temp);
            }
            else {
               addTokenM(300, "id", temp, true); //存入变量表
               addNewSymbolLine(string(temp)); //存入符号表
            }
        }
        if (digit(*words)) { //处理数字
            int i = 0;
            char temp[500] = "0";
            bool hasPoint = true;
            while (digit(*words) || point(*words)) {
                if (point(*words)) {
                    if (hasPoint) {
                        hasPoint = false;
                    }
                    else { //存在两个小数点，错误
                        temp[i] = *words;
                        //addTokenM(100, "num", temp, false);
                        addTokenM(100, "id", temp, false);
                        cout << "ERROR: FloatError in line " << token[top - 1].lineNum << ":" << token[top - 1].position << endl;
                        exit(1);
                    }
                }
                temp[i] = *words;
                words++;
                i++;
            }
            if (point(*(words - 1))) { //小数点后无小数部分，错误
                cout << "This is error in syntax " << string(temp) << endl;
                exit(1);
            }
            //addTokenM(100, "num", temp, false);
            addTokenM(100, "id", temp, false);
        }
        else {
            switch (*words) {
                case '+':
                    addOPToken(ADD, "+");
                    break;

                case '-':
                    addOPToken(SUBTRACT, "-");
                    break;

                case '*':
                    addOPToken(MULTIPLY, "*");
                    break;

                case '/':
                    addOPToken(DIVIDE, "/");
                    break;

                case '=':
                    if (*(words + 1) == '=') {
                        addOPToken(EQ_TWO, "==");
                        words++;
                    }
                    else {
                        addOPToken(EQ, "=");
                    }
                    break;

                case '{':
                    addOPToken(L_BRACE, "{");
                    break;

                case '}':
                    addOPToken(R_BRACE, "}");
                    break;

                case '>':
                    if (*(words + 1) == '=') {
                        addOPToken(BIGGER_EQ, ">=");
                        words++;
                    }
                    else {
                        addOPToken(BIGGER, ">");
                    }
                    break;

                case '<':
                    if (*(words + 1) == '=') {
                        addOPToken(SMALLER_EQ, "<=");
                        words++;
                    }
                    else {
                        addOPToken(SMALLER, "<");
                    }
                    break;

                case '(':
                    addOPToken(L_PARENTHESIS, "(");
                    break;

                case ')':
                    addOPToken(R_PARENTHESIS, ")");
                    break;

                case ';':
                    addOPToken(SEMICOLON, ";");
                    break;
            }
            words++;
        }
    }
}