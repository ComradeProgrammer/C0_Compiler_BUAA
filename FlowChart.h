#pragma once
#include<iostream>
#include<vector>
#include<deque>
#include"MidCodeContainer.h"
#include"Block.h"
#include"MipsGenerator.h"
using namespace std;
class FlowChart {
public:
	FlowChart(MidCodeContainer& c,MipsGenerator&m);
	void addLink(Block* from,Block* to);
	friend ostream& operator<<(ostream& out, FlowChart f);
	void activeVariableAnalyze();
	void conflictEdgeAnalyze();
	void optimize();

	void summarize();
	void go();
private:
	vector<Block*>chart;
	set<int>allVariables;
	set<int>allGlobalVariables;
	MipsGenerator& mips;
};