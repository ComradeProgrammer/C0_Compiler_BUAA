#pragma once
#include"FlowGraph.h"
#include<algorithm>
#include<fstream>
#define GLOBALREG 8
#define TMPREG 10
//是直接就是被临时分配的寄存器
#define REGTMP 3
//占用状态指里面存储着常数
#define REGOCCUPY 2

#define REGVAR 1
#define REGFREE 0
using namespace std;
class MipsTranslator {
public:
	MipsTranslator(string name);
	void setReport(map<int, vector<int>>report);
	void generateProgramHeader();
	void translateFunction(FlowGraph& g);
	
private:
	int currentFunction;
	Block* currentBlock = nullptr;
	set<int>globalVariable;//所有需要分配全局寄存器的变量
	set<int>tmpVariable;
	set<int>allVariable;
	map<int, set<int>>conflictMap;
	map<int, vector<int>> report;//记录着每个函数栈大小的信息
	//全局寄存器分配
	vector<int>Sregister = { 16,17,18,19,20,21,22,23 };
	vector<int>Sstatus = { 0,0,0,0,0,0,0,0 };
	map<int,set<int>>SregisterUser;//一个s寄存器可以分给多个变量，所以要如此使用
	//临时寄存器分配
	//当前策略:即使在基本块内全局变量会立刻被写回
	//其他分配临时寄存器的变量会在基本块结束时根据活跃性写回；或是在被剥夺寄存器时写回
	//参数寄存器可以被写语句剥夺，但是写语句结束之后立刻会恢复
	vector<int>Tregister = { 8,9,10,11,12,13,14,15,24,25 };
	vector<int>Tstatus = { 0,0,0,0,0,0,0,0,0,0,0 };
	vector<int>Tuser = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
	// 参数寄存器分配
	vector<int>Aregister = {4,5,6,7};
	vector<int>Astatus = { 0,0,0,0 };
	vector<int>Auser = { -1,-1,-1,-1 };
	inline int getTmpRegIndex(int i);
	

	//变量-寄存器
	map<int, int>varReg;
	set<int>loaded;//用来记录数组中每个临时变量是不是已经被初始化了
	string name[32] = {
	"$0","$at","$v0","$v1","$a0",
	"$a1","$a2","$a3","$t0","$t1",
	"$t2","$t3","$t4","$t5","$t6",
	"$t7","$s0","$s1","$s2","$s3",
	"$s4","$s5","$s6","$s7","$t8",
	"$t9","$k0","$k1","$gp","$sp",
	"$fp","$ra" };
	fstream out;

	void translateBlock(Block* b);
	void SregisterAlloc();
	vector<int> TregisterAlloc(int var, int isImmediate
		, vector<int>conflictVar, vector<int> conflictReg);
	vector<int> TregisterAlloc(int var, int isImmediate
		, vector<int>conflictVar, vector<int> conflictReg, set<int>*activeVariable);
	int loadOperand(int var, int isImmediate
		, vector<int>conflictVar, vector<int> conflictReg, set<int>* activeVariable);
	void writeback(int var,int reg);
	void translate(MidCode c);
	void translate(vector<MidCode>c,int type);
	void specialVarwriteback(int var,bool isImmediate);
	void revokeAregister(int reg);
	void writeBackAfterBlock();




};