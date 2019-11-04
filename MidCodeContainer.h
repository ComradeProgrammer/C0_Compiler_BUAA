#pragma once
#include<iostream>
#include<vector>
#include<string>
#include"MidCode.h"
using namespace std;
class MidCodeContainer {
public:
	int midCodeInsert(MidCodeOp op,int target,
		int opreand1,bool isImmediate1,
		int opreand2,bool isImmediate,
		int label);
	friend ostream& operator<<(ostream& out, MidCodeContainer c);
private:
	vector<MidCode>v;
};