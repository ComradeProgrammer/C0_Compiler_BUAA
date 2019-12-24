#pragma once
#include<vector>
#include<set>
#include"Block.h"
#include"MidCodeContainer.h"
#include"OptimizeSwitch.h"
using namespace std;
class FlowGraph {
public:
	~FlowGraph();
	set<int>globalVariable;
	set<int>tmpVariable;
	set<int>allVariable;
	string functionName;
	int functionId;
	vector<Block*>graph;
	vector<vector<int>>conflictEdges;
	static vector<Block*>allGraph;
	FlowGraph(MidCodeContainer& container);
	friend ostream& operator<<(ostream& out, FlowGraph& f);
	void optimize();
	void addLink(Block* from, Block* to);
	void activeVariableAnalyze();
	void conflictEdgeAnalyze();
	void variableSummary();
	void DAGoptimize();
	void eliminateDeadCode();
	void blockOptimize();
	void activeVariablePerLine();
	void peepholeOptimize();
};