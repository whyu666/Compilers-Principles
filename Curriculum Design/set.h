#ifndef SET_H
#define SET_H

typedef struct snode {  //使用链表存储符号
	int elem;  //符号在枚举型定义的位置
	struct snode* next;  //链表的next域
} snode, *symset;

symset phi, declbegsys, statbegsys, facbegsys, relset;

symset CreateSet(int elem, .../* SYM_NULL */);
void DestroySet(symset s);
symset UniteSet(symset s1, symset s2);
int FindSet(int elem, symset s);

#endif