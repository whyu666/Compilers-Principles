%{
#include <stdio.h>
#include <math.h>
char* color_blue = "<span style=color:Blue>";
char* color_orange = "<span style=color:Orange>";
char* color_gold = "<span style=color:Gold>";
char* color_green = "<span style=color:Green>";
char* color_end = "</span>";
%}

DIGIT [0-9]

%%

\n { fputs("<br/>", yyout); } //处理换行

{DIGIT}+ { fputs(color_blue, yyout); fputs(yytext, yyout); fputs(color_end, yyout); } //处理整形数据

{DIGIT}+"."{DIGIT}* { fputs(color_blue, yyout); fputs(yytext, yyout); fputs(color_end, yyout); } //处理浮点型数据

ifstream|ofstream|endl { fputs(yytext, yyout); } //无需颜色处理的关键字

if|then|begin|end|procedure|function|else|case|struct|return|for|while|using|namespace { fputs(color_orange, yyout); fputs(yytext, yyout); fputs(color_end, yyout); } //处理关键字

int|char|float|double|long|void { fputs(color_orange, yyout); fputs(yytext, yyout); fputs(color_end, yyout);} //处理声明关键字

#include|#define { fputs(color_gold, yyout); fputs(yytext, yyout); fputs(color_end, yyout); } //处理头文件关键字

\/\/.* { fputs(color_green, yyout); fputs(yytext, yyout); fputs(color_end, yyout); } //处理“//”型注释

\/\*(.|\n)*\*\/ { fputs(color_green, yyout); fputs(yytext, yyout); fputs(color_end, yyout);} //处理“/* */”型注释

\"([^\"]*)\" { fputs(color_green, yyout); fputs(yytext, yyout); fputs(color_end, yyout); } //处理双引号

\'([^\']*)\' { fputs(color_green, yyout); fputs(yytext, yyout); fputs(color_end, yyout); } //处理单引号

"<" { fputs("&lt", yyout); } //在HTML中输出“<”

">" { fputs("&gt", yyout); } //在HTML中输出“>”

\t { fputs("&nbsp&nbsp&nbsp&nbsp", yyout); } //实现缩进

.|\n { fputs(yytext, yyout); } //无需处理

%%

int main() {
	FILE *fp1 = fopen("example.cpp", "r");
	FILE *fp2 = fopen("output.html", "w");
	if (fp1 == NULL) {
		return 1;
	}
	yyin = fp1;
	yyout = fp2;
	return yylex();
}