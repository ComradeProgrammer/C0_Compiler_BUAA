#pragma once
#include<iostream>
#include<vector>
#include<deque>
#include"MidCodeContainer.h"
#include"Block.h"
using namespace std;
class FlowChart {
public:

	//Block* start = nullptr;
	//Block* end = nullptr;
	FlowChart(MidCodeContainer& c);
	void addLink(Block* from,Block* to);
	friend ostream& operator<<(ostream& out, FlowChart f);
	void activeVariableAnalyze();
	//void summarize();
private:
	vector<Block*>chart;
};