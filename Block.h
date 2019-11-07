#pragma once
#include<vector>
#include"MidCode.h"
using namespace std;

class Block {
public:
	static int count;
	int id;
	vector<Block*>prev;
	vector<Block*>next;

	Block();
	void insert(MidCode c);
	void addPrev(Block* b);
	void addNext(Block* b);
private:
	vector<MidCode>v;
};