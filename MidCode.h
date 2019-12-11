#pragma once
#define MIDINT 0
#define MIDCHAR 1
#define MIDVOID 2
#define MIDNOLABEL -1
#define MIDUNUSED -1
#include<iostream>
#include<vector>
#include<set>
#include"SymbolTable.h"
using namespace std;
/*自己的规定：printf，scanf都直接展开在函数代码里*/
/*在赋值语句中-1号变量留给RET这个特殊变量，其他情况-1填充无用的操作数*/
/*标签中-1号变量标志着没有标签*/
enum MidCodeOp {
	MIDFUNC = 0,//函数声明 1-函数的符号表id 2-函数的返回值类型
	MIDPARA,//函数形参声明 1-形式参数的编号id 2-参数的类型
	MIDPUSH,//函数实参确定 1-实参的id
	MIDCALL,//函数调用 1-函数的id
	MIDRET,//函数返回 1-返回编号 若为-1则无返回值
	MIDADD,
	MIDSUB,
	MIDMULT,
	MIDDIV,
	MIDLSS,
	MIDLEQ,
	MIDGRE,
	MIDGEQ,
	MIDEQL,
	MIDNEQ,
	MIDNEGATE,//取相反数，1-被取反的数，target结果
	MIDARRAYGET,// x=a[i],x的id为target，a的id为1,i的id为2
	MIDARRAYWRITE,//a[i]=j a的id为target i的iD为1 j的id为2
	MIDASSIGN,//1-赋值的结果target结果
	MIDGOTO,//无条件跳转，1-跳转目标标号，此处立即数标识使用false
	MIDBNZ,//条件为真跳转 1-所需的条件 2-目标标号
	MIDBZ,//条件为假跳转 1-所需的条件 2-目标标号
	MIDPRINTINT,//SYSCALL1 1-打印值的id
	MIDPRINTCHAR,//SYSCALL11 1-打印字符的id
	MIDPRINTSTRING,//SYSCALL4 1-打印字符串的id
	MIDREADINTEGER,//SYSCALL5 target-读入整数的id
	MIDREADCHAR,//SYSCALL12  target-读入字符的id
	MIDNOP,//对付跳转使用的，优化时候就都干掉了，什么都不要什么都不做
	MIDREM,//取余数 xinzeng
};
/*再这里记录一下*/
class MidCode {
public:
	static SymbolTable* table;
	static int tmpVarNo;
	static int tmpVarAlloc();
	static int labelCount ;
	static int labelAlloc();

	MidCodeOp op;
	int target;
	//第一操作数
	int operand1;
	bool isImmediate1 = false;
	//第二操作数
	int operand2;
	bool isImmediate2 = false;
	int labelNo = -1;
	//该行处活跃变量
	set<int>activeVariable;
	friend ostream& operator<<(ostream& out, MidCode c);
	static string getOperandName(int n,bool isImmediate);
	static string getLabelName(int n);
	static MidCode generateMidCode(MidCodeOp op, int target,
		int operand1, bool isImmediate1,
		int operand2, bool isImmediate2,
		int label );
};