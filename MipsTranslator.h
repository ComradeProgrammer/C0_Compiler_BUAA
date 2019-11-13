#pragma once
#include"FlowGraph.h"
#include<algorithm>
#include<fstream>
#define GLOBALREG 8
#define TMPREG 10
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
	vector<int>Sregister = { 16,17,18,19,20,21,22,23 };
	vector<int>Sstatus = { 0,0,0,0,0,0,0,0 };
	map<int,set<int>>SregisterUser;
	vector<int>Tregister = { 8,9,10,11,12,13,14,15,24,25 };
	vector<int>Tstatus = { 0,0,0,0,0,0,0,0,0,0,0 };
	vector<int>Tuser = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
	map<int, int>varReg;
	set<int>globalVariable;
	set<int>tmpVariable;
	set<int>allVariable;
	map<int, set<int>>conflictMap;
	map<int, vector<int>> report;
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
	int loadOperand(int var, int isImmediate
		, vector<int>conflictVar, vector<int> conflictReg);
	void writeback(int var,int reg);
	void translate(MidCode c);
	void translate(vector<MidCode>c);





};