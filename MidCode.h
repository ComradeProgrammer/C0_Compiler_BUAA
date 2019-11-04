#pragma once
#define MIDINT 0
#define MIDCHAR 1
#define MIDVOID 2
#define MIDNOLABEL -1
#define MIDUNUSED -1
#include<iostream>
#include<vector>
#include"SymbolTable.h"
using namespace std;
/*自己的规定：printf，scanf都直接展开在函数代码里*/
/*在赋值语句中-1号变量留给RET这个特殊变量，其他情况-1填充无用的操作数*/
/*标签中-1号变量标志着没有标签*/
enum MidCodeOp {
	MIDFUNC = 0,//函数声明 1-函数的符号表id 2-函数的返回值类型
	MIDPARA,//函数形参声明 1-形式参数的编号id 2-参数的类型
	MIDPUSH,//函数实参确定 1-实参的id
	MIDCALL,//函数调用 2-函数的id
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
	MIDNEGATE,//取相反数，1-被取反的数
	MIDARRAYGET,// x=a[i],x的id为target，a的id为1,i的id为2
	MIDARRAYWRITE,//a[i]=j a的id为target i的i为1 j的id为2
	MIDASSIGN,//1-赋值的结果
	MIDPRINTINT,//SYSCALL1 1-打印值的id
	MIDPRINTCHAR,//SYSCALL11 1-打印值的id
	MIDPRINTSTRING,//SYSCALL4 1-打印字符串的id
	MIDREADINTEGER,//SYSCALL5 1-读入整数的id target-读入整数的id
	MIDREADCHAR,//SYSCALL12 1-读入字符的id target-读入字符的id
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

	friend ostream& operator<<(ostream& out, MidCode c);
	static string getOperandName(int n,bool isImmediate);
	
	
};