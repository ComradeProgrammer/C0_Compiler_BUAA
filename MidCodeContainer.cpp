#include"MidCodeContainer.h"
int MidCodeContainer::midCodeInsert(MidCodeOp op, int target,
	int operand1,bool isImmediate1,
	int operand2, bool isImmediate2,
	int label=-1 ) {
	MidCode tmp;
	tmp.op = op;
	tmp.target = target;
	tmp.operand1 = operand1;
	tmp.operand2 = operand2;
	tmp.isImmediate1 = isImmediate1;
	tmp.isImmediate2 = isImmediate2;
	tmp.labelNo = label;
	int no = v.size();
	v.push_back(tmp);
	return no;
}

ostream& operator<<(ostream& out, MidCodeContainer c) {
	for (int i = 0; i < c.v.size(); i++) {
		out << c.v[i];
	}
	return out;
}