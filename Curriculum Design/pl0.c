#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pl0.h"
#include "set.h"

//打印错误信息
void Error(int n) {
	int i;
	printf("      ");
	for (i = 1; i <= cc - 1; i++) {
        printf(" ");
    }
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
}

//识别一个字符
void GetCh(void) {
	if (cc == ll) {  //已经读到该行行尾，需要再读一行
		if (feof(infile)) {  //到达文件尾
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = 0;  //当前行的长度
        cc = 0;  //目前读到的位置
		//printf("%5d  ", cx);
        printf("%d\t", cx);
		while ((!feof(infile))&&((ch = getc(infile)) != '\n')) {  //未到文件尾，且输入的不是换行符
            if ((int)ch != 255) {  //文件末尾最后一个字符
                printf("%c", ch);  //在读取字符时完成对代码的输出
            }
            line[++ll] = (char)ch;
		}
		printf("\n");  //是换行符
		line[++ll] = ' ';
	}
	ch = line[++cc];
}

//从输入流中读取一个标识符
void GetSym() {
	int i, k;
	char a[MAX_ID + 1];  //临时存放一个标识符
	if (sym_count > 0) {  //if中使用该变量，当sym_stack中不为空时，将sym_stack栈顶出栈，保证识别的sym的正确。
		sym = sym_stack[--sym_count];
		return;
	}
	while (ch == ' '|| ch == '\t') {  //过滤空格和制表符
        GetCh();
    }

	if (isalpha(ch)) {  //标识符是保留字或变量名
		k = 0;  //a数组的指针
		do {
			if (k < MAX_ID) {
                a[k++] = (char)ch;  //将每一个字符临时存在a数组中
            }
			GetCh();  //结束循环后ch为下一个字符
		} while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NUM_RESERVED;
		while (strcmp(id, word[i--]) != 0);  //将id和保留字匹配
		if (++i) {
            sym = wsym[i];  //匹配到关键字
        }
		else {  //未匹配到关键字，则为变量名
			if (ch == '[') {
                sym = SYM_ARRAY;  //若变量名后有[，则其为一个数组
            }
			else {
                sym = SYM_IDENTIFIER;  //否则就是一个普通变量
            }
		}
	}

	else if (isdigit(ch)) {
		k = 0;
        num = 0;
		sym = SYM_NUMBER;
		do {
			num = num * 10 + ch - '0';  //计算数值大小
			k++;  //记录数据位数
			GetCh();
		} while (isdigit(ch));
		if (k > MAX_NUM) {
            Error(25);  //数值过大
        }
	}

	else if (ch == '\n') {
		GetCh();
        GetSym();
	}

	else if (ch == ':') {
		GetCh();
		if (ch == '=') {
			sym = SYM_BECOMES;  //:=（赋值符号）
			GetCh();
		}
		else {
			sym = SYM_NULL;  //:（空符号）
		}
	}

	else if (ch == '>') {
		GetCh();
		if (ch == '=') {
			sym = SYM_GEQ;  //>=
			GetCh();
		}
		else {
			sym = SYM_GTR;  //>
		}
	}

	else if (ch == '<') {
		GetCh();
		if (ch == '=') {
			sym = SYM_LEQ;  //<=
			GetCh();
		}
		else if (ch == '>') {
			sym = SYM_NEQ;  //<>
			GetCh();
		}
		else {
			sym = SYM_LES;  //<
		}
	}

	else if (ch == '/') {
		GetCh();
		if (ch == '*') {  //多行注释
			GetCh();
			while (1) {
				while (ch != '*') {  //一直过滤字符，直到出现*，程序没有*，会提示incomplete
                    GetCh();
                }
				GetCh();
				if (ch == '/') {
                    break;
                }
			}
			GetCh();
            GetSym();
		}
		else if (ch == '/') {  //单行注释
			cc = ll;  //直接跳过该行
			ch = ' ';
            GetSym();
		}
		else {
			sym = SYM_SLASH;  //只有一个斜杠则为除号
		}
	}

	else if (ch == ']') {  //数组的右括号
		GetCh();
		sym = SYM_RSPAREN;
	}

	else if (ch == '+') {
		GetCh();
		if (ch == '=') {
			GetCh();
			sym = SYM_ADDPLUS;  //+=
		}
		else {
			sym = SYM_PLUS;  //=
		}
	}

	else if (ch == '-') {
		GetCh();
		if (ch == '=') {
			GetCh();
			sym = SYM_SUBPLUS;  //-=
		}
		else {
			sym = SYM_MINUS;  //-
		}
	}

	else {
		i = MAX_SYM;
		csym[0] = (char)ch;
		while (csym[i--] != ch);  //匹配其它正确的符号
		if (++i) {  //匹配到
			sym = ssym[i];
			GetCh();
		}
		else {  //未匹配到，报错
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
}

//生成一条（汇编）指令
void Generate(int x, int y, int z) {
	if (cx > MAX_CX) {
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
}

//使用s1进行测试，如果出错，跳过不属于s1和s2的所有标识符
void Test(symset s1, symset s2, int n) {
	symset s;
	if (!FindSet(sym, s1)) {  //如果sym不在s1中，错误，并进行错误处理
        Error(n);
		s = UniteSet(s1, s2);
		while(!FindSet(sym, s)) {  //跳过不在s1和s2中的所有标识符
            GetSym();
        }
        DestroySet(s);
	}
}

int dx;  //数据分配指针，dx是全局变量

//将变量（const型、var型、procedure型、bool型）存入表格中
void Enter(int kind) {
	mask* mk;
	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind) {
        case ID_CONSTANT:
            if (num > MAX_ADDRESS) {
                Error(25);
                num = 0;
            }
            table[tx].value = num;
            break;
        case ID_VARIABLE:
            mk = (mask*) &table[tx];  //找到当前table中的位置
            mk->level = (short)level;
            mk->address = (short)dx;
            dx++;
            break;
        case ID_PROCEDURE:
            mk = (mask*) &table[tx];
            mk->level = (short)level;
            break;
        case ID_BOOL:
            mk = (mask*) &table[tx];  //找到当前table中的位置
            mk->level = (short)level;
            mk->address = (short)dx;
            dx++;
            break;
        default:
            break;
	}
}

//将数组类型填入数组的符号表中
void EnterArray() {
	ax++;
    arrayTable[ax] = tempArray;
	strcpy(arrayTable[ax].name, id);
    Enter(ID_VARIABLE);  //数组类型一定为var型
	arrayTable[ax].addr = tx;  //记录数组初始偏移地址（第一个数据在table表中的位置）
	for (int i = arrayTable[ax].sum - 1; i > 0; i--) {  //将数组的所有存储空间填入符号表中
        Enter(ID_VARIABLE);  //将剩下的数据也填入table表中
    }
}

//在table表中寻找变量
int Find(char* sid) {
	int i;
	strcpy(table[0].name, sid);
	i = tx + 1;  //table表的当前长度
	while (strcmp(table[--i].name, sid) != 0);  //返回0时，未找到
	return i;
}

//定位标识符在数组符号表中的位置
int FindArray() {
	int i = 0;
	while (strcmp(arrayTable[++i].name, id) != 0);  //判断值为id
	if (i <= ax) {
        return i;
    }
	else {
        return 0;
    }
}

//声明const型变量
void DeclareConst() {
	if (sym == SYM_IDENTIFIER) {  //完成常量定义
        GetSym();
		if (sym == SYM_EQU || sym == SYM_BECOMES) {  //识别到=或:=
			if (sym == SYM_BECOMES) {  //应使用=定义变量
                Error(1);  //报错，但不影响后续操作
            }
            GetSym();
			if (sym == SYM_NUMBER) {  //将数赋给常量
                Enter(ID_CONSTANT);
                GetSym();
			}
			else {
                Error(2);  //缺少数据
			}
		}
		else {
            Error(3);  //缺少等号
		}
	} else {
        Error(4);  //缺少变量
    }
}

//声明var型变量
void DeclareVar() {
	int dim = 0;  //记录数组的维数
	if (sym == SYM_IDENTIFIER) {
        Enter(ID_VARIABLE);  //变量声明
        GetSym();
	}
	else if (sym == SYM_ARRAY) {  //数组声明
		while (ch == '[') {
			dim++;
			GetCh();  //读取'['后的下一个字符
            GetSym();  //读取'['和']'之间的数字，其值num为当前维数的数组空间
			tempArray.dim[dim - 1] = num;  //保存数组所在维数的数据大小
            GetSym();  //读取']'后的下一个字符
		}
        tempArray.n = dim;  //数组的总维数
		tempArray.size[dim - 1] = 1;  //最外层的偏移量大小为1
		for (int i = dim - 1; i > 0; i--) {
            tempArray.size[i - 1] = tempArray.size[i] * tempArray.dim[i];  //计算数组元素的偏移地址
        }  //目的：将多维数组转化为线性结构
		tempArray.sum = tempArray.size[0] * tempArray.dim[0];  //计算出数组的所有元素占用空间
        EnterArray();  //填入数组符号表
        GetSym();
	}
	else {
        Error(4);
	}
}

//声明bool型变量
void DeclareBool() {
    if (sym == SYM_IDENTIFIER) {
        Enter(ID_BOOL);
        GetSym();
    }
}

//打印指令
void ListCode(int from, int to) {
	int i;
	printf("\n");
	for (i = from; i < to; i++) {
		//printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
        printf("%d\t%s\t\t%d\t\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
}

void Expression(symset fsys);

//处理factor（因子）
void Factor(symset fsys) {
	int i, j;
	int dim = 0;  //数组当前维数
	symset set;
	mask* newMask;
    Test(facbegsys, fsys, 24);  //符号不能作为表达式的开始
	while (FindSet(sym, facbegsys)) {  //一直进行factor计算，直到sym不在facbegsys（IDENTIFIER, NUMBER, LPAREN, ARRAY, NULL）中
		if (sym == SYM_IDENTIFIER) {  //因子为标识符类型
			if (!(i = Find(id))) {
                Error(11);  //i未获取到id时，未定义该变量，报错
			}
			else {
				switch (table[i].kind) {
                    case ID_CONSTANT:
                        Generate(LIT, 0, table[i].value);  //常量进栈
                        break;
                    case ID_VARIABLE:
                        newMask = (mask*) &table[i];  //取当前数组，不是数组，层数为当前层数
                        Generate(LOD, level - newMask->level, newMask->address);  //变量进栈
                        break;
                    case ID_PROCEDURE:
                        Error(21);  //procedure变量不能在一个表达式中，报错
                        break;
                    case ID_BOOL:
                        newMask = (mask*) &table[i];
                        Generate(LOD, level - newMask->level, newMask->address);
                        break;
				}
			}
            GetSym();
		}
		else if (sym == SYM_ARRAY) {  //因子为数组类型
			if (!(i = FindArray())) {  //定位数组变量在符号表中的位置
                Error(11);
            }
			else {
				j = arrayTable[i].addr;  //记录数组首地址
				newMask = (mask*) &table[j];  //取当前数组
                Generate(LIT, 0, 0);  //通过累加数组偏移量来确定元素位置，初始为0
				while (ch == '[') {
					dim++;  //进入左括号后维数+1
					GetCh();
                    GetSym();  //读取数组括号中的数值
					set = UniteSet(CreateSet(SYM_RSPAREN, SYM_NULL), fsys);  //将新创建的set和fsys合并在一起
                    Expression(set);  //计算数组中的表达式
                    DestroySet(set);  //计算结束后删除set
                    Generate(LIT, 0, arrayTable[i].size[dim - 1]);  //取当前维数偏移大小到栈顶
                    Generate(OPR, 0, OPR_MUL);  //该维数的值乘以该维数偏移大小
                    Generate(OPR, 0, OPR_ADD);  //累加到总偏移
				}
                Generate(LDA, level - newMask->level, newMask->address);  //生成加载数组指令，记录层差和偏移
			}
            GetSym();
		}
		else if (sym == SYM_NUMBER) {
			if (num > MAX_ADDRESS) {
                Error(25);  //number过大，报错
				num = 0;
			}
            Generate(LIT, 0, num);
            GetSym();
		}
		else if (sym == SYM_LPAREN) {
            GetSym();
			set = UniteSet(CreateSet(SYM_RPAREN, SYM_NULL), fsys);
            Expression(set);  //使用expression计算括号中的表达式，计算完成后应为右括号
            DestroySet(set);
			if (sym == SYM_RPAREN) {
                GetSym();
			}
			else {
                Error(22);  //缺少右括号，报错
			}
		}
        Test(fsys, CreateSet(SYM_LPAREN, SYM_NULL), 23);
	}
}

//处理term（短语）
void Term(symset fsys) {
	int mulOp;
	symset set;
	set = UniteSet(fsys, CreateSet(SYM_TIMES, SYM_SLASH, SYM_LSPAREN, SYM_RSPAREN, SYM_NULL));  //fsys中添加中括号的symtype值
    Factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH) {
        mulOp = sym;
        GetSym();
        Factor(set);  //计算短语中的因子
		if (mulOp == SYM_TIMES) {  //乘法计算
            Generate(OPR, 0, OPR_MUL);
		}
		else {  //除法计算
            Generate(OPR, 0, OPR_DIV);
		}
	}
    DestroySet(set);
}

//处理expression（表达式）
void Expression(symset fsys) {
	int addOp;
	symset set;
	set = UniteSet(fsys, CreateSet(SYM_PLUS, SYM_MINUS, SYM_MOD, SYM_LSPAREN, SYM_RSPAREN, SYM_NULL));  //fsys中添加中括号的symtype值
	if (sym == SYM_PLUS || sym == SYM_MINUS) {  //先读一次加号或减号
		addOp = sym;
        GetSym();
        Term(set);
		if (addOp == SYM_MINUS) {  //开始就是-，则为负数，执行负数操作
            Generate(OPR, 0, OPR_NEG);
		}
	}
	else {
        Term(set);  //不是加减号，用短语来查找到加减号
	}
	while (sym == SYM_PLUS || sym == SYM_MINUS||sym == SYM_MOD) {
        addOp = sym;
        GetSym();
        Term(set);
		if (addOp == SYM_PLUS) {  //加法操作
            Generate(OPR, 0, OPR_ADD);
		}
		else if (sym == SYM_MINUS) {  //减法操作
            Generate(OPR, 0, OPR_MIN);
		}
		else {  //取模操作
            Generate(OPR, 0, OPR_MOD);
		}
	}
    DestroySet(set);
}

//处理condition（关联符号情况）
void Condition(symset fsys) {
	int relop;
	symset set;
	if (sym == SYM_ODD) {  //奇数操作
        GetSym();
        Expression(fsys);  //计算表达式
        Generate(OPR, 0, 6);
	}
	else {
		set = UniteSet(relset, fsys);  //将关联符号加入到fsys中
        Expression(set);  //计算表达式
        DestroySet(set);
		if (!FindSet(sym, relset)) {  //没有找到关联符号，报错
            Error(20);
		}
		else {
			relop = sym;
            GetSym();
            Expression(fsys);
			switch (relop) {  //分别执行相关操作
			case SYM_EQU:
                Generate(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
                Generate(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
                Generate(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
                Generate(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
                Generate(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
                Generate(OPR, 0, OPR_LEQ);
				break;
            default:
                break;
			}
		}
	}
}

//处理not and or情况，进行短路计算
void ExCondition(symset fsys) {
	int reLop;
	conditionLevel++;
    trueCount[conditionLevel] = 0;  //用于记录当前层true和false的个数，运算前先置0
	falseCount[conditionLevel] = 0;
	if (sym == SYM_NOT) {  //第一个条件是not
        GetSym();
        Condition(fsys);
        Generate(OPR, 0, OPR_NOT);
	}
	else {
        Condition(fsys);
	}
	if (sym == SYM_THEN || sym == SYM_SEMICOLON || sym == SYM_DO) {  //只有一个条件，即没有下一个not and or计算
		false[conditionLevel][falseCount[conditionLevel]++] = cx;  //条件不成立，修改false_out
        Generate(JPC, 0, 0);  //不成立则跳转到then的末尾，执行else部分
		true[conditionLevel][trueCount[conditionLevel]++] = cx;  //条件成立，修改true_out
        Generate(JMP, 0, 0);  //成立，则执行then部分，并跳过else部分
		return;
	}
	else if (sym == SYM_AND || sym == SYM_OR || sym == SYM_NOT) {
		while (sym == SYM_AND || sym == SYM_OR || sym == SYM_NOT) {  //进行and or not计算（短路计算）
			reLop = sym;
            GetSym();
			switch (reLop) {
			case SYM_OR:
                true[conditionLevel][trueCount[conditionLevel]++] = cx;  //or前为真：短路计算，表达式为真
                Generate(JPNC, 0, 0);  //如果成立，跳到true出口，等待回填
				if (sym == SYM_NOT) {  //or的后面是not
                    GetSym();
                    Condition(fsys);
                    Generate(OPR, 0, OPR_NOT);  //将布尔值取反
				}
				else {
                    Condition(fsys);
				}
				if (sym == SYM_THEN || sym == SYM_SEMICOLON) {
                    false[conditionLevel][falseCount[conditionLevel]++] = cx;
                    Generate(JPC, 0, 0);
                    true[conditionLevel][trueCount[conditionLevel]++] = cx;
                    Generate(JMP, 0, 0);
					return;
				}
				break;
			case SYM_AND:
                false[conditionLevel][falseCount[conditionLevel]++] = cx;  //and前为假：短路计算，表达式为假
                Generate(JPC, 0, 0);  //如果不成立，跳到false出口，等待回填
				if (sym == SYM_NOT) {  //and后面是not
                    GetSym();
                    Condition(fsys);
                    Generate(OPR, 0, OPR_NOT);
				}
				else {
                    Condition(fsys);
				}
				if (sym == SYM_THEN || sym == SYM_SEMICOLON) {
                    false[conditionLevel][falseCount[conditionLevel]++] = cx;
                    Generate(JPC, 0, 0);
                    true[conditionLevel][trueCount[conditionLevel]++] = cx;
                    Generate(JMP, 0, 0);
					return;
				}
				break;
            default:
                break;
			}
		}
        Error(16);
	}
	else {
        Error(16);
	}
}

//处理statement（各种情况）
void Statement(symset fsys) {
	int i, cx1, dim = 0, j, before_update, before_condition;
	symset set1, set;
	mask* newMask;

	if (sym == SYM_IDENTIFIER) {  //变量赋值
		if (!(i = Find(id))) {  //找不到该标识符的声明，报错
            Error(11);
		}
		else if (table[i].kind != ID_VARIABLE && table[i].kind != ID_BOOL) {  //标识符不是变量，报错
            Error(12);
			i = 0;
		}
        newMask = (mask*)&table[i];
        GetSym();
		if (sym == SYM_BECOMES) {  //复制操作过程中使用:而不是:=
            GetSym();
            Expression(fsys);  //错误，但是仍可继续进行计算表达式
		}
		else if (sym == SYM_ADDPLUS) {  //进行加法运算
            GetSym();
            Generate(LOD, level - newMask->level, newMask->address);
            Expression(fsys);
            Generate(OPR, 0, OPR_ADD);
		}
		else if (sym == SYM_SUBPLUS) {  //进行减法运算
            GetSym();
            Generate(LOD, level - newMask->level, newMask->address);
            Expression(fsys);
            Generate(OPR, 0, OPR_MIN);
		}
		else {
            Error(13);
		}
		if (i) {
            Generate(STO, level - newMask->level, newMask->address);  //栈顶内容赋给变量
		}
	}

    if (sym == SYM_ARRAY) {  //statement语句起始为数组类型（数组赋值）
		if (!(i = FindArray())) {
            Error(11);
        }
		else {
            Generate(LIT, 0, 0);  //常数0入栈
			while (ch == '[') {
				dim++;
				GetCh();
                GetSym();
				set = UniteSet(CreateSet(SYM_RSPAREN, SYM_NULL), fsys);
                Expression(set);  //计算[]内表达式值
                DestroySet(set);
                Generate(LIT, 0, arrayTable[i].size[dim - 1]);  //取当前维数偏移大小到栈顶
                Generate(OPR, 0, OPR_MUL);  //该维数的值乘以该维数偏移大小
                Generate(OPR, 0, OPR_ADD);  //累加到总偏移
			}
		}
        GetSym();
		if (sym == SYM_BECOMES) {
            GetSym();
        }
		else {
            Error(13);  //使用了=而非:=
        }
		set = UniteSet(CreateSet(SYM_RSPAREN, SYM_NULL), fsys);
        Expression(set);  //计算赋值的表达式的值
        DestroySet(set);
		j = arrayTable[i].addr;
        newMask = (mask*)&table[j];
		if (j) {
            Generate(STA, level - newMask->level, newMask->address);  //存储数组值到特定层和特定偏移地址
        }
	}

	if (sym == SYM_CALL) {  //call处理，调用一个过程
        GetSym();
		if (sym != SYM_IDENTIFIER) {  //需要调用一个标识符，不是标识符则报错
            Error(14);
		}
		else {
			if (!(i = Find(id))) {
                Error(11);
			}
			else if (table[i].kind == ID_PROCEDURE) {
                newMask = (mask*) &table[i];
                Generate(CAL, level - newMask->level, newMask->address);  //调用一个过程
			}
			else {
                Error(15);  //不是过程类型，报错
			}
            GetSym();
		}
	}

	if (sym == SYM_IF) {  //if处理，处理过程中注意短路计算
        GetSym();
        set1 = CreateSet(SYM_THEN, SYM_DO, SYM_AND, SYM_NOT, SYM_OR, SYM_NULL);
        set = UniteSet(set1, fsys);
        ExCondition(set);
        DestroySet(set1);
        DestroySet(set);
		if (sym == SYM_THEN) {
            GetSym();
		}
		else {
            printf("%d", sym);
            Error(16);  //有if必有then，缺少then报错
		}
		for (int k = 0; k < trueCount[conditionLevel]; k++) {  //所有真出口都跳到statement的前面（then后）
            code[true[conditionLevel][k]].a = cx;
        }
        Statement(fsys);  //条件成立的执行语句
		cxTemp = cx;  //保存then位置的cx
        Generate(JMP, 0, 0);  //成立语句执行完之后 要么跳到else-statement之后，要么继续执行
		if (sym == SYM_SEMICOLON) {
            GetSym();
		}
		else {
            Error(10);
		}
		if (sym == SYM_ELSE) {  //如果有else语句，则false出口到else语句（else后）
            GetSym();
			for (i = 0; i < falseCount[conditionLevel]; i++) {  //所有假出口都跳到else后面
                code[false[conditionLevel][i]].a = cx;
            }
            Statement(fsys);
		}
		else {  //没有else语句
			sym_stack[sym_count++] = sym;
			sym = SYM_SEMICOLON;
			for (int k = 0; k < falseCount[conditionLevel]; k++) {  //所有假出口都跳到if-statement后面（整个if后）
                code[false[conditionLevel][k]].a = cx;
            }
		}
		code[cxTemp].a = cx;
		conditionLevel--;
	}

	if (sym == SYM_FOR) {  //for处理，举例：for(int i=0;i<=10;i++)
        GetSym();
		breakCount++;
		if (sym == SYM_LPAREN) {  //for(这里必须是左括号
            GetSym();
		}
		else {
            Error(26);  //缺少左括号，报错
		}
        Statement(fsys);  //循环变量初始值
		if (sym == SYM_SEMICOLON) {  //for(int i=0;这里必须有分号
            GetSym();
		}
		else {
            Error(17);  //缺少分号，报错
		}
		before_condition = cx;  //记录循环前的指令地址，执行完需要更新：跳转到这里（记录的是i<=10的位置）
		set1 = CreateSet(SYM_THEN, SYM_DO, SYM_AND, SYM_NOT, SYM_OR, SYM_NULL);
		set = UniteSet(set1, fsys);
        ExCondition(set);  //使用ex_condition处理的是i<=10
        DestroySet(set1);
        DestroySet(set);
		if (sym == SYM_SEMICOLON) {  //for(int i=0;i<=10;这里必须有分号，否则报错
            GetSym();
		}
		else {
            Error(17);
		}
		before_update = cx;  //记录更新变量的指令地址，当循环体执行完后需要跳转到这里（记录的是i++的位置）
        Statement(fsys);  //循环变量自增自减
        Generate(JMP, 0, before_condition);  //更新完后跳到条件处
		if (sym == SYM_RPAREN) {  //for(int i=0;i<=10;i++)最后必须是右括号，否则报错
            GetSym();
		}
		else {
            Error(26);
		}
		for (int k = 0; k < trueCount[conditionLevel]; k++) {  //所有真出口都跳到loop-statement的前面（右括号后）
            code[true[conditionLevel][k]].a = cx;
        }
        Statement(fsys);  //执行循环体
        Generate(JMP, 0, before_update);  //执行完后跳到自增自减语句
		for (int k = 0; k < falseCount[conditionLevel]; k++) {  //所有假出口都跳到loop-statement后面（整个循环体后）
            code[false[conditionLevel][k]].a = cx;
        }
		conditionLevel--;  //减少条件层次
		if (cxBreak[breakCount] > 0) {  //如果存在break，则跳到该处（整个循环体后）
			code[cxBreak[breakCount]].a = cx;
			breakCount--;
		}
	}

	if (sym == SYM_BEGIN) {  //begin处理，开始一个block
        GetSym();
		set1 = CreateSet(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = UniteSet(set1, fsys);
        Statement(set);
		while (sym == SYM_SEMICOLON || FindSet(sym, statbegsys)) {  //寻找一个状态开始符
			if (sym == SYM_SEMICOLON) {  //并检查此段程序中的分号
                GetSym();
			}
			else {
                Error(10);
			}
            Statement(set);  //处理这个块
		}
        DestroySet(set1);
        DestroySet(set);
		if (sym == SYM_END) {  //块结束应有end标识符，否则报错
            GetSym();
		}
		else {
            Error(17);
		}
	}

	if (sym == SYM_BREAK) {  //break处理
        GetSym();
		if (breakCount <= 0) {
            Error(27);  //此时不能break，报错
		}
		else {  //可以break，则直接跳到循环体的后面
			cxBreak[breakCount] = cx;
            Generate(JMP, 0, 0);
		}
	}

	if (sym == SYM_EXIT) {  //exit处理，这里的exit指的是跳出过程
        GetSym();
        Generate(OPR, 0, OPR_RET);
	}

    if (sym == SYM_WHILE) {  //while处理，例子：while a<0 do a:=a+1
		breakCount++;
		cx1 = cx;  //cx1保存的是while后的地址（a<0前）
        GetSym();
		set1 = CreateSet(SYM_DO, SYM_AND, SYM_NOT, SYM_OR, SYM_NULL);
		set = UniteSet(set1, fsys);
        ExCondition(set);  //用ex_expression跳到do
        DestroySet(set1);
        DestroySet(set);
		if (sym == SYM_DO) {
            GetSym();
		}
		else {  //有while无do，报错
            Error(18);
		}
		for (int k = 0; k < trueCount[conditionLevel]; k++) {  //所有真出口都跳到while-statement的前面（a:=a+1前）
            code[true[conditionLevel][k]].a = cx;
        }
        Statement(fsys);  //计算表达式，执行完循环
        Generate(JMP, 0, cx1); //执行完循环体，跳回条件前进行判断
		for (int k = 0; k < falseCount[conditionLevel]; k++) {  //所有假出口都跳到while-Statement 后面（整个循环体后）
			code[false[conditionLevel][k]].a = cx;
        }
		conditionLevel--;  //减小条件层次
		if (cxBreak[breakCount] > 0) {  //如果存在break，则跳到该处（整个循环体后）
			code[cxBreak[breakCount]].a = cx;
			breakCount--;
		}
	}

	if (sym == SYM_READ) {  //读取操作，例子：read(s)
        GetSym();
		if (sym != SYM_LPAREN) {  //缺少左括号，报错
            Error(26);
        }
		do {
            GetSym();
			if (sym == SYM_IDENTIFIER) {  //读取的数据是变量值
				if (!(i = Find(id))) {
                    Error(11);
                }
				else {
                    newMask = (mask*)& table[i];
                    Generate(REA, level - newMask->level, newMask->address);  //生成读取变量的REA指令
				}
			}
			else if (sym == SYM_ARRAY) {  //读取的数据是数组
				dim = 0;
				if (!(i = FindArray())) {
                    Error(11);
                }
				else {
					j = arrayTable[i].addr;
                    newMask = (mask*)&table[j];
                    Generate(LIT, 0, 0);
					while (ch == '[') {  //计算数组地址
						dim++;
						GetCh();
                        GetSym();
						set = UniteSet(CreateSet(SYM_RSPAREN, SYM_NULL), fsys);
                        Expression(set);
                        DestroySet(set);
                        Generate(LIT, 0, arrayTable[i].size[dim - 1]);
                        Generate(OPR, 0, OPR_MUL);
                        Generate(OPR, 0, OPR_ADD);
					}
                    Generate(RDA, level - newMask->level, newMask->address);  //生成读取数组的RDA指令
				}
			}
            GetSym();
		} while (sym == SYM_COMMA);  //如果变量之间仍有逗号表示未读完，继续读取，例如read(s,r)
		if (sym == SYM_RPAREN) {  //最后以右括号结尾，否则报错
            GetSym();
        }
		else {
            Error(22);
        }
	}

	if (sym == SYM_WRITE) {  //写入操作
        GetSym();
		if (sym != SYM_LPAREN) {
            Error(26);
        }
		do {
            GetSym();
			if (sym == SYM_RPAREN) {  //如果读取完左括号，下一个字符读到的是右括号，该过程为输出回车
                Generate(OPR, 0, OPR_WEN);	//输出回车符，并退出
				break;
			}
			else if (sym == SYM_IDENTIFIER) {  //输出的内容为常量或变量
				if (!(i = Find(id))) {
                    Error(11);
                }
                newMask = (mask *)&table[i];
				if (table[i].kind == ID_CONSTANT) {  //输出常量值
                    Generate(LIT, 0, table[i].value);
                    Generate(OPR, 0, OPR_WRT);
				}
				else if (i)	{
                    Generate(WRT, level - newMask->level, newMask->address);  //若为变量，用WRITE指令
                }
			}
			else if (sym == SYM_ARRAY) {  //若为数组
				dim = 0;
				if (!(i = FindArray())) {
                    Error(11);
                }
				else {
					j = arrayTable[i].addr;
                    newMask = (mask *)&table[j];
                    Generate(LIT, 0, 0);
					while (ch == '[') {
						dim++;
						GetCh();
                        GetSym();
						set = UniteSet(CreateSet(SYM_RSPAREN, SYM_NULL), fsys);
                        Expression(set);
                        DestroySet(set);
                        Generate(LIT, 0, arrayTable[i].size[dim - 1]);
                        Generate(OPR, 0, OPR_MUL);
                        Generate(OPR, 0, OPR_ADD);
					}
                    Generate(WTA, level - newMask->level, newMask->address);  //用WTA指令打印所在层及偏移地址的数组值
				}
			}
			else if (sym == SYM_NUMBER) {  //如果是数字，直接取数到栈顶
                Generate(LIT, 0, num);
                Generate(OPR, 0, OPR_WRT);
			}
            GetSym();
		} while (sym == SYM_COMMA);  //若中间为逗号，则继续输出
		if (sym == SYM_RPAREN) {
            GetSym();
        }
		else {
            Error(22);
        }
	}
    Test(fsys, phi, 19);
}

//处理block（块）
void Block(symset fsys) {
	int dx1, tx1;
	mask* mk;
	symset set1, set;
	dx = 3;
	mk = (mask*) &table[tx];
	mk->address = (short)cx;
    Generate(JMP, 0, 0);
	if (level > MAX_LEVEL) {  //超过块的最大深度，报错
        Error(32);
	}
	do {
		if (sym == SYM_CONST) {
            GetSym();
			do {
                DeclareConst();
				while (sym == SYM_COMMA) {
                    GetSym();
                    DeclareConst();
				}
				if (sym == SYM_SEMICOLON) {
                    GetSym();
				}
				else {  //缺少逗号或分号
                    Error(5);
				}
			} while (sym == SYM_IDENTIFIER);  //只要是标识符就继续循环操作
		}
		if (sym == SYM_VAR) {
            GetSym();
			do {
                DeclareVar();
				while (sym == SYM_COMMA) {
                    GetSym();
                    DeclareVar();
				}
				if (sym == SYM_SEMICOLON) {
                    GetSym();
				}
				else {
                    Error(5);
				}
			} while (sym == SYM_IDENTIFIER);
		}
        if (sym == SYM_BOOL) {
            GetSym();
            do {
                DeclareBool();
                while (sym == SYM_COMMA) {
                    GetSym();
                    DeclareBool();
                }
                if (sym == SYM_SEMICOLON) {
                    GetSym();
                }
                else {
                    Error(5);
                }
            } while (sym == SYM_IDENTIFIER);
        }
		while (sym == SYM_PROCEDURE) {
            GetSym();
			if (sym == SYM_IDENTIFIER) {
                Enter(ID_PROCEDURE);
                GetSym();
			}
			else {
                Error(4);
			}
			if (sym == SYM_SEMICOLON) {
                GetSym();
			}
			else {
                Error(5);
			}
			level++;
            tx1 = tx;
			dx1 = dx;
			set1 = CreateSet(SYM_SEMICOLON, SYM_NULL);
			set = UniteSet(set1, fsys);
            Block(set);
            DestroySet(set1);
            DestroySet(set);
			tx = tx1;
			dx = dx1;
			level--;
			if (sym == SYM_SEMICOLON) {
                GetSym();
				set1 = CreateSet(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = UniteSet(statbegsys, set1);
                Test(set, fsys, 6);
                DestroySet(set1);
                DestroySet(set);
			}
			else {
                Error(5);
			}
		 }
		set1 = CreateSet(SYM_IDENTIFIER, SYM_NULL);
		set = UniteSet(statbegsys, set1);
        Test(set, declbegsys, 7);  //务必按照const/var/bool的顺序声明
        DestroySet(set1);
        DestroySet(set);
	} while (FindSet(sym, declbegsys));  //当符号是声明符时，循环执行这部分

    //进一步调用
	code[mk->address].a = cx;
	mk->address = (short)cx;
    //用INT预留空间，务必确保预留充足空间，否则后续在输出操作时，栈中的数据会被覆盖。
    Generate(INT, 0, dx);  //这里就是dx（被修改后的值，而不是初始值）
	set1 = CreateSet(SYM_SEMICOLON, SYM_END, SYM_ELSE, SYM_RPAREN, SYM_NULL);
	set = UniteSet(set1, fsys);
    Statement(set);  //调用Statement函数，处理块内部分
    DestroySet(set1);
    DestroySet(set);
    Generate(OPR, 0, OPR_RET);  //执行完一个过程，返回
    Test(fsys, phi, 8);  //跟随语句是一个不正确的符号
}

int Base(const int stack[], int currentLevel, int levelDiff) {  //寻找Base
	int b = currentLevel;
	while (levelDiff--) {  //执行levelDiff次循环
        b = stack[b];
    }
	return b;  //返回levelDiff == 0时stack的level
}

//解释和执行代码
void Interpret() {
	int pc = 0;        //程序计数
	int stack[SIZE_STACK], input;
	int top = 3;       //栈顶指针
	int b = 1;         //程序、基本和顶层寄存器
	instruction i;     //指令寄存器
	printf("Begin executing PL/0 program.\n");
	stack[1] = 0;  //栈中初始化为三个元素0
    stack[2] = 0;
    stack[3] = 0;
	do {
		i = code[pc++];  //从code中读取代码
		switch (i.f) {
            case LIT:
                stack[++top] = i.a;
                break;
            case OPR:
                switch (i.a) {
                    case OPR_RET:
                        top = b - 1;
                        pc = stack[top + 3];
                        b = stack[top + 2];
                        break;
                    case OPR_NEG:
                        stack[top] = -stack[top];
                        break;
                    case OPR_NOT:
                        stack[top] = !stack[top];
                        break;
                    case OPR_ADD:
                        top--;
                        stack[top] += stack[top + 1];
                        break;
                    case OPR_MIN:
                        top--;
                        stack[top] -= stack[top + 1];
                        break;
                    case OPR_MOD:
                        top--;
                        stack[top] %= stack[top + 1];
                        break;
                    case OPR_MUL:
                        top--;
                        stack[top] *= stack[top + 1];
                        break;
                    case OPR_DIV:
                        top--;
                        if (stack[top + 1] == 0) {
                            fprintf(stderr, "Runtime Error: Divided by zero.\n");
                            fprintf(stderr, "Program terminated.\n");
                            continue;
                        }
                        stack[top] /= stack[top + 1];
                        break;
                    case OPR_ODD:
                        stack[top] %= 2;
                        break;
                    case OPR_EQU:
                        top--;
                        stack[top] = stack[top] == stack[top + 1];
                        break;
                    case OPR_NEQ:
                        top--;
                        stack[top] = stack[top] != stack[top + 1];
                    case OPR_LES:
                        top--;
                        stack[top] = stack[top] < stack[top + 1];
                        break;
                    case OPR_GEQ:
                        top--;
                        stack[top] = stack[top] >= stack[top + 1];
                    case OPR_GTR:
                        top--;
                        stack[top] = stack[top] > stack[top + 1];
                        break;
                    case OPR_LEQ:
                        top--;
                        stack[top] = stack[top] <= stack[top + 1];
                    case OPR_WEN:  //打印回车
                        printf("\n");
                        break;
                    case OPR_WRT:  //打印数字或常量，后加制表符
                        printf("%d\t", stack[top]);
                        top--;
                        break;
                    }
                break;
            case LOD:
                stack[++top] = stack[Base(stack, b, i.l) + i.a];
                break;
            case STO:
                stack[Base(stack, b, i.l) + i.a] = stack[top];
                top--;
                break;
            case CAL:
                stack[top + 1] = Base(stack, b, i.l);  //生成新的块
                stack[top + 2] = b;
                stack[top + 3] = pc;
                b = top + 1;
                pc = i.a;
                break;
            case INT:
                top += i.a;
                break;
            case JMP:
                pc = i.a;
                break;
            case JPC:
                if (stack[top] <= 0) {
                    pc = i.a;
                }
                top--;
                break;
            case JPNC:
                if (stack[top] > 0) {
                    pc = i.a;
                }
                top--;
                break;
            case REA:  //读取变量
                scanf("%d", &input);
                stack[Base(stack, b, i.l) + i.a] = input;
                break;
            case WRT:  //输出变量
                printf("%d\t", stack[Base(stack, b, i.l) + i.a]);
                break;
            case LDA:  //加载数组
                stack[top] = stack[Base(stack, b, i.l) + i.a + stack[top]];
                break;
            case STA:  //存储数组
                stack[Base(stack, b, i.l) + i.a + stack[top - 1]] = stack[top];
                top--;
                break;
            case WTA:  //输出数组值，后加制表符
                printf("%d\t", stack[Base(stack, b, i.l) + i.a + stack[top]]);
                break;
            case RDA:  //读取数组变量值
                scanf("%d", &input);
                stack[Base(stack, b, i.l) + i.a + stack[top]] = input;
                break;
            default:
                fprintf(stderr, "Runtime Error: unexpected instruction.\n");
		}
	} while (pc);
	printf("\nEnd executing PL/0 program.\n");
}

int main() {
	FILE* bin;
	char s[80];
	int i;
	symset set, set1, set2;

	//打开文件
    printf("Please input source file name: ");
	scanf("%s", s);
    //strcpy(s, ".pl0");
	if ((infile = fopen(s, "r")) == NULL) {
		printf("File %s can't be opened.", s);
		exit(1);
	}

	phi = CreateSet(SYM_NULL);
	relset = CreateSet(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	//创建开始符号集
    //decl beg sys：声明开始符号集
    //stat beg sys：状态开始符号集
    //fac beg sys：因子开始符号集
	declbegsys = CreateSet(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL, SYM_BOOL);
	statbegsys = CreateSet(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_FOR, SYM_IDENTIFIER, SYM_BREAK, SYM_EXIT,
                           SYM_NULL);
	facbegsys = CreateSet(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_ARRAY, SYM_NULL);  //在因子的FIRST集中增加数组类型

	//初始化全局变量
    err = 0;
    cc = 0;
    cx = 0;
    ll = 0;
	ch = ' ';

    GetSym();  //先读取第一个标识符

	set1 = CreateSet(SYM_PERIOD, SYM_NULL);
	set2 = UniteSet(declbegsys, statbegsys);
	set = UniteSet(set1, set2);

    //通过block开始进行pl0编译
    Block(set);  //初始的fsys包括点、空、声明开始符、状态开始符

    //销毁掉所有创建的set，包括在.h文件中创建的set，释放空间
    DestroySet(set1);
    DestroySet(set2);
    DestroySet(set);
    DestroySet(phi);
    DestroySet(relset);
    DestroySet(declbegsys);
    DestroySet(statbegsys);
    DestroySet(facbegsys);

	if (sym != SYM_PERIOD) {  //程序运行到最后没有以点结尾，报错
        Error(9);
    }

	/*if (err == 0) {  //没有错误，输出二进制文件
        bin = fopen("bin.txt", "w");
		for (i = 0; i < cx; i++) {
            fwrite(&code[i], sizeof(instruction), 1, bin);
        }
		fclose(bin);
	}*/

    ListCode(0, cx);  //打印指令

	if (err == 0) {
        Interpret();  //程序没有错误，输出程序运行结果
    }
	else {
        if (err == 1) {
            printf("There is %d Error in PL/0 program.\n", err);
        }
        else {
            printf("There are %d Errors in PL/0 program.\n", err);
        }
    }

    return 0;  //注意程序正常返回0
}