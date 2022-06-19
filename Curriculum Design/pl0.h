#ifndef PL0_H
#define PL0_H

#include <stdio.h>
FILE* infile;

#define NUM_RESERVED    21      //保留字的数量（精确值，用于查找）
#define MAX_TX          2000    //tx（table表格）的最大长度
#define MAX_NUM         14      //最大数字位数
#define MAX_SYM         17      //数组ssym和csym的最大符号数量
#define MAX_ID          10      //变量名的最大长度
#define MAX_ADDRESS     32767   //最大地址
#define MAX_LEVEL       32      //块的最大深度
#define MAX_CX          2000    //cx（存储指令数量）的最大值
#define SIZE_STACK      4000    //栈的最大值
#define MAX_DIM         10	    //数组维数的最大值
#define MAX_INS         15      //指令数量的最大值

enum symtype {
	SYM_NULL, SYM_IDENTIFIER, SYM_NUMBER, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH, SYM_ODD, SYM_EQU, SYM_NEQ,
	SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_LPAREN, SYM_RPAREN, SYM_COMMA, SYM_SEMICOLON,SYM_PERIOD, SYM_BECOMES,
    SYM_BEGIN, SYM_END, SYM_IF, SYM_THEN, SYM_WHILE, SYM_DO, SYM_CALL, SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_AND,
	SYM_OR, SYM_NOT, SYM_ELSE, SYM_FOR, SYM_BREAK, SYM_EXIT, SYM_LSPAREN, SYM_RSPAREN, SYM_ARRAY, SYM_READ, SYM_WRITE,
    SYM_MOD, SYM_ADDPLUS, SYM_SUBPLUS, SYM_BOOL
};

enum idtype	{
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE, ID_BOOL
};

/*
 *JPNC:满足条件则跳转
 *LDA:加载数组值到栈顶
 *STA:存储数组值到偏移地址上
 *RDA:读取数组
 *WTA:打印数组
 *REA:读取变量值
 *WRT:打印变量值
 */
enum opcode {
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC, JPNC, LDA, STA, RDA, WTA, REA, WRT
};

/*
 *OPR_NOT:对栈顶值取非
 *OPR_WEN:打印空格指令
 *OPR_WRT:打印栈顶常数指令
 */
enum oprcode {
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN, OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR, OPR_GEQ, OPR_NOT, OPR_WEN, OPR_WRT, OPR_MOD
};

typedef struct {
	int f; //操作码
	int l; //层次差
	int a; //多用途（位移地址）
} instruction;

//错误代码
char* err_msg[] = {
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the Statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an Expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a Factor.",
/* 24 */    "The symbol can not be as the beginning of an Expression.",
/* 25 */    "The number is too great.",
/* 26 */    "expected ')' or '('",
/* 27 */    "can't break",
/* 28 */    "Missing ']'.",
/* 29 */    "",
/* 30 */    "",
/* 31 */    "",
/* 32 */    "There are too many levels."
};

unsigned char ch;       //最后读到的字符
int  sym;               //最后读到的符号
char id[MAX_ID + 1];    //最后读到的标识符
int  num;               //最后读到的数字
int  cc;                //字符计数
int  ll;                //行长度
int  err;               //记录错误数量
int  cx = 0;            //指令数组的指针
int  level = 0;         //记录当前嵌套层数
int  tx = 0;		    //符号表（table）指针
int  ax = 0;		    //数组符号表索引

/*
	注：在一个语句可能嵌套多个条件语句 比如 for（；；） begin if（）then Statement end；
	这样for的条件，可能会与if的条件相重合，故真、假值链用多维数组，以避免回填时跳到错误指令
	用 conditionLevel 记录当前条件嵌套层数
	trueCount[]、falseCount[]表示当前层要跳到真、假出口的指令数目
*/
int true[4][10] = { 0 };      //真值链（短路计算）
int false[4][10] = { 0 };     //假值链（短路计算）
int trueCount[4] = { 0 };     //真值计数
int falseCount[4] = { 0 };    //假值计数
int conditionLevel = 0;       //条件（包括if和for）的层数
int cxTemp;                   //临时保存cx（then的地址）

/*
	下面两个变量用于if语句，如果不存在else，在if中将会去读取语句后面的分号后面的值。
	例如： if a > 1 then 
			Statement ;
		  j := 1;
		当执行完if语句后，当前的符号变成标识符，即为'j'。
		因此需要将'j'保存起来，将sym 赋值为';'，然后下一个符号取'j'，称'j'为预知符号。
*/
int sym_count = 0;  //预知的符号数目
int sym_stack[10] = { 0 };  //预知的符号

/*
	以下符号为break语句所用到的，breakCount 表示break的层数，
	cxBreak[]存储当前层break后应该调转到的指令
*/
int breakCount = 0;  //记录循环层数，以判断是否允许break
int cxBreak[4] = { 0 };

char* word[NUM_RESERVED + 1] = {
	"",
	"begin", "call", "const", "do", "end", "if", "odd", "procedure", "then", "var", "while", "and", "or", "not",
    "else", "for", "break", "exit", "read", "print", "bool"
};

int wsym[NUM_RESERVED + 1] = {
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE, SYM_AND, SYM_OR, SYM_NOT,
	SYM_ELSE, SYM_FOR, SYM_BREAK, SYM_EXIT, SYM_READ, SYM_WRITE, SYM_BOOL
};

/*
 * NULL         =
 * PLUS         +
 * MINUS        -
 * TIMES        *
 * SLASH        /
 * LPAREN       (
 * RPAREN       )
 * EQU          :=
 * COMMA        ,
 * PERIOD       .
 * SEMICOLON    ;
 * AND          and &
 * OR           or  |
 * NOT          not !
 * LSPAREN      [
 * RSPAREN      ]
 * MOD          %
 */
int ssym[MAX_SYM + 1] = {
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,
	SYM_AND, SYM_OR, SYM_NOT, SYM_LSPAREN, SYM_RSPAREN, SYM_MOD
};

char csym[MAX_SYM + 1] = {
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';','&','|','!', '[', ']','%'
};

char* mnemonic[MAX_INS] = {
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC", "JPNC",
	"LDA", "STA", "RDA", "WTA", "REA", "WRT"
};

//使用mask结构（添加value变量）来构建符号表。
typedef struct {
	char  name[MAX_ID + 1];
	int   kind;
	int   value;
	short level;
	short address;
} mask;  //单个变量的信息

//数组结构
typedef struct {
	char name[MAX_ID + 1];	    //数组变量名称
	int  sum;					//数组所有元素占用空间
	int  n;						//数组总维数
	int  dim[MAX_DIM];			//数组对应维数的存储空间
	int  size[MAX_DIM];			//数组对应维数的地址偏移量大小
	int  addr;					//数组首地址
} arr;  //单个数组类型的信息

mask table[MAX_TX];
arr tempArray, arrayTable[MAX_TX];	//利用数组结构构建数组符号表
char line[80];
instruction code[MAX_CX];  //指令数组

#endif