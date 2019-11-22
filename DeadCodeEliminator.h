#pragma once
#include<set>
#include"Midcode.h"
using namespace std;
class DeadCodeEliminator {
public:
	DeadCodeEliminator( set<int>& activeOut);
	vector<MidCode> eliminateDeadCode(vector<MidCode>&v);
private:
	set<int>activeOut;
	vector<int>del;
};