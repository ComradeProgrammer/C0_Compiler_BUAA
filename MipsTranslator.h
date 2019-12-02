#pragma once
#include"FlowGraph.h"
#include<algorithm>
#include<fstream>
#define GLOBALREG 8
#define TMPREG 10
#define REGOCCUPY 2
//占用状态指里面存储着常数或是直接就是被临时分配的寄存器
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

	int loadOperand(int var, bool isImmediate, int reg);
	void writeback(int var, int reg);
	void translate(MidCode c);
	void translate(vector<MidCode>c);
};