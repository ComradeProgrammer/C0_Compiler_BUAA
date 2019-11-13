#pragma once
#include<iostream>
#include<vector>
#include<string>
#include"MidCode.h"
using namespace std;
class MidCodeContainer {
public:
	string functionName;
	vector<MidCode>v;

	int midCodeInsert(MidCodeOp op,int target,
		int opreand1,bool isImmediate1,
		int opreand2,bool isImmediate,
		int label);

	//以下四个函数都是因函数调用时调整计算顺序而使用的
	void midCodeInsert(vector<MidCode>& tmp);
	int getIndex();//返回新的一条指令将会被填入的位置的下标
	vector<MidCode>::iterator getIterator(int index);//返回指定位置的迭代器
	void erase(int start, int end);//擦除从start-end的指令

	void removeNops();
	friend ostream& operator<<(ostream& out, MidCodeContainer c);
	
};