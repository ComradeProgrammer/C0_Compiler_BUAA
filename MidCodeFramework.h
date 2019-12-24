#pragma once
#include"MidCodeContainer.h"
#include"FlowGraph.h"
#include"MipsTranslator.h"
using namespace std;
struct ReturnBundle;
class MidCodeFramework {
public :
	MidCodeFramework(MipsTranslator& _mips);
	~MidCodeFramework();
	vector<MidCodeContainer>functionContainer;
	MidCodeContainer container;
	void functionStart(string name);
	void functionEnd();

	int midCodeInsert(MidCodeOp op, int target,
		int operand1, bool isImmediate1,
		int operand2, bool isImmediate2,
		int label);
	void midCodeInsert(vector<MidCode>& tmp);
	int getIndex();//返回新的一条指令将会被填入的位置的下标
	vector<MidCode>::iterator getIterator(int index);//返回指定位置的迭代器
	void erase(int start, int end);//擦除从start-end的指令

	friend ostream& operator<<(ostream&out, MidCodeFramework& frame);
	void optimize();
	void generateMips();
	void removeNops();
	void dumpNewMidCode(ostream& out);
	vector<MidCode> inlinedSimpleFunction(string functionName
		, vector<ReturnBundle>parameters, int returnVar);
private:
	
	vector<FlowGraph>graph;
	MipsTranslator& mips;
	map<int,vector<int>>report;

};