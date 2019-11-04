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
	int getIndex();//�����µ�һ��ָ��ᱻ�����λ�õ��±�
	vector<MidCode>::iterator getIterator(int index);//����ָ��λ�õĵ�����
	void erase(int start, int end);//������start-end��ָ��
private:
	vector<MidCode>v;
};