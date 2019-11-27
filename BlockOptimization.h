#pragma once
#include"MidCode.h"
struct Item {
	int id;
	bool isImmediate=false;
};
class BlockOptimization {
public:
	BlockOptimization(set<int>&activeOut);
	vector<MidCode>propagationInBlock(vector<MidCode>& v);
private:
	set<int>activeOut;
	int label = -1;
};