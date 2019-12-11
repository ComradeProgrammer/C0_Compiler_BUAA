#pragma once
#include<vector>
#include<set>
#include"MidCode.h"
#include"DagMap.h"
#include"DeadCodeEliminator.h"
#include"BlockOptimization.h"
#include"PeepHoleOptimization.h"
using namespace std;

class Block {
public:
	static int count;
	int id;//每个block有唯一确定的id 
	int functionId;//所属函数id
	vector<Block*>prev;
	vector<Block*>next;
	set<int>def;
	set<int>use;
	set<int>activeIn;
	set<int>activeOut;
	vector<MidCode>v;

	Block(int _functionId);
	void insert(MidCode c);
	void addPrev(Block* b);
	void addNext(Block* b);
	void useDefScan();
	bool activeVariableAnalyzeEpoch();
	vector<vector<int>>conflictEdgeAnalyze();
	friend ostream& operator<<(ostream& out, Block b);
	static set<int> setUnion(set<int> a, set<int> b);
	static set<int> setDifference(set<int> a, set<int> b);
	void DAGoptimize();
	void eliminateDeadCode();
	void blockOptimize();
	void activeVariableAnalyzePerLine();
	void peepholeOptimize();
};
