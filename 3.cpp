#include <iostream>
#include <cstring>
#include <cmath>

#define MAX_IN 1000  //最大输入字符数
#define MAX_RESULT 5000  //推导过程保存的最大数量
#define MAX_NUM 30  //最大数字位数

using namespace std;

char InputString[MAX_IN];  //输入的字符串
char str[MAX_IN];  //将输入的字符串中的数字转换为i，以便进行语法分析
string ResultString[MAX_RESULT];  //保存每一次的中间推导过程
int ptr = 0;  //当前分析符号的下标
int Result_Length = 0;  //推导过程的长度
int now_return = 0;  //语义分析过程中，记录返回的数字在number数组中的位置
double number[MAX_IN];  //将数字从char转换为double后依次记录在数组中

struct attribute {  //语义分析所需的inh、syn和val存放在结构体中
    double inh, syn, val;
};

void E(attribute &e);

void T(attribute &t, attribute &f);

void M(attribute &m, attribute &f);

void F(attribute &f);

void N(attribute &n);

void UpdateResult(const string& non_terminal, const string& replace_str);  //用于添加每一次的中间推导式
void ShowResult();  //显示所有中间推导式
void Deal_Production(const string& production);  //接受产生式，该函数把它分解成左边和右边，并调用UpdateResult(),防止代码冗余
void Trim(char s[]);  //去除空格

double trans(const char t[], int length) {  //将合法数字由char[]转换为double
    double sum = 0;
    int point_position = -1;
    for (int i = 0; i < length; i++) {  //查找数字中是否存在小数点
        if (t[i] == '.') {
            point_position = i;
        }
    }
    int j = 0;
    if (point_position == -1) {  //没有小数点
        for (int i = length - 1; i >= 0; i--) {
            sum += int(t[i] - '0') * pow(10, j);
            j++;
        }
    }
    else {  //存在小数点
        for (int i = point_position - 1; i >= 0; i--) {  //先计算整数部分
            sum += int(t[i] - '0') * pow(10, j);
            j++;
        }
        j = 1;
        for (int i = point_position + 1; i < length; i++) {  //再计算小数部分
            sum += int(t[i] - '0') * pow(10, j * (-1));
            j++;
        }
    }
    return sum;
}

int main() {
    cin.getline(InputString, MAX_IN);
    Trim(InputString);  //去除空格
    int left = 0, right = 0;  //left、right判断左右括号数量，不相等一定不接受
    char temp[MAX_NUM], now = 0;  //temp数组保存每位数字，now为temp的指针
    int j = 0, number_num = 0;  //j是str数组的指针，number_num是number数组的指针
    bool num = false, point = false;
    for (int i = 0; i < strlen(InputString); i++) {  //生成str数组
        if (InputString[i] == '(') {
            left++;
        }
        if (InputString[i] == ')') {
            right++;
        }
        if (InputString[i] >= '0' && InputString[i] <= '9') {  //是数字，将每一位数字记录到temp中
            num = true;
            temp[now] = InputString[i];
            now++;
        }
        else if (InputString[i] == '.') {
            if (point) {  //存在多个小数点
                cout << "*** not accept! ***";
                exit(0);
            }
            point = true;
            temp[now] = InputString[i];  //将小数点记录
            now++;
        }
        else {  //非数字和小数点
            if (num) {
                str[j] = 'i';  //数字转化为i，供语法分析使用
                j++;
                str[j] = InputString[i];  //将本字符也保存在str数组中
                number[number_num] = trans(temp, now);  //将数据按照顺序保存在数组中
                number_num++;
                now = 0;  //将temp数组的指针清零
            }
            else {
                str[j] = InputString[i];  //如果符号前不是数字，那么直接将该字符保存在str数组中
            }
            j++;
            num = false;
            point = false;
        }
    }
    cout << str << endl;  //输出转换后的数组
    cout << "number: ";
    for (int i = 0; i < number_num; i++) {  //输出保存的数字
        cout << number[i] << "\t";
    }
    cout << endl;
    attribute e{};
    E(e);  //进行语法和语义分析
    string test = ResultString[Result_Length - 1];  //取最后一次语法分析的结果
    bool t = true;
    for (char i : test) {
        if (i == 'E' || i == 'T' || i == 'M' || i == 'N' || i == 'F') {
            t = false;  //如果分析后，结果中仍有非终结符，不接受
            break;
        }
    }
    if (left != right) {  //左右括号数量不相等，不接受
        t = false;
    }
    if (t) {  //接受则输出信息和计算结果
        cout << "*** accept! ***" << endl << "result: " << e.val << endl;
    }
    else {
        cout << "*** not accept! ***" << endl;
    }
    ShowResult();  //显示已保存的推导式
    return 0;
}

void E(attribute &e) {
    attribute t{}, m{}, f{};
    if (str[ptr] == 'i' || str[ptr] == '(') {  //防止无限推导，在预测分析表中只有i或(，E才有相应产生式
        Deal_Production("E --> TM");
        T(t ,f);  //调用所有非终结符的相应的函数
        m.inh = t.val;  //inh属性在执行前计算
        M(m, f);
        e.val = m.syn;  //val和syn属性在执行后计算
    }
}

void T(attribute &t, attribute &f) {
    if (str[ptr] == 'i' || str[ptr] == '(') {
        Deal_Production("T --> FN");
        attribute n{};
        F(f);
        n.inh = f.val;
        N(n);
        t.val = n.syn;
    }
}

void M(attribute &m1, attribute &f) {
    if (str[ptr] == '+') {
        attribute t{}, m2{};
        Deal_Production("M --> +TM");
        ptr++;  //匹配到一个+，扫描下一个输入符号
        T(t, f);
        m2.inh = m1.inh + f.val;
        M(m2, f);
        m1.syn = m2.syn;
    }
    else if (str[ptr] == ')' || str[ptr] == '#' || str[ptr] == '*') {
        Deal_Production("M --> ε");
        m1.syn = m1.inh;
    }
}

void N(attribute &n1) {
    if (str[ptr] == '*') {
        attribute f{}, n2{};
        Deal_Production("N --> *FN");
        ptr++;
        F(f);
        n2.inh = n1.inh * f.val;
        N(n2);
        n1.syn = n2.syn;
    }
    else if (str[ptr] == ')' || str[ptr] == '#' || str[ptr] == '+') {
        Deal_Production("N --> ε");
        if (str[ptr] == ')') {
            ptr++;
        }
        n1.syn = n1.inh;
    }
}

void F(attribute &f) {
    if (str[ptr] == 'i') {
        Deal_Production("F --> i");
        ptr++;
        f.val = number[now_return];
        now_return++;
    }
    else if (str[ptr] == '(') {
        Deal_Production("F --> (E)");
        ptr++;
        attribute e{};
        E(e);
        f.val = e.val;
    }
}

void UpdateResult(const string& non_terminal, const string& replace_str) {
    if (Result_Length == 0) {  //开始的时候要初始化，加个开始符号
        ResultString[Result_Length++] = non_terminal;
        UpdateResult(non_terminal, replace_str);
    }
    else {
        string str_temp = ResultString[Result_Length - 1];  //先获取上次的推导式
        unsigned long index = str_temp.find(non_terminal);  //找到需要替换的终结符的下标
        string str1 = str_temp.substr(0, index);  //获取终结符前的子串
        string str2 = str_temp.substr(index + non_terminal.length());  //获取终结符后的子串
        if (replace_str != "ε") {  //如果替换为ε，该符号不用加进推导式中
            str1 += replace_str;
        }
        str1 += str2;
        ResultString[Result_Length++] = str1;
    }
}

void ShowResult() {
    cout << "process: " << endl;
    for (int i = 0; i < Result_Length; i++) {
        cout << ResultString[i] << endl;
    }
}

void Deal_Production(const string& production) {
    unsigned long front_index = production.find(' ');
    unsigned long back_index = production.find('>');
    string str1 = production.substr(0, front_index);  //截取前面
    string str2 = production.substr(back_index + 2, production.length() - back_index - 1);  //截取后面
    cout << production << endl;
    UpdateResult(str1, str2);
}

void Trim(char s[]) {
    string str_temp = s;
    string str1, str2;
    while (true) {
        unsigned long flag = str_temp.find(' ');
        if (flag == std::string::npos) {  //没找到空格
            break;
        }
        str1 = str_temp.substr(0, flag); //分割字符串为两部分
        str2 = str_temp.substr(flag + 1, str_temp.length() - flag - 1);
        str1 += str2;  //将没有空格的两部分合并到一起
        str_temp = str1;  //将合并好的值放到临时变量中，继续搜索
    }
    for (int i = 0; i < MAX_IN; i++) {
        s[i] = '\0';  //清空数组
    }
    strcpy(s, str_temp.c_str());
    if (s[strlen(s) - 1] != '#') {
        s[strlen(s)] = '#';  //补充结束符
    }
}