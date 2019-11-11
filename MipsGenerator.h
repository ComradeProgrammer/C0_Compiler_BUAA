#pragma once
#include<map>
#include<set>
#include<vector>
#include<iostream>
#include<fstream>
#include"MidCode.h"
using namespace std;
#define NOREG -1
#define UNASSIGNED -2
#define GLOBALREG 8
#define TMPREG 8
using namespace std;
class MipsGenerator {
public:
	void init(set<int>globalVar,set<int>allVar, map<int, vector<int>>report);
	void outputFile(string s);
	void addConflictEdge(int i, int j);


	void globalRegisterAlloc();
	vector<int> tmpRegisterAlloc(int var , int associate1, int associate2);
	
	void generateProgramHeader();
	void parseToMips(MidCode c);
	void pushToMips(vector<MidCode>c);

	void printRegisterAllocStatus(ostream& out);
	void printConflictMap(ostream& out);
private:
	vector<int>globalRegs = { 16,17,18,19,20,21,22,23 };
	vector<int>tmpRegs = { 8,9,10,11,12,13,14,15 };
	vector<int>tmpRegValid= { 0,0,0,0,0,0,0,0,0,0 };
	vector<int>tmpRegPoolVar = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
	map<int, int>regAlloc;//变量-寄存器

	map<int, vector<int>> summaryReport;
	set<int>globalVariables;
	set<int>allVariables;
	map<int, set<int>>conflictMap;
	fstream out;

	int immReg=0;
	string getRegName(int i);
	int currentFunction;
	int genLoadMips(int i,bool isImmediate,int varid2,int varid3);
	void writeback(int i,bool isImmediate);
	void release(int i, bool isImmediate);




};