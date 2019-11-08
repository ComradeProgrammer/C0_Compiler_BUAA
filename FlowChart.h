#pragma once
#include<iostream>
#include<vector>
#include"MidCodeContainer.h"
#include"Block.h"
using namespace std;
class FlowChart {
public:
	vector<Block*>chart;
	void addLink(Block* from,Block* to);
	FlowChart(MidCodeContainer& c);
	friend ostream& operator<<(ostream& out, FlowChart f);
};