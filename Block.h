#pragma once
#include<vector>
#include<set>
#include"MidCode.h"
using namespace std;

class Block {
public:
	static int count;
	int id; 
	int functionId;
	vector<Block*>prev;
	vector<Block*>next;
	set<int>def;
	set<int>use;
	set<int>activeIn;
	set<int>activeOut;

	Block();
	void insert(MidCode c);
	void addPrev(Block* b);
	void addNext(Block* b);
	void useDefScan();
	bool activeVariableAnalyzeEpoch();
	friend ostream& operator<<(ostream& out, Block b);
	set<int> setUnion(set<int> a, set<int> b);
	set<int> setDifference(set<int> a, set<int> b);
private:
	vector<MidCode>v;
};