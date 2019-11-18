#pragma once
#include<vector>
#include<set>
#include"Block.h"
#include"MidCodeContainer.h"
using namespace std;
class FlowGraph {
public:
	set<int>globalVariable;
	set<int>tmpVariable;
	set<int>allVariable;
	string functionName;
	int functionId;
	vector<Block*>graph;
	vector<vector<int>>conflictEdges;

	FlowGraph(MidCodeContainer& container);
	friend ostream& operator<<(ostream& out, FlowGraph& f);
	void optimize();
	void addLink(Block* from, Block* to);
	void activeVariableAnalyze();
	void conflictEdgeAnalyze();
	void variableSummary();
	void DAGoptimize();
};