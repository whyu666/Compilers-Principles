#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "set.h"

//合并两个set
symset UniteSet(symset s1, symset s2) {
	symset s;
	snode* p;
    s = (snode*) malloc(sizeof(snode));
    p = s;  //切记：这里不可以写成p = (snode*) malloc(sizeof(snode));否则会卡begin错误7
    //当s1和s2都未到链表尾时，按照elem值哦那个小到大进行合并
	while (s1 && s2) {
		p->next = (snode*) malloc(sizeof(snode));  //p->next取一个新的结点
		p = p->next;  //将工作指针挪到新结点处
		if (s1->elem < s2->elem) {  //s1小，加入s1
			p->elem = s1->elem;
			s1 = s1->next;
		}
		else {  //s2小，加入s2
			p->elem = s2->elem;
			s2 = s2->next;
		}
	}
	while (s1) {
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		p->elem = s1->elem;
		s1 = s1->next;
	}
	while (s2) {
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		p->elem = s2->elem;
		s2 = s2->next;
	}
	p->next = NULL;  //结束，将尾指针修改为空
	return s;
}

//向链表中加入新数据，一次操作只加入一个新数据
void set_insert(symset s, int elem) {
	snode* p = s;
	snode* q;
	while (p->next && p->next->elem < elem) {  //跳过这些结点
		p = p->next;
	}
    //向链表中加入数据，在p和p->next中插入q，尾插法
	q = (snode*) malloc(sizeof(snode));
	q->elem = elem;
	q->next = p->next;
	p->next = q;
}

//创建一个set并完成初始化，使用stdarg.h中的宏实现可变参数
symset CreateSet(int elem, .../* SYM_NULL */) {
	va_list list;
	symset s;
	s = (snode*) malloc(sizeof(snode));
	s->next = NULL;
    //可用于在参数个数未知（即参数个数可变）时获取函数中的参数。
    //可变参数的函数通在参数列表的末尾是使用省略号(,...)定义的。
	va_start(list, elem);
    //这个宏初始化 ap 变量，它与 va_arg 和 va_end 宏是一起使用的。last_arg 是最后一个传递给函数的已知的固定参数，即省略号之前的参数。
	while (elem) {  //循环完成，将elem后的所有数据都存入s中
        set_insert(s, elem);  //完成初始化，将elem指针中的数据传入新定义的s链表中
		elem = va_arg(list, int);  //这个宏检索函数参数列表中类型为 type 的下一个参数。
	}
	va_end(list);  //这个宏允许使用了 va_start 宏的带有可变参数的函数返回。如果在从函数返回之前没有调用 va_end，则结果为未定义。
	return s;
}

//销毁一个set
void DestroySet(symset s) {
	snode* p;
	while (s) {
		p = s;
		p->elem = -1000000;
		s = s->next;
		free(p);
	}
}

//在set中寻找值elem的项
int FindSet(int elem, symset s) {
	s = s->next;
	while (s && (s->elem < elem)) {  //寻找elem和s->elem相等的项
		s = s->next;
	}
	if (s && (s->elem == elem)) {  //找到返回1
		return 1;
	}
	else {  //未找到返回0
		return 0;
	}
}