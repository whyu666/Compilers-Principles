#include <iostream>
#include <iomanip>
#include "item.h"
#include "production_rule.h"
#include "state.h"
#include "analysis_table_cell.h"
#include "word.h"
#include "symbol.h"
#include "collection.h"
#include "SemanticAnalysis.h"

using namespace std;

//产生式
Production productions[100]; //产生式集合
int productionTop = 0; //产生式集合最大下标

//非终结符
string vtn[100]; //非终结符集合
int vtnTop = 0; //非终结符集合最大下标

//终结符
string terminal[100]; //非终结符集合
int terminalTop = 0;

//状态
State CC[100]; //项目集规范族 /* NOLINT */
int CCTop = 0; //最大下标
State firstState; //初始状态 /* NOLINT */

//分析表
AnalysisCell actionTable[100][100]; //ACTION表
AnalysisCell gotoTable[100][100]; //GOTO表

//FIRST集
Collection FIRST[100]; //非终结符下标对应的FIRST集 /* NOLINT */
//int firstTop = 0; //数组尾

//FOLLOW集
Collection FOLLOW[100]; // 非终结符下标对应的FOLLOW集 /* NOLINT */
//int followTop = 0; //数组尾

//复制item数组
Item* copyItems (State target) {
    Item *newArr = (Item*)malloc(100 * sizeof(Item));
    for (int i = 0; i < target.top; i++) {
        newArr[i] = target.items[i];
    }
    return newArr;
}

//判断指定字符串是不是终结符
bool isNonTerminal (const string& target) {
    for (int i = 0; i < vtnTop; i++) {
        if (target == vtn[i]) {
            return true;
        }
    }
    return false;
}

//判断一个状态里的item是否重复
bool itemRepeat(State state, int pId, int idx) {
    for (int i = 0; i < state.top; i++) {
        if (state.items[i].pId == pId && state.items[i].idx == idx) {
            return true;
        }
    }
    return false;
}

// 是否存在产生式左侧是指定字符串的产生式并返回其id
int* existLeftPro(const string& target) {
    int* result = (int*)malloc(sizeof(int) * productionTop);
    int idx = 0;
    for (int i = 0; i < productionTop; i++) { //初始化为-1
        result[i] = -1;
    }
    for (int i = 0; i < productionTop; i++) { //获取左侧为指定非终结符的产生式
        if (productions[i].leftPart == target) {
            *(result + (idx++)) = i;
        }
    }
    return result;
}

//获取闭包
State CLOSURE(State target) {
    State newState;
    newState.items = copyItems(target);
    newState.top = target.top;
    //判断是否停止
    Item *itemStack = copyItems(target); //待处理itemStack
    int stackTop = target.top; //设置栈顶
    while (stackTop != 0) {
        stackTop--; //出栈
        int idx = itemStack[stackTop].idx;
        Production tempPro = productions[itemStack[stackTop].pId];
        if (idx < tempPro.rightPartLength) { //判断该item是否还能扩展
            string tempVT = tempPro.rightPart[idx + 1]; //点移到右边的第一个字符
            if (isNonTerminal(tempVT)) { //如果是非终结符
                int* pId = existLeftPro(tempVT);
                for (int i = 0; i < productionTop; i++) {
                    int id = *(pId + i);
                    if (id != -1 && !itemRepeat(newState, id, -1)) {
                        //添加新的item，点标在最左边
                        newState.items[newState.top].pId = id;
                        newState.items[newState.top].idx = -1;
                        //入栈
                        itemStack[stackTop++] = newState.items[newState.top];
                        newState.top++;
                    }
                }
            }
        }
    }
    return newState;
}

State GOTO(State state, const string& x) {
    State j;
    j.top = 0;
    for (int i = 0; i < state.top; i++) {
        int pId = state.items[i].pId;
        int idx = state.items[i].idx;
        if (productions[pId].rightPart[idx + 1] == x) { //如果下一个符号是目标符号
            j.items[j.top].idx = idx + 1;
            j.items[j.top].pId = pId;
            j.top++;
        }
    }
    return CLOSURE(j);
}

//检查两个状态是否一致
bool checkTwoState(State s1, State s2) {
    if (s1.top != s2.top) { //item个数不一样，一定不一致
        return false;
    }
    for (int i = 0; i < s1.top; i++) { //判断item是否一致
        if (!itemRepeat(s2, s1.items[i].pId, s1.items[i].idx)) {
            return false;
        }
    }
    return true;
}

//检查项目集规范族是否已经有一个状态
bool checkItemInCC(State state) {
    for (int i = 0; i < CCTop; i++) {
        if (checkTwoState(CC[i], state)) {
            return true;
        }
    }
    return false;
}

/*
//打印状态项目
void printState(State tempState) {
    for (int j = 0; j < tempState.top; j++) {
            Item tempItem = tempState.items[j];
            Production tempPro = productions[tempItem.pId];
            cout << tempPro.leftPart << "-->";
            for (int k = 0; k <= tempPro.rightPartLength + 1; k++) {
                if (tempItem.idx == k - 1) {
                    cout << ".";
                }
                if (k != tempPro.rightPartLength + 1) {
                    cout << tempPro.rightPart[k];
                }
            }
            cout << endl;
        }
        cout << "--------------------------------" << endl;
}
*/

//获取项目集规范族
void getCanonicalCollection() {
    //获取拓广文法新加产生式的项目（如：S'->.S）
    firstState.items[0].pId = 0;
    firstState.items[0].idx = -1;
    firstState.top = 1;
    CC[CCTop++] = CLOSURE(firstState); //获取该项目的闭包作为初始状态
    State stateStack[MAX];
    stateStack[0] = CC[CCTop - 1];
    int stackTop = 1;
    while (stackTop != 0) {
        stackTop--;
        State topState = stateStack[stackTop];
        for (int i = 0; i < vtnTop; i++) { //对非终结符
            State temp = GOTO(topState, vtn[i]);
            if (temp.top !=0 && !checkItemInCC(temp)) { //一个状态不能为空，如当GOTO(i, "#")时
                stateStack[stackTop++] = temp;
                CC[CCTop++] = temp;
            }
        }
        for (int i = 0; i < terminalTop; i++) { //对终结符
            State temp = GOTO(topState, terminal[i]);
            if (temp.top !=0 && !checkItemInCC(temp)) {
                stateStack[stackTop++] = temp;
                CC[CCTop++] = temp;
            }
        }
    }
}

//获取一个状态的序号
int getStateId(State target) {
    for (int i = 0; i < CCTop; i++) {
        if (checkTwoState(CC[i], target)) {
            return i;
        }
    }
    cerr << "ERROR: get state id error!" << endl;
    return -1;
}

// 获取一个非终结符id
int getVtnId(const string& target) {
    for (int i = 0; i < vtnTop; i++) {
        if (target == vtn[i]) {
            return i;
        }
    }
    return -1;
}

//获取一个终结符id
int getTerminalId(const string& target) {
    for (int i = 0; i < terminalTop; i++) {
        if (target == terminal[i]) {
            return i;
        }
    }
    return -1;
}

//初始化分析表
void initAnalysisTable() {
    //GOTO表
    for (int i = 0; i < CCTop; i++) {
        for (int j = 0; j < vtnTop; j++) {
            gotoTable[i][j].direct = -1;
            gotoTable[i][j].op = "*";
        }
    }
    //ACTION表
    for (int i = 0; i < CCTop; i++) {
        for (int j = 0; j < terminalTop; j++) {
            actionTable[i][j].direct = -1;
            actionTable[i][j].op = "*";
        }
    }
}

//检查FIRST或FOLLOW集中是否有一个字符
bool checkCollection(Collection cl, int vId) {
    for (int i = 0; i < cl.top; i++) {
        if (cl.vts[i] == vId) {
            return true;
        }
    }
    return false;
}

//将一个集合加到另一个中，参数follow是指是否加入到FOLLOW集中
void addCollection(int id, Collection target, bool follow) {
    if(follow) { //加入到FOLLOW集
        for (int i = 0; i < target.top; i++) {
            if (!checkCollection(FOLLOW[id], target.vts[i])) {
                FOLLOW[id].vts[FOLLOW[id].top++] = target.vts[i];
            }
        }
    }
    else { //加入到FIRST集
        for (int i = 0; i < target.top; i++) {
            if (!checkCollection(FIRST[id], target.vts[i])) {
                FIRST[id].vts[FIRST[id].top++] = target.vts[i];
            }
        }
    }
}

//获取并产生某一非终结符的FIRST集
Collection getFIRST(int vId) {
    if (FIRST[vId].top != 0) { //如果该FIRST集存在，直接返回
        return FIRST[vId];
    }
    //计算FIRST集
    for (int i = 0; i < productionTop; i++) {
        if (productions[i].leftPart == vtn[vId]) {
            if (productions[i].rightPart[0] == vtn[vId]) { //如果产生式存在左递归，要跳过，否则就是将FIRST[X]放到FIRST[X]中，会死循环
                continue;
            }
            int id = getTerminalId(productions[i].rightPart[0]);
            if (id != -1) { //如果产生式右部第一个符号是终结符，则把该终结符加入
                if (!checkCollection(FIRST[vId], id)) { //FIRST集中无id则将id加入其中
                    FIRST[vId].vts[FIRST[vId].top++] = id;
                }
            }
            else { //如果是非终结符
                id = getVtnId(productions[i].rightPart[0]);
                if (id == -1) {
                    cerr << "ERROR: can not find symbol [" << productions[i].rightPart[0] << "]" << endl;
                    return FIRST[vId];
                }
                addCollection(vId, getFIRST(id), false); //产生式右部第一个符号是非终结符，将它的FIRST集加入
            }
        }
    }
    return FIRST[vId];
}

//初始化FIRST集
void initFIRST() {
    for (int i = 0; i < vtnTop; i++) { //为每个非终结符计算FIRST集
        getFIRST(i);
    }
}

//获取FOLLOW集
Collection getFOLLOW(int vId) {
    if (FOLLOW[vId].top != 0) { //如果该FOLLOW集存在，直接返回
        return FOLLOW[vId];
    }
    if (vId == 0) {
        FOLLOW[0].vts[FOLLOW[0].top++] = getTerminalId("#"); //增广文法新加的产生式的右侧自带#，如 S' -> S 中的 S
    }
    //构建FOLLOW集
    for (int i = 1; i < productionTop; i++) {
        for (int j = 0; j <= productions[i].rightPartLength; j++) {
            if (vtn[vId] == productions[i].rightPart[j]) {
                int id;
                if (j == productions[i].rightPartLength) { //如果该非终结符在产生式右侧最后
                    if (vtn[vId] == productions[i].leftPart) { //右侧最后一个符号不能是自己，否则会死循环
                        continue;
                    }
                    id = getVtnId(productions[i].leftPart);
                    if (id == -1) {
                        cerr << "ERROR: unknown non-terminal v: [" << productions[i].leftPart << "]" << endl;
                        return FOLLOW[vId];
                    }
                    addCollection(vId, getFOLLOW(id), true); //把产生式左侧非终结符的FOLLOW集加入
                }
                else if ((id = getTerminalId(productions[i].rightPart[j + 1])) != -1) {
                    if (!checkCollection(FOLLOW[vId], id)) { //如果该非终结符的下一个符号是终结符，直接加到FOLLOW集中
                        FOLLOW[vId].vts[FOLLOW[vId].top++] = id;
                    }
                }
                else if ((id = getVtnId(productions[i].rightPart[j + 1])) != -1) {
                    addCollection(vId, getFIRST(id), true); //如果该非终结符下一个符号是另一个非终结符，就把该非终结符的FIRST集加入
                }
            }
        }
    }
    return FOLLOW[vId];
}

void initFOLLOW() { //初始化FOLLOW集
    for (int i = 0; i < vtnTop; i++) { //为每个非终结符计算FOLLOW集
        getFOLLOW(i);
    }
}

//判断FOLLOW集中是否存在某个元素
bool inFOLLOW(const string& v, int target) {
    int vId = getVtnId(v); //获取非终结符id
    if(vId == -1) {
        cerr << "ERROR: get non-terminal v error" << endl;
        return false;
    }
    Collection flc = FOLLOW[vId]; //获取FOLLOW集
    for (int i = 0; i < flc.top; i++) { //判断对应终结符是否在非终结符v的FOLLOW集中
        if (target == flc.vts[i]) {
            return true;
        }
    }
    return false;
}

//打印分析表
void printAnalysisTable() {
    cout << std::setw(10) << "";
    for (int i = 0; i < terminalTop; i++) { //打印终结符
        cout << std::setw(10) << terminal[i];
    }
    for (int i = 0; i < vtnTop; i++) { //打印非终结符
        cout << std::setw(10) << vtn[i];
    }
    cout << endl;
    for (int i = 0; i < CCTop; i++) { //打印每个状态
        cout << std::left << std::setw(10) << i;
        for (int j = 0; j < terminalTop; j++) {
            if (actionTable[i][j].direct == -1) {
                cout << std::left << std::setw(10) << actionTable[i][j].op;
            }
            else {
                string display = actionTable[i][j].op + to_string(actionTable[i][j].direct);
                cout << std::left << std::setw(10) << display;
            }
        }
        for (int j = 0; j < vtnTop; j++) {
            if (gotoTable[i][j].direct == -1) {
                cout << std::left << std::setw(10) << gotoTable[i][j].op;
            }
            else {
                string display = gotoTable[i][j].op + to_string(gotoTable[i][j].direct);
                cout << std::left << std::setw(10) << display;
            }
        }
        cout << endl;
    }
}

//构造分析表
void developAnalysisTable() {
    for (int i = 0; i < CCTop; i++) { //遍历状态
        State tempState = CC[i]; //当前状态
        for (int j = 0; j < tempState.top; j++) { //遍历状态中的项目
            Item tempItem = tempState.items[j]; //当前项目
            //构建ACTION和GOTO
            if (tempItem.idx < productions[tempItem.pId].rightPartLength) {
                string tempEle = productions[tempItem.pId].rightPart[tempItem.idx + 1];
                int eleId = getTerminalId(tempEle); //终结符
                if (eleId == -1) { //如果不是终结符，填GOTO表
                    eleId = getVtnId(tempEle);
                    if (eleId == -1) {
                        cerr << "ERROR: cannot recognize the symbol [" << tempEle << "]" << endl;
                        return;
                    }
                    State nextState = GOTO(tempState, tempEle); //获取下一个状态
                    int nextStateId = getStateId(nextState); //获取状态id
                    if (nextStateId == -1) {
                        cerr << "ERROR: cannot get state id" << endl;
                        return;
                    }
                    //填GOTO表
                    gotoTable[i][eleId].direct = nextStateId;
                    gotoTable[i][eleId].op = "gt";
                }
                else { //是终结符，填ACTION表
                    State nextState = GOTO(tempState, tempEle); //获取下一个状态
                    int nextStateId = getStateId(nextState); //获取状态id
                    if (nextStateId == -1) {
                        cerr << "ERROR: cannot get state id" << endl;
                        return;
                    }
                    //填ACTION表
                    actionTable[i][eleId].direct = nextStateId;
                    actionTable[i][eleId].op = "s";
                }
            }
            else {
                if (tempItem.pId == 0 && tempItem.idx == 0) { //标记acc
                    actionTable[i][getTerminalId("#")].op = "acc";
                }
                else { //归约
                    for (int m = 0; m < terminalTop; m++) { //这里是是否是SLR的关键，SLR需要判断下一个非终结符是否在该变量的FOLLOW集中，LR则不需要，直接true即可
                        if (inFOLLOW(productions[tempItem.pId].leftPart, m)) {
                            actionTable[i][getTerminalId(terminal[m])].op = "r";
                            actionTable[i][getTerminalId(terminal[m])].direct = tempItem.pId;
                        }
                    }
                }
            }
        }
    }
}

//打印栈内外字符串
void printSymbolString(WORD *symbolStack, int stackTop, WORD *symbolNotInStack, int waitIn, int notInTop) {
    string in;
    for (int i = 0; i < stackTop; i++) {
        in.append("{");
        in.append(symbolStack[i].key);
        in.append("}");
    }
    string out;
    for (int i = waitIn; i < notInTop; i++) {
        out.append("{");
        out.append(symbolNotInStack[i].key);
        out.append("}");
    }
    cout << "symbol stack: " << endl;
    cout << std::left << std::setw(50) << in;
    cout << std::setw(50) << out;
    cout << endl;
}

//打印状态栈
void printStateString(int *stateStack, int stackTop) {
    string states;
    for (int i = 0; i < stackTop; i++) {
        states.append("{");
        states.append(to_string(stateStack[i]));
        states.append("}");
    }
    cout << "state stack:" << endl;
    cout << std::left << std::setw(50) << states;
    cout << endl;
}

//语法分析
void analysis(WORD target[], int wordNum) {
    WORD symbolStack[MAX]; //符号栈
    int symbolStackTop = 0; //符号栈顶
    int stateStack[MAX]; //状态栈
    int stateStackTop = 0; //状态栈顶
    //最后是#
    target[wordNum].key = "#";
    target[wordNum].code = getTerminalId("#");
    wordNum++;
    //将状态0和#入栈
    symbolStack[symbolStackTop].code = START_END;
    symbolStack[symbolStackTop].key = "#";
    symbolStackTop++;
    stateStack[stateStackTop++] = 0;
    //进行归约
    int wait = 0;
    cout << "------------------State Analysis-------------------" << endl;
    printStateString(stateStack, stateStackTop);
    printSymbolString(symbolStack, symbolStackTop, target, wait, wordNum);
    cout << "---------------------------------------------" << endl;
    while(true) {
        WORD tempWord = target[wait++]; //获取要读取的符号
        int id = getTerminalId(tempWord.key);
        int currentState = stateStack[stateStackTop - 1];
        if (id == -1) { //不是终结符
            id = getVtnId(tempWord.key);
            if (id == -1) {
                cerr << "ERROR: cannot recognize the symbol [" << tempWord.key << "]" << endl;
            }
            AnalysisCell cell = gotoTable[currentState][id];
            if (cell.op == "*") {
                cerr << "ERROR: analysis error in GOTO table (" << currentState << "," << id << ")" << endl;
                return;
            }
            cout << "GOTO(" << currentState << ", " << tempWord.key << ") = " << cell.direct << endl;
            stateStack[stateStackTop++] = cell.direct; //将状态压入栈
            symbolStack[symbolStackTop++] = tempWord; //将符号压入栈
        }
        else { //是终结符
            AnalysisCell cell = actionTable[currentState][id];
            if (cell.op == "acc") {
                symbolStack[symbolStackTop].key = "#";
                symbolStack[symbolStackTop].code = getTerminalId("#");
                symbolStackTop++;
                cout << "---------------------------------------------" << endl;
                printStateString(stateStack, stateStackTop);
                printSymbolString(symbolStack, symbolStackTop, target, wait, wordNum);
                cout << "---------------------------------------------" << endl;
                cout << "analysis finish!" << endl;
                return;
            }
            if (cell.op == "*") {
                cerr << "ERROR: analysis error in action table (" << currentState << "," << id << ")" << endl;
                cerr << "throw error at line " << tempWord.lineNum << ":" << tempWord.position << endl;
                exit(1);
            }
            if (cell.op == "s") { //移入
                cout << "ACTION(" << currentState << ", " << tempWord.key << ") = " << cell.op << to_string(cell.direct) << endl;
                stateStack[stateStackTop++] = cell.direct;
                symbolStack[symbolStackTop++] = tempWord;
            }
            if (cell.op == "r") { //归约
                Production tempProduction = productions[cell.direct]; //获取产生式
                cout << "ACTION(" << currentState << ", " << tempWord.key << ") = " << cell.op << to_string(cell.direct) << endl;
                cout << "use production: {" << tempProduction.leftPart << " -> ";
                for (int k = 0; k <= tempProduction.rightPartLength; k++) {
                    cout << tempProduction.rightPart[k];
                }
                cout << "} to reduce" << endl;
                wait = wait - 2; //将待识别的符号改成产生式左部
                target[wait] = createCode(cell.direct, tempProduction.leftPart, symbolStack, symbolStackTop);
                //符号栈和状态栈弹出产生式右部长度个元素
                stateStackTop = stateStackTop - tempProduction.rightPartLength - 1;
                symbolStackTop = symbolStackTop - tempProduction.rightPartLength - 1;
            }
        }
        //打印分析过程
        printStateString(stateStack, stateStackTop);
        printSymbolString(symbolStack, symbolStackTop, target, wait, wordNum);
        cout << "---------------------------------------------" << endl;
    }
}

//打印FIRST集
void printFIRST() {
    cout << "----------------------- FIRST --------------------------" << endl;
    for (int i = 0; i < vtnTop; i++) {
        cout << std::left << std::setw(7) << "{" + vtn[i] + "}" + "'s first collection: ";
        for (int j = 0; j < FIRST[i].top; j++) {
            cout << "{" + terminal[FIRST[i].vts[j]] + "}";
        }
        cout << endl;
    }
}

//打印FOLLOW集
void printFOLLOW() {
    cout << "----------------------- FOLLOW --------------------------" << endl;
    for (int i = 0; i < vtnTop; i++) {
        cout << std::left << std::setw(7) << "{" + vtn[i] + "}" + "'s follow collection: ";
        for (int j = 0; j < FOLLOW[i].top; j++) {
            cout << "{" + terminal[FOLLOW[i].vts[j]] + "}";
        }
        cout << endl;
    }
}

//语法分析，target是待处理符号串，wordNum是target的符号个数
void grammaticalAnalysis(WORD* target, int wordNum) {
    //设定产生式
    productions[productionTop].leftPart = "E'";
    productions[productionTop].rightPart[0] = "E";
    productions[productionTop].rightPartLength = 0;
    productionTop++;

    productions[productionTop].leftPart = "E";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "+";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "T";
    productionTop++;

    productions[productionTop].leftPart = "E";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "T";
    productionTop++;

    productions[productionTop].leftPart = "T";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "T";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "*";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "F";
    productionTop++;

    productions[productionTop].leftPart = "T";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "F";
    productionTop++;

    productions[productionTop].leftPart = "F";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "(";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = ")";
    productionTop++;

    productions[productionTop].leftPart = "F";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "id";
    productionTop++;

    cout << "----------------Produce Formula-----------------" << endl;

    for (int i = 0; i < productionTop; i++) {
        cout << i << ". " << productions[i].leftPart << " -> ";
        for (int j = 0; j <= productions[i].rightPartLength; j++) {
            cout << productions[i].rightPart[j];
        }
        cout << endl;
    }

    //设定非终结符，第一个非终结符一定要是扩广文法加的那个产生式的右部，比如 S' -> S 的 S
    vtn[vtnTop++] = "E";
    vtn[vtnTop++] = "T";
    vtn[vtnTop++] = "F";

    //设定终结符
    terminal[terminalTop++] = "id";
    terminal[terminalTop++] = "#";
    terminal[terminalTop++] = "(";
    terminal[terminalTop++] = ")";
    terminal[terminalTop++] = "+";
    terminal[terminalTop++] = "*";

    //生成FIRST集
    initFIRST();
    printFIRST();

    // 生成FOLLOW集
    initFOLLOW();
    printFOLLOW();

    // 获取规范族
    getCanonicalCollection();

    // 初始化分析表
    initAnalysisTable();

    // 构造分析表
    developAnalysisTable();

    cout << "---------------Analysis Table------------------" << endl;
    printAnalysisTable();

    // 分析
    analysis(target, wordNum);
}

void grammaticalAnalysis_test(WORD* target, int wordNum) {
    productions[productionTop].leftPart = "S'";
    productions[productionTop].rightPart[0] = "S";
    productions[productionTop].rightPartLength = 0;
    productionTop++;

    productions[productionTop].leftPart = "S";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "W";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "(";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "C";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = ")";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "{";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "S";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "}";
    productionTop++;

    productions[productionTop].leftPart = "W";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "if";
    productionTop++;

    productions[productionTop].leftPart = "S";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "S";
    productionTop++;

    productions[productionTop].leftPart = "S";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productionTop++;

    productions[productionTop].leftPart = "S";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = ";";
    productionTop++;

    productions[productionTop].leftPart = "W";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "while";
    productionTop++;

    productions[productionTop].leftPart = "E";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "id";
    productionTop++;

    productions[productionTop].leftPart = "E";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "num";
    productionTop++;

    productions[productionTop].leftPart = "C";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "U";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productionTop++;

    productions[productionTop].leftPart = "U";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "==";
    productionTop++;

    productions[productionTop].leftPart = "U";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "<";
    productionTop++;

    productions[productionTop].leftPart = "U";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = ">";
    productionTop++;

    productions[productionTop].leftPart = "E";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "O";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productionTop++;

    productions[productionTop].leftPart = "O";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "+";
    productionTop++;

    productions[productionTop].leftPart = "O";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "-";
    productionTop++;

    productions[productionTop].leftPart = "O";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "*";
    productionTop++;

    productions[productionTop].leftPart = "O";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "/";
    productionTop++;

    productions[productionTop].leftPart = "O";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "=";
    productionTop++;

    productions[productionTop].leftPart = "E";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "T";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "=";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "E";
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = ";";
    productionTop++;

    productions[productionTop].leftPart = "T";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "int";
    productionTop++;

    productions[productionTop].leftPart = "T";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "double";
    productionTop++;

    productions[productionTop].leftPart = "T";
    productions[productionTop].rightPartLength = -1;
    productions[productionTop].rightPart[++productions[productionTop].rightPartLength] = "boolean";
    productionTop++;

    vtn[vtnTop++] = "S";
    vtn[vtnTop++] = "C";
    vtn[vtnTop++] = "W";
    vtn[vtnTop++] = "O";
    vtn[vtnTop++] = "U";
    vtn[vtnTop++] = "E";
    vtn[vtnTop++] = "T";

    terminal[terminalTop++] = "id";
    terminal[terminalTop++] = "num";
    terminal[terminalTop++] = "==";
    terminal[terminalTop++] = ">";
    terminal[terminalTop++] = "<";
    terminal[terminalTop++] = "=";
    terminal[terminalTop++] = "+";
    terminal[terminalTop++] = "-";
    terminal[terminalTop++] = "*";
    terminal[terminalTop++] = "/";
    terminal[terminalTop++] = "if";
    terminal[terminalTop++] = "while";
    terminal[terminalTop++] = "int";
    terminal[terminalTop++] = "double";
    terminal[terminalTop++] = "boolean";
    terminal[terminalTop++] = ";";
    terminal[terminalTop++] = "{";
    terminal[terminalTop++] = "}";
    terminal[terminalTop++] = "(";
    terminal[terminalTop++] = ")";
    terminal[terminalTop++] = "#";

    //生成FIRST集
    initFIRST();
    printFIRST();

    // 生成FOLLOW集
    initFOLLOW();
    printFOLLOW();
    getCanonicalCollection();
    initAnalysisTable();
    developAnalysisTable();

    cout << "---------------Analysis Table------------------" << endl;
    printAnalysisTable();
    analysis(target, wordNum);
}