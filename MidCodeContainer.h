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
	void midCodeInsert(vector<MidCode>&tmp);
	friend ostream& operator<<(ostream& out, MidCodeContainer c);
	int getIndex();//返回新的一条指令将会被填入的位置的下标
	vector<MidCode>::iterator getIterator(int index);//返回指定位置的迭代器
	void erase(int start, int end);//擦除从start-end的指令
private:
	vector<MidCode>v;
};